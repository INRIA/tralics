#include "tralics/Parser.h"
#include "tralics/AllIndex.h"
#include "tralics/Bbl.h"
#include "tralics/Bibliography.h"
#include "tralics/Bibtex.h"
#include "tralics/ColSpec.h"
#include "tralics/Dispatcher.h"
#include "tralics/LineList.h"
#include "tralics/Logger.h"
#include "tralics/NameMapper.h"
#include "tralics/NewArray.h"
#include "tralics/Saver.h"
#include "tralics/Symcode.h"
#include "tralics/globals.h"
#include "tralics/util.h"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

namespace accent_ns {
    auto fetch_double_accent(int a, int acc3) -> Token;
    auto double_a_accent(int acc3) -> unsigned;
    auto double_e_accent(int acc3) -> unsigned;
    auto double_o_accent(int acc3) -> unsigned;
    auto double_u_accent(int acc3) -> unsigned;
    auto double_other_accent(int a, int acc3) -> unsigned;
    void special_acc_hack(TokenList &y);
} // namespace accent_ns

namespace {
    std::string saved_dim;

    // This is now cline
    void T_cline() {
        Xml *R       = the_stack.top_stack()->last_addr();
        auto cl_span = (cline_last - cline_first) + 1;
        if (R != nullptr) {
            if (R->try_cline(false)) {
                R->try_cline(true);
                return;
            }
            long tot_span = 0;
            if (R->total_span(tot_span)) {
                tot_span = cline_first - 1 - tot_span;
                if (0 <= tot_span) {
                    if (tot_span != 0) {
                        Xml *x = new Xml(the_names["cell"], nullptr);
                        R->push_back_unless_nullptr(x);
                        x->id.add_span(tot_span);
                    }
                    Xml *x = new Xml(the_names["cell"], nullptr);
                    R->push_back_unless_nullptr(x);
                    x->id.add_span(cl_span);
                    x->id.add_bottom_rule();
                    return;
                }
            }
            if (R->is_element() && R->has_name(the_names["row"])) {
                if (R->try_cline_again(false)) {
                    R->try_cline_again(true);
                    R->name = std::string();
                    the_stack.add_border(cline_first, cl_span);
                    the_log << "\\cline killed a cell \n";
                    return;
                }
            }
        }
        the_stack.add_border(cline_first, cl_span);
    }

    // This is for lower-case upper-case conversions.
    // Defines values for the character c
    void mklcuc(size_t c, size_t lc, size_t uc) {
        eqtb_int_table[c + lc_code_offset].val = to_signed(lc);
        eqtb_int_table[c + uc_code_offset].val = to_signed(uc);
    }

    // This is for lower-case upper-case conversions.
    // Defines values for the pair lc, uc
    void mklcuc(size_t lc, size_t uc) {
        eqtb_int_table[lc + lc_code_offset].val = to_signed(lc);
        eqtb_int_table[lc + uc_code_offset].val = to_signed(uc);
        eqtb_int_table[uc + lc_code_offset].val = to_signed(lc);
        eqtb_int_table[uc + uc_code_offset].val = to_signed(uc);
    }

    // This creates the lc and uc tables.
    // Only ascii characters have a non-zero value
    void make_uclc_table() {
        for (unsigned i = 'a'; i <= 'z'; i++) mklcuc(i, i, i - 32);
        for (unsigned i = 224; i <= 255; i++) mklcuc(i, i, i - 32);
        for (unsigned i = 'A'; i <= 'Z'; i++) mklcuc(i, i + 32, i);
        for (unsigned i = 192; i <= 223; i++) mklcuc(i, i + 32, i);
        mklcuc(215, 0, 0);
        mklcuc(247, 0, 0);    // multiplication division (\367 \327)
        mklcuc(223, 0, 0);    // sharp s (\377 347)
        mklcuc(0xFF, 0x178);  // \"y
        mklcuc(0x153, 0x152); // oe
        mklcuc(0x133, 0x132); // ij
        mklcuc(0x142, 0x141); // \l
        mklcuc(0x14B, 0x14A); // \ng
        mklcuc(0x111, 0x110); // \dh
    }

    void finish_index() {
        auto &the_index = AllIndex::the_index();
        auto  labels    = std::vector<std::string>(the_index.last_iid, "");
        tralics_ns::find_index_labels(labels);
        size_t idx_size = 0, idx_nb = 0;
        auto   q = the_index.size();
        for (size_t jj = 1; jj <= q; jj++) {
            auto  j  = jj == q ? 0 : jj;
            auto &CI = the_index.at(j);
            auto  n  = CI.size();
            if (n == 0) continue;
            idx_size += n;
            idx_nb++;
            Xml *res = new Xml(the_names[j == 0 ? "theglossary" : "theindex"], nullptr); // OK?
            Xid  id  = res->id;
            {
                const std::string &t = CI.title;
                if (!t.empty()) id.add_attribute(the_names["title"], std::string(t));
            }
            {
                AttList &L = the_stack.get_att_list(CI.AL);
                id.add_attribute(L, true);
            }
            for (size_t i = 0; i < n; i++) {
                Xml *A = CI.at(i).translation;
                A->id.add_attribute(the_names["target"], std::string(labels[CI.at(i).iid]));
            }
            std::sort(CI.begin(), CI.end(), [](const auto &A, const auto &B) { return A.key < B.key; });
            for (size_t i = 0; i < n; i++) {
                Xml *A = CI.at(i).translation;
                res->push_back_unless_nullptr(A);
                res->add_nl();
            }
            the_stack.document_element()->insert_bib(res, CI.position);
        }
        if (idx_size == 0) return;
        log_and_tty << "Index has " << idx_size << " entries";
        if (idx_nb > 1) log_and_tty << " in " << idx_nb << " clusters";
        log_and_tty << "\n";
    }

    // This creates the bbl file by running an external program.
    void create_aux_file_and_run_pgm() {
        if (!the_main.shell_escape_allowed) {
            spdlog::warn("Cannot call external program unless using option -shell-escape");
            return;
        }
        Buffer B;
        bbl.lines.clear();
        Bibliography &T = the_bibliography;
        T.dump(B);
        if (B.empty()) return;
        T.dump_data(B);
        std::string auxname = file_name + ".aux";
        try {
            std::ofstream(auxname) << B;
        } catch (...) {
            spdlog::warn("Cannot open file {} for output, bibliography will be missing", auxname);
            return;
        }
        spdlog::info("Executing bibliography command: {}", T.cmd);
        system(T.cmd.c_str());
        bbl.lines.read(file_name + ".bbl", 1);
    }

    // This creates a <ref target='bidN'/> element. This is the REF that needs
    // to be solved later. In the case of \footcite[p.25]{Knuth},
    // the arguments of the function are foot and Knuth; the `p.25' will be
    // considered elsewhere.
    auto make_cit_ref(const std::string &type, const std::string &ref) -> Xml {
        auto        n  = *the_bibliography.find_citation_item(type, ref, true);
        std::string id = the_bibliography.citation_table[n].get_id();
        Xml         res(the_names["ref"], nullptr);
        res.id.add_attribute(the_names["target"], id);
        return res;
    }

    // For the case of \'\^e, construct an accent code.
    // It is assumed that the order is irrelevant.

    auto combine_accents(unsigned acc1, unsigned acc2) -> int {
        // start with acute and something
        if (acc1 == '\'' && acc2 == '~') return 9;
        if (acc2 == '\'' && acc1 == '~') return 9;
        if (acc1 == '\'' && acc2 == '=') return 10;
        if (acc2 == '\'' && acc1 == '=') return 10;
        if (acc1 == '\'' && acc2 == '.') return 11;
        if (acc2 == '\'' && acc1 == '.') return 11;
        if (acc1 == '\'' && acc2 == '^') return 12;
        if (acc2 == '\'' && acc1 == '^') return 12;
        if (acc1 == '\'' && acc2 == 'H') return 16;
        if (acc2 == '\'' && acc1 == 'H') return 16;
        if (acc1 == '\'' && acc2 == 'c') return 17;
        if (acc2 == '\'' && acc1 == 'c') return 17;
        if (acc1 == '\'' && acc2 == '"') return 2;
        if (acc2 == '\'' && acc1 == '"') return 2;
        if (acc1 == '\'' && acc2 == '^') return 21;
        if (acc2 == '\'' && acc1 == '^') return 21;
        if (acc1 == '\'' && acc2 == 'u') return 34;
        if (acc2 == '\'' && acc1 == 'u') return 34;
        // now grave and something
        if (acc1 == '`' && acc2 == '=') return 13;
        if (acc2 == '`' && acc1 == '=') return 13;
        if (acc1 == '`' && acc2 == '^') return 15;
        if (acc2 == '`' && acc1 == '^') return 15;
        if (acc1 == '`' && acc2 == 'H') return 18;
        if (acc2 == '`' && acc1 == 'H') return 18;
        if (acc1 == '`' && acc2 == '"') return 4;
        if (acc2 == '`' && acc1 == '"') return 4;
        if (acc1 == '`' && acc2 == 'u') return 22;
        if (acc2 == '`' && acc1 == 'u') return 22;
        // now dot and something
        if (acc1 == '.' && acc2 == '=') return 5;
        if (acc2 == '.' && acc1 == '=') return 5;
        if (acc1 == '.' && acc2 == 'v') return 19;
        if (acc2 == '.' && acc1 == 'v') return 19;
        if (acc1 == '.' && acc2 == 'd') return 20;
        if (acc2 == '.' && acc1 == 'd') return 20;
        // now hat and something
        if (acc1 == '^' && acc2 == 'h') return 23;
        if (acc2 == '^' && acc1 == 'h') return 23;
        if (acc1 == '^' && acc2 == '~') return 24;
        if (acc2 == '^' && acc1 == '~') return 24;
        if (acc1 == '^' && acc2 == 'd') return 25;
        if (acc2 == '^' && acc1 == 'd') return 25;
        // diaresis
        if (acc1 == '"' && acc2 == '=') return 1;
        if (acc2 == '"' && acc1 == '=') return 1;
        if (acc1 == '"' && acc2 == 'v') return 3;
        if (acc2 == '"' && acc1 == 'v') return 3;
        if (acc1 == '"' && acc2 == '~') return 27;
        if (acc2 == '"' && acc1 == '~') return 27;
        // other
        if (acc1 == '=' && acc2 == '~') return 6;
        if (acc2 == '=' && acc1 == '~') return 6;
        if (acc1 == 'k' && acc2 == '=') return 7;
        if (acc2 == 'k' && acc1 == '=') return 7;
        if (acc1 == 'd' && acc2 == '=') return 14;
        if (acc2 == 'd' && acc1 == '=') return 14;
        if (acc1 == 'u' && acc2 == 'c') return 26;
        if (acc2 == 'u' && acc1 == 'c') return 26;
        if (acc1 == 'H' && acc2 == 'h') return 28;
        if (acc2 == 'H' && acc1 == 'h') return 28;
        if (acc1 == 'H' && acc2 == '~') return 29;
        if (acc2 == 'H' && acc1 == '~') return 29;
        if (acc1 == 'H' && acc2 == 'd') return 30;
        if (acc2 == 'H' && acc1 == 'd') return 30;
        if (acc1 == 'u' && acc2 == 'h') return 31;
        if (acc2 == 'u' && acc1 == 'h') return 31;
        if (acc1 == 'u' && acc2 == '~') return 32;
        if (acc2 == 'u' && acc1 == '~') return 32;
        if (acc1 == 'u' && acc2 == 'd') return 33;
        if (acc2 == 'u' && acc1 == 'd') return 33;
        return 0;
    }

    // Accent tables. Holds the result of the expansion
    std::array<Token, nb_accents>           accent_cir;        // \^
    std::array<Token, nb_accents>           accent_acute;      // \'
    std::array<Token, nb_accents>           accent_grave;      // \`
    std::array<Token, nb_accents>           accent_trema;      // \"
    std::array<Token, nb_accents>           accent_cedille;    // \c
    std::array<Token, nb_accents>           accent_breve;      // \u
    std::array<Token, nb_accents>           accent_check;      // \v
    std::array<Token, nb_accents>           accent_tilde;      // \~
    std::array<Token, nb_accents>           accent_uml;        // \H
    std::array<Token, nb_accents>           accent_ogon;       // \k
    std::array<Token, nb_accents>           accent_dotabove;   // \.
    std::array<Token, nb_accents>           accent_macron;     // \=
    std::array<Token, nb_accents>           accent_ring;       // \r
    std::array<Token, nb_accents>           accent_barunder;   // \b
    std::array<Token, nb_accents>           accent_dotunder;   // \d
    std::array<Token, nb_accents>           accent_ibreve;     // \f (inverted breve)
    std::array<Token, nb_accents>           accent_dgrave;     // \C like \`\`
    std::array<Token, nb_accents>           accent_tildeunder; // \T
    std::array<Token, nb_accents>           accent_circunder;  // \V
    std::array<Token, nb_accents>           accent_rondunder;  // \D
    std::array<Token, nb_accents>           accent_hook;       // \h
    std::array<Token, special_table_length> other_accent;

    // Returns the token associated to acc for character chr.
    // Return zero-token in case of failure
    auto fetch_accent(size_t chr, unsigned acc) -> Token {
        switch (acc) {
        case '\'': return accent_acute[chr];
        case '`': return accent_grave[chr];
        case '^': return accent_cir[chr];
        case '"': return accent_trema[chr];
        case '~': return accent_tilde[chr];
        case 'k': return accent_ogon[chr];
        case 'H': return accent_uml[chr];
        case 'v': return accent_check[chr];
        case 'u': return accent_breve[chr];
        case 'c': return accent_cedille[chr];
        case '.': return accent_dotabove[chr];
        case '=': return accent_macron[chr];
        case 'r': return accent_ring[chr];
        case 'b': return accent_barunder[chr];
        case 'd': return accent_dotunder[chr];
        case 'C': return accent_dgrave[chr];
        case 'f': return accent_ibreve[chr];
        case 'T': return accent_tildeunder[chr];
        case 'V': return accent_circunder[chr];
        case 'D': return accent_rondunder[chr];
        case 'h': return accent_hook[chr];
        default: return Token(0);
        }
    }

    // Puts char k, with letter catcode, in the table T at position k.
    void mk_letter(Token *T, uchar k) { T[k] = Token(letter_t_offset, k); }

    // Simple case of \^a that gives \342.
    auto mk_acc(unsigned s) -> Token { return Token(other_t_offset, char32_t(s)); }

    // Creates the table for accents.
    void boot_accents() {
        accent_acute[0]      = mk_acc(0x301);
        accent_grave[0]      = mk_acc(0x300);
        accent_cir[0]        = mk_acc(0x302);
        accent_trema[0]      = mk_acc(0x30e);
        accent_tilde[0]      = mk_acc(0x303);
        accent_ogon[0]       = mk_acc(0x328);
        accent_uml[0]        = mk_acc(0x30b);
        accent_check[0]      = mk_acc(0x30c);
        accent_breve[0]      = mk_acc(0x306);
        accent_cedille[0]    = mk_acc(0x327);
        accent_dotabove[0]   = mk_acc(0x307);
        accent_macron[0]     = mk_acc(0x304);
        accent_ring[0]       = mk_acc(0x30a);
        accent_barunder[0]   = mk_acc(0x320);
        accent_dotunder[0]   = mk_acc(0x323);
        accent_dgrave[0]     = mk_acc(0x30f);
        accent_ibreve[0]     = mk_acc(0x311);
        accent_tildeunder[0] = mk_acc(0x330);
        accent_circunder[0]  = mk_acc(0x32d);
        accent_rondunder[0]  = mk_acc(0x325);
        accent_hook[0]       = mk_acc(0x309);

        accent_grave[uchar('A')] = mk_acc(uchar('\300'));
        accent_acute[uchar('A')] = mk_acc(uchar('\301'));
        accent_cir[uchar('A')]   = mk_acc(uchar('\302'));
        accent_tilde[uchar('A')] = mk_acc(uchar('\303'));
        accent_trema[uchar('A')] = mk_acc(uchar('\304'));
        accent_ring[uchar('A')]  = mk_acc(0305);
        // 306=\AE
        accent_cedille[uchar('C')] = mk_acc(uchar('\307'));
        accent_grave[uchar('E')]   = mk_acc(uchar('\310'));
        accent_acute[uchar('E')]   = mk_acc(uchar('\311'));
        accent_cir[uchar('E')]     = mk_acc(uchar('\312'));
        accent_trema[uchar('E')]   = mk_acc(uchar('\313'));
        accent_grave[uchar('I')]   = mk_acc(uchar('\314'));
        accent_acute[uchar('I')]   = mk_acc(uchar('\315'));
        accent_cir[uchar('I')]     = mk_acc(uchar('\316'));
        accent_trema[uchar('I')]   = mk_acc(uchar('\317'));
        // 320= \DH
        accent_tilde[uchar('N')] = mk_acc(uchar('\321'));
        accent_grave[uchar('O')] = mk_acc(uchar('\322'));
        accent_acute[uchar('O')] = mk_acc(uchar('\323'));
        accent_cir[uchar('O')]   = mk_acc(uchar('\324'));
        accent_tilde[uchar('O')] = mk_acc(uchar('\325'));
        accent_trema[uchar('O')] = mk_acc(uchar('\326'));
        // 327=\times
        // 330=\O
        accent_grave[uchar('U')] = mk_acc(uchar('\331'));
        accent_acute[uchar('U')] = mk_acc(uchar('\332'));
        accent_cir[uchar('U')]   = mk_acc(uchar('\333'));
        accent_trema[uchar('U')] = mk_acc(uchar('\334'));
        accent_acute[uchar('Y')] = mk_acc(uchar('\335'));
        // 336=\TH, \337=\ss
        accent_grave[uchar('a')] = mk_acc(uchar('\340'));
        accent_acute[uchar('a')] = mk_acc(uchar('\341'));
        accent_cir[uchar('a')]   = mk_acc(uchar('\342'));
        accent_tilde[uchar('a')] = mk_acc(uchar('\343'));
        accent_trema[uchar('a')] = mk_acc(uchar('\344'));
        accent_ring[uchar('a')]  = mk_acc(uchar('\345'));
        // 346=\ae
        accent_cedille[uchar('c')] = mk_acc(uchar('\347'));
        accent_grave[uchar('e')]   = mk_acc(uchar('\350'));
        accent_acute[uchar('e')]   = mk_acc(uchar('\351'));
        accent_cir[uchar('e')]     = mk_acc(uchar('\352'));
        accent_trema[uchar('e')]   = mk_acc(uchar('\353'));
        accent_grave[uchar('i')]   = mk_acc(uchar('\354'));
        accent_acute[uchar('i')]   = mk_acc(uchar('\355'));
        accent_cir[uchar('i')]     = mk_acc(uchar('\356'));
        accent_trema[uchar('i')]   = mk_acc(uchar('\357'));
        // 360=\dh
        accent_tilde[uchar('n')] = mk_acc(uchar('\361'));
        accent_grave[uchar('o')] = mk_acc(uchar('\362'));
        accent_acute[uchar('o')] = mk_acc(uchar('\363'));
        accent_cir[uchar('o')]   = mk_acc(uchar('\364'));
        accent_tilde[uchar('o')] = mk_acc(uchar('\365'));
        accent_trema[uchar('o')] = mk_acc(uchar('\366'));
        // 367=\div 370=\o
        accent_grave[uchar('u')] = mk_acc(uchar('\371'));
        accent_acute[uchar('u')] = mk_acc(uchar('\372'));
        accent_cir[uchar('u')]   = mk_acc(uchar('\373'));
        accent_trema[uchar('u')] = mk_acc(uchar('\374'));
        accent_acute[uchar('y')] = mk_acc(uchar('\375'));
        // 376 =\th
        accent_trema[uchar('y')] = mk_acc(uchar('\377'));

        accent_macron[uchar('A')]   = mk_acc(0x100);
        accent_macron[uchar('a')]   = mk_acc(0x101);
        accent_breve[uchar('A')]    = mk_acc(0x102);
        accent_breve[uchar('a')]    = mk_acc(0x103);
        accent_ogon[uchar('A')]     = mk_acc(0x104);
        accent_ogon[uchar('a')]     = mk_acc(0x105);
        accent_acute[uchar('C')]    = mk_acc(0x106);
        accent_acute[uchar('c')]    = mk_acc(0x107);
        accent_cir[uchar('C')]      = mk_acc(0x108);
        accent_cir[uchar('c')]      = mk_acc(0x109);
        accent_dotabove[uchar('C')] = mk_acc(0x10a);
        accent_dotabove[uchar('c')] = mk_acc(0x10b);
        accent_check[uchar('C')]    = mk_acc(0x10c);
        accent_check[uchar('c')]    = mk_acc(0x10d);
        accent_check[uchar('D')]    = mk_acc(0x10e);
        accent_check[uchar('d')]    = mk_acc(0x10f);

        // 110 = \Dj, 111=\dj
        accent_macron[uchar('E')]   = mk_acc(0x112);
        accent_macron[uchar('e')]   = mk_acc(0x113);
        accent_breve[uchar('E')]    = mk_acc(0x114);
        accent_breve[uchar('e')]    = mk_acc(0x115);
        accent_dotabove[uchar('E')] = mk_acc(0x116);
        accent_dotabove[uchar('e')] = mk_acc(0x117);
        accent_ogon[uchar('E')]     = mk_acc(0x118);
        accent_ogon[uchar('e')]     = mk_acc(0x119);
        accent_check[uchar('E')]    = mk_acc(0x11a);
        accent_check[uchar('e')]    = mk_acc(0x11b);
        accent_cir[uchar('G')]      = mk_acc(0x11c);
        accent_cir[uchar('g')]      = mk_acc(0x11d);
        accent_breve[uchar('G')]    = mk_acc(0x11e);
        accent_breve[uchar('g')]    = mk_acc(0x11f);

        accent_dotabove[uchar('G')] = mk_acc(0x120);
        accent_dotabove[uchar('g')] = mk_acc(0x121);
        accent_cedille[uchar('G')]  = mk_acc(0x122);
        accent_cedille[uchar('g')]  = mk_acc(0x123);
        accent_cir[uchar('H')]      = mk_acc(0x124);
        accent_cir[uchar('h')]      = mk_acc(0x125);
        accent_macron[uchar('H')]   = mk_acc(0x126);
        accent_macron[uchar('h')]   = mk_acc(0x127);
        accent_tilde[uchar('I')]    = mk_acc(0x128);
        accent_tilde[uchar('i')]    = mk_acc(0x129);
        accent_macron[uchar('I')]   = mk_acc(0x12a);
        accent_macron[uchar('i')]   = mk_acc(0x12b);
        accent_breve[uchar('I')]    = mk_acc(0x12c);
        accent_breve[uchar('i')]    = mk_acc(0x12d);
        accent_ogon[uchar('I')]     = mk_acc(0x12e);
        accent_ogon[uchar('i')]     = mk_acc(0x12f);

        accent_dotabove[uchar('I')] = mk_acc(0x130);
        // i_code=131, IJ = 0x132, ij = 0x133
        accent_cir[uchar('J')]     = mk_acc(0x134);
        accent_cir[uchar('j')]     = mk_acc(0x135);
        accent_cedille[uchar('K')] = mk_acc(0x136);
        accent_cedille[uchar('k')] = mk_acc(0x137);
        // kgreen = 138
        accent_acute[uchar('L')]    = mk_acc(0x139);
        accent_acute[uchar('l')]    = mk_acc(0x13a);
        accent_cedille[uchar('L')]  = mk_acc(0x13b);
        accent_cedille[uchar('l')]  = mk_acc(0x13c);
        accent_check[uchar('L')]    = mk_acc(0x13d);
        accent_check[uchar('l')]    = mk_acc(0x13e);
        accent_dotabove[uchar('L')] = mk_acc(0x13f);
        accent_dotabove[uchar('l')] = mk_acc(0x140);
        // \L = 0x141, \l = 0x142
        accent_acute[uchar('N')]   = mk_acc(0x143);
        accent_acute[uchar('n')]   = mk_acc(0x144);
        accent_cedille[uchar('N')] = mk_acc(0x145);
        accent_cedille[uchar('n')] = mk_acc(0x146);
        accent_check[uchar('N')]   = mk_acc(0x147);
        accent_check[uchar('n')]   = mk_acc(0x148);
        // 0x149 = n apostrophe "ng" 0x14a, NG = 0x14b
        accent_macron[uchar('O')] = mk_acc(0x14c);
        accent_macron[uchar('o')] = mk_acc(0x14d);
        accent_breve[uchar('O')]  = mk_acc(0x14e);
        accent_breve[uchar('o')]  = mk_acc(0x14f);
        accent_uml[uchar('O')]    = mk_acc(0x150);
        accent_uml[uchar('o')]    = mk_acc(0x151);
        // mk_acc("OE",0x152);
        // mk_acc("oe",0x153);
        accent_acute[uchar('R')]   = mk_acc(0x154);
        accent_acute[uchar('r')]   = mk_acc(0x155);
        accent_cedille[uchar('R')] = mk_acc(0x156);
        accent_cedille[uchar('r')] = mk_acc(0x157);
        accent_check[uchar('R')]   = mk_acc(0x158);
        accent_check[uchar('r')]   = mk_acc(0x159);
        accent_acute[uchar('S')]   = mk_acc(0x15a);
        accent_acute[uchar('s')]   = mk_acc(0x15b);
        accent_cir[uchar('S')]     = mk_acc(0x15c);
        accent_cir[uchar('s')]     = mk_acc(0x15d);
        accent_cedille[uchar('S')] = mk_acc(0x15e);
        accent_cedille[uchar('s')] = mk_acc(0x15f);
        accent_check[uchar('S')]   = mk_acc(0x160);
        accent_check[uchar('s')]   = mk_acc(0x161);
        accent_cedille[uchar('T')] = mk_acc(0x162);
        accent_cedille[uchar('t')] = mk_acc(0x163);
        accent_check[uchar('T')]   = mk_acc(0x164);
        accent_check[uchar('t')]   = mk_acc(0x165);
        accent_macron[uchar('T')]  = mk_acc(0x166);
        accent_macron[uchar('t')]  = mk_acc(0x167);
        accent_tilde[uchar('U')]   = mk_acc(0x168);
        accent_tilde[uchar('u')]   = mk_acc(0x169);
        accent_macron[uchar('U')]  = mk_acc(0x16a);
        accent_macron[uchar('u')]  = mk_acc(0x16b);
        accent_breve[uchar('U')]   = mk_acc(0x16c);
        accent_breve[uchar('u')]   = mk_acc(0x16d);
        accent_ring[uchar('U')]    = mk_acc(0x16e);
        accent_ring[uchar('u')]    = mk_acc(0x16f);

        accent_uml[uchar('U')]      = mk_acc(0x170);
        accent_uml[uchar('u')]      = mk_acc(0x171);
        accent_ogon[uchar('U')]     = mk_acc(0x172);
        accent_ogon[uchar('u')]     = mk_acc(0x173);
        accent_cir[uchar('W')]      = mk_acc(0x174);
        accent_cir[uchar('w')]      = mk_acc(0x175);
        accent_cir[uchar('Y')]      = mk_acc(0x176);
        accent_cir[uchar('y')]      = mk_acc(0x177);
        accent_trema[uchar('Y')]    = mk_acc(0x178);
        accent_acute[uchar('Z')]    = mk_acc(0x179);
        accent_acute[uchar('z')]    = mk_acc(0x17a);
        accent_dotabove[uchar('Z')] = mk_acc(0x17b);
        accent_dotabove[uchar('z')] = mk_acc(0x17c);
        accent_check[uchar('Z')]    = mk_acc(0x17d);
        accent_check[uchar('z')]    = mk_acc(0x17e);
        // long_s  0x17f

        // 01c?
        accent_check[uchar('A')] = mk_acc(0x1cd);
        accent_check[uchar('a')] = mk_acc(0x1ce);
        accent_check[uchar('I')] = mk_acc(0x1cf);

        accent_check[uchar('i')]   = mk_acc(0x1d0);
        accent_check[uchar('O')]   = mk_acc(0x1d1);
        accent_check[uchar('o')]   = mk_acc(0x1d2);
        accent_check[uchar('U')]   = mk_acc(0x1d3);
        accent_check[uchar('u')]   = mk_acc(0x1d4);
        other_accent[Udiamacro_cc] = mk_acc(0x1d5);
        other_accent[udiamacro_cc] = mk_acc(0x1d6);
        other_accent[Udiaacute_cc] = mk_acc(0x1d7);
        other_accent[udiaacute_cc] = mk_acc(0x1d8);
        other_accent[Udiacaron_cc] = mk_acc(0x1d9);
        other_accent[udiacaron_cc] = mk_acc(0x1da);
        other_accent[Udiagrave_cc] = mk_acc(0x1db);
        other_accent[udiagrave_cc] = mk_acc(0x1dc);
        // inverted e = 1dd
        other_accent[Adiamacro_cc] = mk_acc(0x1de);
        other_accent[adiamacro_cc] = mk_acc(0x1df);

        other_accent[Adotmacro_cc] = mk_acc(0x1e0);
        other_accent[adotmacro_cc] = mk_acc(0x1e1);
        accent_macron[1]           = mk_acc(0x1e2);
        accent_macron[2]           = mk_acc(0x1e3);
        // 1e4 1e5: G with bar
        accent_check[uchar('G')]     = mk_acc(0x1e6);
        accent_check[uchar('g')]     = mk_acc(0x1e7);
        accent_check[uchar('K')]     = mk_acc(0x1e8);
        accent_check[uchar('k')]     = mk_acc(0x1e9);
        accent_ogon[uchar('O')]      = mk_acc(0x1ea);
        accent_ogon[uchar('o')]      = mk_acc(0x1eb);
        other_accent[Oogonmacron_cc] = mk_acc(0x1ec);
        other_accent[oogonmacron_cc] = mk_acc(0x1ed);
        // 1ee 1ef: ezh with caron.

        accent_check[uchar('j')] = mk_acc(0x1f0);
        // 1f1 1f2 1f3: DZ, Dz and dz
        accent_acute[uchar('G')] = mk_acc(0x1f4);
        accent_acute[uchar('g')] = mk_acc(0x1f5);
        // 1f6 1f7 ?
        accent_grave[uchar('N')] = mk_acc(0x1f8);
        accent_grave[uchar('n')] = mk_acc(0x1f9);
        accent_acute[3]          = mk_acc(0x1fa);
        accent_acute[4]          = mk_acc(0x1fb);
        accent_acute[1]          = mk_acc(0x1fc);
        accent_acute[2]          = mk_acc(0x1fd);
        accent_acute[5]          = mk_acc(0x1fe);
        accent_acute[6]          = mk_acc(0x1ff);

        accent_dgrave[uchar('A')] = mk_acc(0x200);
        accent_dgrave[uchar('a')] = mk_acc(0x201);
        accent_ibreve[uchar('A')] = mk_acc(0x202);
        accent_ibreve[uchar('a')] = mk_acc(0x203);
        accent_dgrave[uchar('E')] = mk_acc(0x204);
        accent_dgrave[uchar('e')] = mk_acc(0x205);
        accent_ibreve[uchar('E')] = mk_acc(0x206);
        accent_ibreve[uchar('e')] = mk_acc(0x207);
        accent_dgrave[uchar('I')] = mk_acc(0x208);
        accent_dgrave[uchar('i')] = mk_acc(0x209);
        accent_ibreve[uchar('I')] = mk_acc(0x20a);
        accent_ibreve[uchar('i')] = mk_acc(0x20b);
        accent_dgrave[uchar('O')] = mk_acc(0x20c);
        accent_dgrave[uchar('o')] = mk_acc(0x20d);
        accent_ibreve[uchar('O')] = mk_acc(0x20e);
        accent_ibreve[uchar('o')] = mk_acc(0x20f);

        accent_dgrave[uchar('R')] = mk_acc(0x210);
        accent_dgrave[uchar('r')] = mk_acc(0x211);
        accent_ibreve[uchar('R')] = mk_acc(0x212);
        accent_ibreve[uchar('r')] = mk_acc(0x213);
        accent_dgrave[uchar('U')] = mk_acc(0x214);
        accent_dgrave[uchar('u')] = mk_acc(0x215);
        accent_ibreve[uchar('U')] = mk_acc(0x216);
        accent_ibreve[uchar('u')] = mk_acc(0x217);
        // 218 219 220 221: ST with comma below
        accent_check[uchar('H')] = mk_acc(0x21e);
        accent_check[uchar('h')] = mk_acc(0x21f);

        // 0220 to  225 missing
        accent_dotabove[uchar('A')] = mk_acc(0x226);
        accent_dotabove[uchar('a')] = mk_acc(0x227);
        accent_cedille[uchar('E')]  = mk_acc(0x228);
        accent_cedille[uchar('e')]  = mk_acc(0x229);
        other_accent[Odiamacro_cc]  = mk_acc(0x22a);
        other_accent[odiamacro_cc]  = mk_acc(0x22b);
        other_accent[Otilmacro_cc]  = mk_acc(0x22c);
        other_accent[otilmacro_cc]  = mk_acc(0x22d);
        accent_dotabove[uchar('O')] = mk_acc(0x22e);
        accent_dotabove[uchar('o')] = mk_acc(0x22f);
        other_accent[Odotmacro_cc]  = mk_acc(0x230);
        other_accent[odotmacro_cc]  = mk_acc(0x231);
        accent_macron[uchar('Y')]   = mk_acc(0x232);
        accent_macron[uchar('y')]   = mk_acc(0x233);

        // later
        accent_rondunder[uchar('A')] = mk_acc(0x1e00);
        accent_rondunder[uchar('a')] = mk_acc(0x1e01);
        accent_dotabove[uchar('B')]  = mk_acc(0x1e02);
        accent_dotabove[uchar('b')]  = mk_acc(0x1e03);
        accent_dotunder[uchar('B')]  = mk_acc(0x1e04);
        accent_dotunder[uchar('b')]  = mk_acc(0x1e05);
        accent_barunder[uchar('B')]  = mk_acc(0x1e06);
        accent_barunder[uchar('b')]  = mk_acc(0x1e07);
        other_accent[Ccedilacute_cc] = mk_acc(0x1e08);
        other_accent[ccedilacute_cc] = mk_acc(0x1e09);
        accent_dotabove[uchar('D')]  = mk_acc(0x1e0a);
        accent_dotabove[uchar('d')]  = mk_acc(0x1e0b);
        accent_dotunder[uchar('D')]  = mk_acc(0x1e0c);
        accent_dotunder[uchar('d')]  = mk_acc(0x1e0d);
        accent_barunder[uchar('D')]  = mk_acc(0x1e0e);
        accent_barunder[uchar('d')]  = mk_acc(0x1e0f);

        accent_cedille[uchar('D')]    = mk_acc(0x1e10);
        accent_cedille[uchar('d')]    = mk_acc(0x1e11);
        accent_circunder[uchar('D')]  = mk_acc(0x1e12);
        accent_circunder[uchar('d')]  = mk_acc(0x1e13);
        other_accent[Ebargrave_cc]    = mk_acc(0x1e14);
        other_accent[ebargrave_cc]    = mk_acc(0x1e15);
        other_accent[Ebaracute_cc]    = mk_acc(0x1e16);
        other_accent[ebaracute_cc]    = mk_acc(0x1e17);
        accent_circunder[uchar('E')]  = mk_acc(0x1e18);
        accent_circunder[uchar('e')]  = mk_acc(0x1e19);
        accent_tildeunder[uchar('E')] = mk_acc(0x1e1a);
        accent_tildeunder[uchar('e')] = mk_acc(0x1e1b);
        other_accent[Ecedrond_cc]     = mk_acc(0x1e1c);
        other_accent[ecedrond_cc]     = mk_acc(0x1e1d);
        accent_dotabove[uchar('F')]   = mk_acc(0x1e1e);
        accent_dotabove[uchar('f')]   = mk_acc(0x1e1f);

        accent_macron[uchar('G')]   = mk_acc(0x1e20);
        accent_macron[uchar('g')]   = mk_acc(0x1e21);
        accent_dotabove[uchar('H')] = mk_acc(0x1e22);
        accent_dotabove[uchar('h')] = mk_acc(0x1e23);
        accent_dotunder[uchar('H')] = mk_acc(0x1e24);
        accent_dotunder[uchar('h')] = mk_acc(0x1e25);
        accent_trema[uchar('H')]    = mk_acc(0x1e26);
        accent_trema[uchar('h')]    = mk_acc(0x1e27);
        accent_cedille[uchar('H')]  = mk_acc(0x1e28);
        accent_cedille[uchar('h')]  = mk_acc(0x1e29);
        // 1e2a 122b H breve below
        accent_tildeunder[uchar('I')] = mk_acc(0x1e2c);
        accent_tildeunder[uchar('i')] = mk_acc(0x1e2d);
        other_accent[Itremaacute_cc]  = mk_acc(0x1e2e);
        other_accent[itremaacute_cc]  = mk_acc(0x1e2f);

        accent_acute[uchar('K')]     = mk_acc(0x1e30);
        accent_acute[uchar('k')]     = mk_acc(0x1e31);
        accent_dotunder[uchar('K')]  = mk_acc(0x1e32);
        accent_dotunder[uchar('k')]  = mk_acc(0x1e33);
        accent_barunder[uchar('K')]  = mk_acc(0x1e34);
        accent_barunder[uchar('k')]  = mk_acc(0x1e35);
        accent_dotunder[uchar('L')]  = mk_acc(0x1e36);
        accent_dotunder[uchar('l')]  = mk_acc(0x1e37);
        other_accent[Ldotbar_cc]     = mk_acc(0x1e38);
        other_accent[ldotbar_cc]     = mk_acc(0x1e39);
        accent_barunder[uchar('L')]  = mk_acc(0x1e3a);
        accent_barunder[uchar('l')]  = mk_acc(0x1e3b);
        accent_circunder[uchar('L')] = mk_acc(0x1e3c);
        accent_circunder[uchar('l')] = mk_acc(0x1e3d);
        accent_acute[uchar('M')]     = mk_acc(0x1e3e);
        accent_acute[uchar('m')]     = mk_acc(0x1e3f);

        accent_dotabove[uchar('M')]  = mk_acc(0x1e40);
        accent_dotabove[uchar('m')]  = mk_acc(0x1e41);
        accent_dotunder[uchar('M')]  = mk_acc(0x1e42);
        accent_dotunder[uchar('m')]  = mk_acc(0x1e43);
        accent_dotabove[uchar('N')]  = mk_acc(0x1e44);
        accent_dotabove[uchar('n')]  = mk_acc(0x1e45);
        accent_dotunder[uchar('N')]  = mk_acc(0x1e46);
        accent_dotunder[uchar('n')]  = mk_acc(0x1e47);
        accent_barunder[uchar('N')]  = mk_acc(0x1e48);
        accent_barunder[uchar('n')]  = mk_acc(0x1e49);
        accent_circunder[uchar('N')] = mk_acc(0x1e4a);
        accent_circunder[uchar('n')] = mk_acc(0x1e4b);
        other_accent[Otildeaigu_cc]  = mk_acc(0x1e4c);
        other_accent[otildeaigu_cc]  = mk_acc(0x1e4d);
        other_accent[Otildedp_cc]    = mk_acc(0x1e4e);
        other_accent[otildedp_cc]    = mk_acc(0x1e4f);

        other_accent[Obargrave_cc]  = mk_acc(0x1e50);
        other_accent[obargrave_cc]  = mk_acc(0x1e51);
        other_accent[Obaraigu_cc]   = mk_acc(0x1e52);
        other_accent[obaraigu_cc]   = mk_acc(0x1e53);
        accent_acute[uchar('P')]    = mk_acc(0x1e54);
        accent_acute[uchar('p')]    = mk_acc(0x1e55);
        accent_dotabove[uchar('P')] = mk_acc(0x1e56);
        accent_dotabove[uchar('p')] = mk_acc(0x1e57);
        accent_dotabove[uchar('R')] = mk_acc(0x1e58);
        accent_dotabove[uchar('r')] = mk_acc(0x1e59);
        accent_dotunder[uchar('R')] = mk_acc(0x1e5a);
        accent_dotunder[uchar('r')] = mk_acc(0x1e5b);
        other_accent[Rdotbar_cc]    = mk_acc(0x1e5c);
        other_accent[rdotbar_cc]    = mk_acc(0x1e5d);
        accent_barunder[uchar('R')] = mk_acc(0x1e5e);
        accent_barunder[uchar('r')] = mk_acc(0x1e5f);

        accent_dotabove[uchar('S')] = mk_acc(0x1e60);
        accent_dotabove[uchar('s')] = mk_acc(0x1e61);
        accent_dotunder[uchar('S')] = mk_acc(0x1e62);
        accent_dotunder[uchar('s')] = mk_acc(0x1e63);
        other_accent[Sdotacute_cc]  = mk_acc(0x1e64);
        other_accent[sdotacute_cc]  = mk_acc(0x1e65);
        other_accent[Sdotcaron_cc]  = mk_acc(0x1e66);
        other_accent[sdotcaron_cc]  = mk_acc(0x1e67);
        other_accent[Sdotdot_cc]    = mk_acc(0x1e68);
        other_accent[sdotdot_cc]    = mk_acc(0x1e69);
        accent_dotabove[uchar('T')] = mk_acc(0x1e6a);
        accent_dotabove[uchar('t')] = mk_acc(0x1e6b);
        accent_dotunder[uchar('T')] = mk_acc(0x1e6c);
        accent_dotunder[uchar('t')] = mk_acc(0x1e6d);
        accent_barunder[uchar('T')] = mk_acc(0x1e6e);
        accent_barunder[uchar('t')] = mk_acc(0x1e6f);

        accent_circunder[uchar('T')] = mk_acc(0x1e70);
        accent_circunder[uchar('t')] = mk_acc(0x1e71);
        // 1e72 1e73 etrange
        accent_tildeunder[uchar('U')] = mk_acc(0x1e74);
        accent_tildeunder[uchar('u')] = mk_acc(0x1e75);
        accent_circunder[uchar('U')]  = mk_acc(0x1e76);
        accent_circunder[uchar('u')]  = mk_acc(0x1e77);
        other_accent[Utildeacute_cc]  = mk_acc(0x1e78);
        other_accent[utildeacute_cc]  = mk_acc(0x1e79);
        other_accent[Udiamacro_cc]    = mk_acc(0x1e7a);
        other_accent[udiamacro_cc]    = mk_acc(0x1e7b);
        accent_tilde[uchar('V')]      = mk_acc(0x1e7c);
        accent_tilde[uchar('v')]      = mk_acc(0x1e7d);
        accent_dotunder[uchar('V')]   = mk_acc(0x1e7e);
        accent_dotunder[uchar('v')]   = mk_acc(0x1e7f);

        accent_grave[uchar('W')]    = mk_acc(0x1e80);
        accent_grave[uchar('w')]    = mk_acc(0x1e81);
        accent_acute[uchar('W')]    = mk_acc(0x1e82);
        accent_acute[uchar('w')]    = mk_acc(0x1e83);
        accent_trema[uchar('W')]    = mk_acc(0x1e84);
        accent_trema[uchar('w')]    = mk_acc(0x1e85);
        accent_dotabove[uchar('W')] = mk_acc(0x1e86);
        accent_dotabove[uchar('w')] = mk_acc(0x1e87);
        accent_dotunder[uchar('W')] = mk_acc(0x1e88);
        accent_dotunder[uchar('w')] = mk_acc(0x1e89);
        accent_dotabove[uchar('X')] = mk_acc(0x1e8a);
        accent_dotabove[uchar('x')] = mk_acc(0x1e8b);
        accent_trema[uchar('X')]    = mk_acc(0x1e8c);
        accent_trema[uchar('x')]    = mk_acc(0x1e8d);
        accent_dotabove[uchar('Y')] = mk_acc(0x1e8e);
        accent_dotabove[uchar('y')] = mk_acc(0x1e8f);

        accent_cir[uchar('Z')]      = mk_acc(0x1e90);
        accent_cir[uchar('z')]      = mk_acc(0x1e91);
        accent_dotunder[uchar('Z')] = mk_acc(0x1e92);
        accent_dotunder[uchar('z')] = mk_acc(0x1e93);
        accent_barunder[uchar('Z')] = mk_acc(0x1e94);
        accent_barunder[uchar('z')] = mk_acc(0x1e95);
        accent_barunder[uchar('h')] = mk_acc(0x1e96);
        accent_trema[uchar('t')]    = mk_acc(0x1e97);
        accent_ring[uchar('w')]     = mk_acc(0x1e98);
        accent_ring[uchar('y')]     = mk_acc(0x1e99);

        // hole
        accent_dotunder[uchar('A')]  = mk_acc(0x1ea0);
        accent_dotunder[uchar('a')]  = mk_acc(0x1ea1);
        accent_hook[uchar('A')]      = mk_acc(0x1ea2);
        accent_hook[uchar('a')]      = mk_acc(0x1ea3);
        other_accent[Aciracute_cc]   = mk_acc(0x1ea4);
        other_accent[aciracute_cc]   = mk_acc(0x1ea5);
        other_accent[Acirgrave_cc]   = mk_acc(0x1ea6);
        other_accent[acirgrave_cc]   = mk_acc(0x1ea7);
        other_accent[Acirxx_cc]      = mk_acc(0x1ea8);
        other_accent[acirxx_cc]      = mk_acc(0x1ea9);
        other_accent[Acirtilde_cc]   = mk_acc(0x1eaa);
        other_accent[acirtilde_cc]   = mk_acc(0x1eab);
        other_accent[Acirdot_cc]     = mk_acc(0x1eac);
        other_accent[acirdot_cc]     = mk_acc(0x1ead);
        other_accent[Abreveacute_cc] = mk_acc(0x1eae);
        other_accent[abreveacute_cc] = mk_acc(0x1eaf);

        other_accent[Abrevegrave_cc] = mk_acc(0x1eb0);
        other_accent[abrevegrave_cc] = mk_acc(0x1eb1);
        other_accent[Abrevexx_cc]    = mk_acc(0x1eb2);
        other_accent[abrevexx_cc]    = mk_acc(0x1eb3);
        other_accent[Abrevetilde_cc] = mk_acc(0x1eb4);
        other_accent[abrevetilde_cc] = mk_acc(0x1eb5);
        other_accent[Abrevedot_cc]   = mk_acc(0x1eb6);
        other_accent[abrevedot_cc]   = mk_acc(0x1eb7);
        accent_dotunder[uchar('E')]  = mk_acc(0x1eb8);
        accent_dotunder[uchar('e')]  = mk_acc(0x1eb9);
        accent_hook[uchar('E')]      = mk_acc(0x1eba);
        accent_hook[uchar('e')]      = mk_acc(0x1ebb);
        accent_tilde[uchar('E')]     = mk_acc(0x1ebc);
        accent_tilde[uchar('e')]     = mk_acc(0x1ebd);
        other_accent[Eciracute_cc]   = mk_acc(0x1ebe);
        other_accent[eciracute_cc]   = mk_acc(0x1ebf);

        other_accent[Ecirgrave_cc]  = mk_acc(0x1ec0);
        other_accent[ecirgrave_cc]  = mk_acc(0x1ec1);
        other_accent[Ecirxx_cc]     = mk_acc(0x1ec2);
        other_accent[ecirxx_cc]     = mk_acc(0x1ec3);
        other_accent[Ecirtilde_cc]  = mk_acc(0x1ec4);
        other_accent[ecirtilde_cc]  = mk_acc(0x1ec5);
        other_accent[Ecirdot_cc]    = mk_acc(0x1ec6);
        other_accent[ecirdot_cc]    = mk_acc(0x1ec7);
        accent_hook[uchar('I')]     = mk_acc(0x1ec8);
        accent_hook[uchar('i')]     = mk_acc(0x1ec9);
        accent_dotunder[uchar('I')] = mk_acc(0x1eca);
        accent_dotunder[uchar('i')] = mk_acc(0x1ecb);
        accent_dotunder[uchar('O')] = mk_acc(0x1ecc);
        accent_dotunder[uchar('o')] = mk_acc(0x1ecd);
        accent_hook[uchar('O')]     = mk_acc(0x1ece);
        accent_hook[uchar('o')]     = mk_acc(0x1ecf);

        other_accent[Ociracute_cc] = mk_acc(0x1ed0);
        other_accent[ociracute_cc] = mk_acc(0x1ed1);
        other_accent[Ocirgrave_cc] = mk_acc(0x1ed2);
        other_accent[ocirgrave_cc] = mk_acc(0x1ed3);
        other_accent[Ocirxx_cc]    = mk_acc(0x1ed4);
        other_accent[ocirxx_cc]    = mk_acc(0x1ed5);
        other_accent[Ocirtilde_cc] = mk_acc(0x1ed6);
        other_accent[ocirtilde_cc] = mk_acc(0x1ed7);
        other_accent[Ocirdot_cc]   = mk_acc(0x1ed8);
        other_accent[ocirdot_cc]   = mk_acc(0x1ed9);
        other_accent[Oacuteyy_cc]  = mk_acc(0x1eda);
        other_accent[oacuteyy_cc]  = mk_acc(0x1edb);
        other_accent[Ograveyy_cc]  = mk_acc(0x1edc);
        other_accent[ograveyy_cc]  = mk_acc(0x1edd);
        other_accent[Oxxyy_cc]     = mk_acc(0x1ede);
        other_accent[oxxyy_cc]     = mk_acc(0x1edf);

        other_accent[Otildeyy_cc]   = mk_acc(0x1ee0);
        other_accent[otildeyy_cc]   = mk_acc(0x1ee1);
        other_accent[Odotyy_cc]     = mk_acc(0x1ee2);
        other_accent[odotyy_cc]     = mk_acc(0x1ee3);
        accent_dotunder[uchar('U')] = mk_acc(0x1ee4);
        accent_dotunder[uchar('u')] = mk_acc(0x1ee5);
        accent_hook[uchar('U')]     = mk_acc(0x1ee6);
        accent_hook[uchar('u')]     = mk_acc(0x1ee7);
        other_accent[Uacuteyy_cc]   = mk_acc(0x1ee8);
        other_accent[uacuteyy_cc]   = mk_acc(0x1ee9);
        other_accent[Ugraveyy_cc]   = mk_acc(0x1eea);
        other_accent[ugraveyy_cc]   = mk_acc(0x1eeb);
        other_accent[Uxxyy_cc]      = mk_acc(0x1eec);
        other_accent[uxxyy_cc]      = mk_acc(0x1eed);
        other_accent[Utildeyy_cc]   = mk_acc(0x1eee);
        other_accent[utildeyy_cc]   = mk_acc(0x1eef);

        other_accent[Udotyy_cc]     = mk_acc(0x1ef0);
        other_accent[udotyy_cc]     = mk_acc(0x1ef1);
        accent_grave[uchar('Y')]    = mk_acc(0x1ef2);
        accent_grave[uchar('y')]    = mk_acc(0x1ef3);
        accent_dotunder[uchar('Y')] = mk_acc(0x1ef4);
        accent_dotunder[uchar('y')] = mk_acc(0x1ef5);
        accent_hook[uchar('Y')]     = mk_acc(0x1ef6);
        accent_hook[uchar('y')]     = mk_acc(0x1ef7);
        accent_tilde[uchar('Y')]    = mk_acc(0x1ef8);
        accent_tilde[uchar('y')]    = mk_acc(0x1ef9);

        other_accent[unused_accent_even_cc] = Token(0);
        other_accent[unused_accent_odd_cc]  = Token(0);
        special_double[0]                   = hash_table.locate("textsubgrave");
        special_double[1]                   = hash_table.locate("textgravedot");
        special_double[2]                   = hash_table.locate("textsubacute");
        special_double[3]                   = hash_table.locate("textacutemacron");
        special_double[4]                   = hash_table.locate("textsubcircum");
        special_double[5]                   = hash_table.locate("textcircumdot");
        special_double[6]                   = hash_table.locate("textsubtilde");
        special_double[7]                   = hash_table.locate("texttildedot");
        special_double[8]                   = hash_table.locate("textsubumlaut");
        special_double[9]                   = hash_table.locate("textdoublegrave");
        special_double[10]                  = hash_table.locate("textsubring");
        special_double[11]                  = hash_table.locate("textringmacron");
        special_double[12]                  = hash_table.locate("textsubwedge");
        special_double[13]                  = hash_table.locate("textacutewedge");
        special_double[14]                  = hash_table.locate("textsubarch");
        special_double[15]                  = hash_table.locate("textbrevemacron");
        special_double[16]                  = hash_table.locate("textsubbar");
        special_double[17]                  = hash_table.locate("textsubdot");
        special_double[18]                  = hash_table.locate("textdotacute");
        special_double[19]                  = hash_table.locate("textbottomtiebar");
    }

    /// This creates the table with all the names.
    void make_names() { the_names.boot(); }

    auto special_double_acc(unsigned chr, unsigned acc) -> Token {
        if (acc == '`' && chr == '*') return special_double[0];
        if (acc == '`' && chr == '.') return special_double[1];
        if (acc == '\'' && chr == '*') return special_double[2];
        if (acc == '\'' && chr == '=') return special_double[3];
        if (acc == '^' && chr == '*') return special_double[4];
        if (acc == '^' && chr == '.') return special_double[5];
        if (acc == '~' && chr == '*') return special_double[6];
        if (acc == '~' && chr == '.') return special_double[7];
        if (acc == '"' && chr == '*') return special_double[8];
        if (acc == 'H' && chr == '*') return special_double[9];
        if (acc == 'r' && chr == '*') return special_double[10];
        if (acc == 'r' && chr == '=') return special_double[11];
        if (acc == 'v' && chr == '*') return special_double[12];
        if (acc == 'v' && chr == '\'') return special_double[13];
        if (acc == 'u' && chr == '*') return special_double[14];
        if (acc == 'u' && chr == '=') return special_double[15];
        if (acc == '=' && chr == '*') return special_double[16];
        if (acc == '.' && chr == '*') return special_double[17];
        if (acc == '.' && chr == '\'') return special_double[18];
        if (acc == 't' && chr == '*') return special_double[19];
        return Token();
    }

    // Creates some little constants.
    void make_constants() {
        hash_table.OB_token      = make_char_token('{', 1);
        hash_table.CB_token      = make_char_token('}', 2);
        hash_table.dollar_token  = make_char_token('$', 3);
        hash_table.hat_token     = Token(hat_t_offset, '^');
        hash_table.zero_token    = Token(other_t_offset, '0');
        hash_table.comma_token   = Token(other_t_offset, ',');
        hash_table.equals_token  = Token(other_t_offset, '=');
        hash_table.dot_token     = Token(other_t_offset, '.');
        hash_table.star_token    = Token(other_t_offset, '*');
        hash_table.plus_token    = Token(other_t_offset, '+');
        hash_table.minus_token   = Token(other_t_offset, '-');
        hash_table.space_token   = Token(space_token_val);
        hash_table.newline_token = Token(newline_token_val);
    }

    // In ref_list, we have  (e,v), (e,v), (e,v) etc
    // where E is the xid of a <ref> element, and V is an entry in the
    // hash table of the label. After the translation is complete,
    // we know the value of the label, and can add the attribute target=...
    void check_all_ids() {
        for (auto &i : ref_list) {
            auto        E = i.first;
            std::string V = i.second;
            auto *      L = labinfo(V);
            if (!L->defined) {
                Logger::finish_seq();
                log_and_tty << "Error signaled in postprocessor\n"
                            << "undefined label `" << V << "' (first use at line " << L->lineno << " in file " << L->filename << ")";
                Xid(E).add_attribute(the_names["target"], V);
                std::string B = L->id;
                for (auto &removed_label : removed_labels) {
                    if (removed_label.second == B) log_and_tty << "\n(Label was removed with `" << removed_label.first << "')";
                    break;
                }
                log_and_tty << "\n";
                nb_errs++;
            }
            std::string B = L->id;
            if (!B.empty()) Xid(E).add_attribute(the_names["target"], B);
        }
    }

    void finish_color() {
        if (std::none_of(all_colors.begin(), all_colors.end(), [](const auto &c) { return c.used; })) return;
        Xml *res = new Xml(std::string("colorpool"), nullptr);
        for (auto &color : all_colors)
            if (color.used) {
                res->push_back_unless_nullptr(&color.xval);
                res->add_nl();
            }
        the_stack.document_element()->replace_first(res);
    }
} // namespace

// This command puts a double accent on a letter.
// The result is a token (character or command) from
// the other_accent table defined above; or empty token in case of failure.
// Assumes that lower case letters follow upper case ones.
auto accent_ns::fetch_double_accent(int a, int acc3) -> Token {
    if (a == 'U') return other_accent[double_u_accent(acc3)];
    if (a == 'u') return other_accent[double_u_accent(acc3) + 1];
    if (a == 'A') return other_accent[double_a_accent(acc3)];
    if (a == 'a') return other_accent[double_a_accent(acc3) + 1];
    if (a == 'O') return other_accent[double_o_accent(acc3)];
    if (a == 'o') return other_accent[double_o_accent(acc3) + 1];
    if (a == 'E') return other_accent[double_e_accent(acc3)];
    if (a == 'e') return other_accent[double_e_accent(acc3) + 1];
    return other_accent[double_other_accent(a, acc3)];
}

// In case of error, we add the current line number as attribute
// via this function
auto Parser::cur_line_to_istring() const -> std::string { return std::string(fmt::format("{}", get_cur_line())); }

// Enter a new image file, if ok is false, do not increase the occ count
void Parser::enter_file_in_table(const std::string &nm, bool ok) {
    auto i = std::find_if(the_images.begin(), the_images.end(), [&nm](const auto &X) { return X.name == nm; });
    if (i != the_images.end()) {
        if (ok) i->occ++;
        return;
    }
    the_images.emplace_back(nm, ok ? 1 : 0);
}

// finish handling the images,
void Parser::finish_images() {
    if (the_images.empty()) return;
    std::string   name = file_name + ".img";
    auto          wn   = get_out_dir(name);
    std::ofstream fp(wn);
    fp << "# images info, 1=ps, 2=eps, 4=epsi, 8=epsf, 16=pdf, 32=png, 64=gif\n";
    Buffer B1, B2;
    for (auto &the_image : the_images) {
        if (the_image.occ != 0) {
            the_image.check_existence();
            the_image.check(B1, B2);
            fp << the_image;
        }
    }
    if (the_images.empty())
        spdlog::info("There was no image.");
    else
        spdlog::info("There were {} images.", the_images.size());
    if (!B1.empty()) spdlog::warn("Following images have multiple PS source: {}.", B1);
    if (!B2.empty()) spdlog::warn("Following images not defined: {}.", B2);
}

// In verbatim mode, all chars are of catcode 12, with these exceptions.
// that are of catcode 11.
void Parser::boot_verbatim() {
    Token *T = verbatim_chars.data();
    for (unsigned i = 0; i < nb_characters; i++) T[i] = Token(other_t_offset, char32_t(i));
    T[unsigned(' ')] = hash_table.tilda_token;
    mk_letter(T, '\'');
    mk_letter(T, '`');
    mk_letter(T, '-');
    mk_letter(T, '<');
    mk_letter(T, '>');
    mk_letter(T, '~');
    mk_letter(T, '&');
    mk_letter(T, ':');
    mk_letter(T, ';');
    mk_letter(T, '?');
    mk_letter(T, '!');
    mk_letter(T, '"');
    mk_letter(T, uchar('\253'));
    mk_letter(T, uchar('\273'));
}

// The list of characters for which \xspace should not add space.
// Only 7bit characters are in the list
void Parser::boot_xspace() {
    std::fill(ok_for_xspace.begin(), ok_for_xspace.end(), false);
    ok_for_xspace[uchar('.')]  = true;
    ok_for_xspace[uchar('!')]  = true;
    ok_for_xspace[uchar(',')]  = true;
    ok_for_xspace[uchar(':')]  = true;
    ok_for_xspace[uchar(';')]  = true;
    ok_for_xspace[uchar('?')]  = true;
    ok_for_xspace[uchar('/')]  = true;
    ok_for_xspace[uchar('\'')] = true;
    ok_for_xspace[uchar(')')]  = true;
    ok_for_xspace[uchar('-')]  = true;
    ok_for_xspace[uchar('~')]  = true;
}

// This computes the current date (first thing done after printing the banner)
// stores the date in the table of equivalents.
void Parser::boot_time() {
    time_t xx = 0;
    time(&xx);
    struct tm *x                   = localtime(&xx);
    int        sec                 = x->tm_sec;
    int        min                 = x->tm_min;
    int        hour                = x->tm_hour;
    int        the_time            = min + hour * 60;
    int        year                = x->tm_year + 1900;
    int        month               = x->tm_mon + 1;
    int        day                 = x->tm_mday;
    eqtb_int_table[time_code].val  = the_time;
    eqtb_int_table[day_code].val   = day;
    eqtb_int_table[month_code].val = month;
    eqtb_int_table[year_code].val  = year;
    std::srand(to_unsigned(sec + 60 * (min + 60 * (hour + 24 * (day + 31 * month)))));
    auto      short_date   = fmt::format("{}/{:02d}/{:02d}", year, month, day);
    auto      long_date    = fmt::format("{} {:02d}:{:02d}:{:02d}", short_date, hour, min, sec);
    TokenList today_tokens = Buffer{long_date}.str_toks(nlt_space);
    new_prim("today", today_tokens);
    the_main.short_date = short_date;
}

// Installs the default language.
void Parser::set_default_language(int v) {
    eqtb_int_table[language_code] = {v, 1};
    set_def_language_num(v);
}

// Locations 2N and 2N+1 are lowercase/uppercase equivalent.
void Parser::boot_uclc() {
    uclc_list[0]  = hash_table.locate("oe");
    uclc_list[1]  = hash_table.locate("OE");
    uclc_list[2]  = hash_table.locate("o");
    uclc_list[3]  = hash_table.locate("O");
    uclc_list[4]  = hash_table.locate("ae");
    uclc_list[5]  = hash_table.locate("AE");
    uclc_list[6]  = hash_table.locate("dh");
    uclc_list[7]  = hash_table.locate("DH");
    uclc_list[8]  = hash_table.locate("dj");
    uclc_list[9]  = hash_table.locate("DJ");
    uclc_list[10] = hash_table.locate("l");
    uclc_list[11] = hash_table.locate("L");
    uclc_list[12] = hash_table.locate("ng");
    uclc_list[13] = hash_table.locate("NG");
    uclc_list[14] = hash_table.locate("ss");
    uclc_list[15] = hash_table.locate("SS");
    uclc_list[16] = hash_table.locate("th");
    uclc_list[17] = hash_table.locate("TH");
    uclc_list[18] = hash_table.locate("i");
    uclc_list[19] = hash_table.locate("I");
    uclc_list[20] = hash_table.locate("j");
    uclc_list[21] = hash_table.locate("J");
}

// ------------------------------------------------------------

// This function is called after all primitives are constructed.
// We add some new macros.

void Parser::load_latex() {
    make_uclc_table();
    TokenList body;
    new_prim("@empty", body);
    hash_table.empty_token = hash_table.locate("@empty");
    // this is like \let\bgroup={, etc
    hash_table.primitive("bgroup", open_catcode, subtypes('{'));
    hash_table.primitive("egroup", close_catcode, subtypes('}'));
    hash_table.primitive("sp", hat_catcode, subtypes('^'));
    hash_table.primitive("sb", underscore_catcode, subtypes('_'));
    hash_table.primitive("@sptoken", space_catcode, subtypes(' '));
    eqtb_int_table[escapechar_code].val   = '\\';
    eqtb_dim_table[unitlength_code].val   = ScaledInt(1 << 16);
    eqtb_dim_table[textwidth_code].val    = ScaledInt(427 << 16);
    eqtb_dim_table[linewidth_code].val    = ScaledInt(427 << 16);
    eqtb_dim_table[textwidth_code].val    = ScaledInt(427 << 16);
    eqtb_dim_table[columnwidth_code].val  = ScaledInt(427 << 16);
    eqtb_dim_table[textheight_code].val   = ScaledInt(570 << 16);
    eqtb_int_table[pretolerance_code].val = 100;
    eqtb_int_table[tolerance_code].val    = 200;

    hash_table.eval_let("endgraf", "par");
    hash_table.eval_let("@@par", "par");
    hash_table.eval_let("endline", "cr");
    hash_table.eval_let("epsffile", "epsfbox");
    hash_table.eval_let("epsfig", "psfig");
    hash_table.eval_let("newsavebox", "newbox");
    hash_table.eval_let("@ybegintheorem", "@begintheorem");
    hash_table.eval_let("@@href", "Href");
    hash_table.eval_let("@@italiccorr", "/");
    hash_table.eval_let("verbatimfont", "tt");
    hash_table.eval_let("verbatimnumberfont", "small");
    hash_table.eval_let("empty", "@empty");
    hash_table.eval_let("verbprefix", "@empty");
    hash_table.eval_let("verbatimprefix", "@empty");
    hash_table.eval_let("urlfont", "@empty");
    hash_table.eval_let("@currentlabel", "@empty");
    hash_table.eval_let("bye", "endinput");
    hash_table.eval_let("sortnoop", "@gobble");
    hash_table.eval_let("noopsort", "@gobble");
    hash_table.eval_let("chaptermark", "@gobble");
    hash_table.eval_let("sectionmark", "@gobble");
    hash_table.eval_let("subsectionmark", "@gobble");
    hash_table.eval_let("subsubsectionmark", "@gobble");
    hash_table.eval_let("paragraphmark", "@gobble");
    hash_table.eval_let("subparagraphmark", "@gobble");
    hash_table.eval_let("dotsc", "cdots");
    hash_table.eval_let("dotso", "cdots");
    hash_table.eval_let("dotsb", "cdots");
    hash_table.eval_let("dotsi", "cdots");
    hash_table.eval_let("dotsm", "cdots");
    hash_table.eval_let("mathbm", "mathbb");
    hash_table.eval_let("mathbbm", "mathbb");
    hash_table.eval_let("pounds", "textsterling");
    hash_table.eval_let("@itemlabel", "relax");
    hash_table.eval_let("default@ds", "OptionNotUsed");
    hash_table.eval_let("texttwelveudash", "textemdash");
    hash_table.eval_let("textthreequartersemdash", "textemdash");
    hash_table.eval_let("textminus", "textemdash");
    hash_table.eval_let("texorpdfstring", "@firstoftwo");
    hash_table.eval_let("m@th", "relax");
    hash_table.eval_let("mathdollar", "$");
    hash_table.eval_let("mathsterling", "pounds");
    hash_table.eval_let("varcopyright", "copyright");
    hash_table.eval_let("@typeset@protect", "relax");
    hash_table.eval_let("protect", "relax");
    hash_table.eval_let("topfigrule", "relax");
    hash_table.eval_let("botfigrule", "relax");
    hash_table.eval_let("dblfigrule", "relax");
    hash_table.eval_let("mathalpha", "relax");
    hash_table.eval_let("@iden", "@firstofone");
    hash_table.eval_let("reset@font", "normalfont");

    // commands like \countdef\foo=\bar
    shorthand_gdefine(count_def_code, "count@", 255);
    shorthand_gdefine(count_def_code, "c@page", 0);
    shorthand_gdefine(dimen_def_code, "dimen@", 0);
    shorthand_gdefine(dimen_def_code, "dimen@i", 1);
    shorthand_gdefine(dimen_def_code, "dimen@ii", 2);
    shorthand_gdefine(char_def_code, "@ne", 1);
    shorthand_gdefine(char_def_code, "tw@", 2);
    shorthand_gdefine(char_def_code, "thr@@", 3);
    shorthand_gdefine(char_def_code, "sixt@@n", 16);
    shorthand_gdefine(char_def_code, "@cclv", 255);
    shorthand_gdefine(math_char_def_code, "@cclvi", 256);
    shorthand_gdefine(math_char_def_code, "@m", 1000);
    shorthand_gdefine(math_char_def_code, "@M", 10000);
    shorthand_gdefine(math_char_def_code, "@Mi", 10001);
    shorthand_gdefine(math_char_def_code, "@Mii", 10002);
    shorthand_gdefine(math_char_def_code, "@Miii", 10003);
    shorthand_gdefine(math_char_def_code, "@Miv", 10004);
    shorthand_gdefine(math_char_def_code, "@MM", 20000);
    shorthand_gdefine(char_def_code, "active", 13);
    shorthand_gdefine(toks_def_code, "toks@", 0);
    shorthand_gdefine(skip_def_code, "skip@", 0);
    // commands like \newdimen\foo
    make_token("epsfxsize");
    new_constant(newdimen_code);
    make_token("epsfysize");
    new_constant(newdimen_code);
    make_token("z@");
    new_constant(newdimen_code);
    make_token("p@");
    new_constant(newdimen_code);
    make_token("oddsidemargin");
    new_constant(newdimen_code);
    make_token("evensidemargin");
    new_constant(newdimen_code);
    make_token("leftmargin");
    new_constant(newdimen_code);
    make_token("rightmargin");
    new_constant(newdimen_code);
    make_token("leftmargini");
    new_constant(newdimen_code);
    make_token("leftmarginii");
    new_constant(newdimen_code);
    make_token("leftmarginiii");
    new_constant(newdimen_code);
    make_token("leftmarginiv");
    new_constant(newdimen_code);
    make_token("leftmarginv");
    new_constant(newdimen_code);
    make_token("leftmarginvi");
    new_constant(newdimen_code);
    make_token("itemindent");
    new_constant(newdimen_code);
    make_token("labelwidth");
    new_constant(newdimen_code);
    make_token("fboxsep");
    new_constant(newdimen_code);
    make_token("fboxrule");
    new_constant(newdimen_code);
    make_token("arraycolsep");
    new_constant(newdimen_code);
    make_token("tabcolsep");
    new_constant(newdimen_code);
    make_token("arrayrulewidth");
    new_constant(newdimen_code);
    make_token("doublerulesep");
    new_constant(newdimen_code);
    make_token("@tempdima");
    new_constant(newdimen_code);
    make_token("@tempdimb");
    new_constant(newdimen_code);
    make_token("@tempdimc");
    new_constant(newdimen_code);
    make_token("footnotesep");
    new_constant(newdimen_code);
    make_token("topmargin");
    new_constant(newdimen_code);
    make_token("headheight");
    new_constant(newdimen_code);
    make_token("headsep");
    new_constant(newdimen_code);
    make_token("footskip");
    new_constant(newdimen_code);
    make_token("columnsep");
    new_constant(newdimen_code);
    make_token("columnseprule");
    new_constant(newdimen_code);
    make_token("marginparwidth");
    new_constant(newdimen_code);
    make_token("marginparsep");
    new_constant(newdimen_code);
    make_token("marginparpush");
    new_constant(newdimen_code);
    make_token("maxdimen");
    new_constant(newdimen_code);
    make_token("normallineskiplimit");
    new_constant(newdimen_code);
    make_token("jot");
    new_constant(newdimen_code);
    make_token("paperheight");
    new_constant(newdimen_code);
    make_token("paperwidth");
    new_constant(newdimen_code);
    // commands like \newskip\foo
    make_token("topsep");
    new_constant(newlength_code);
    make_token("partopsep");
    new_constant(newlength_code);
    make_token("itemsep");
    new_constant(newlength_code);
    make_token("labelsep");
    new_constant(newlength_code);
    make_token("parsep");
    new_constant(newlength_code);
    make_token("fill");
    new_constant(newlength_code);
    make_token("@tempskipa");
    new_constant(newlength_code);
    make_token("@tempskipb");
    new_constant(newlength_code);
    make_token("@flushglue");
    new_constant(newlength_code);
    make_token("listparindent");
    new_constant(newlength_code);
    make_token("hideskip");
    new_constant(newlength_code);
    make_token("z@skip");
    new_constant(newlength_code);
    make_token("normalbaselineskip");
    new_constant(newlength_code);
    make_token("normallineskip");
    new_constant(newlength_code);
    make_token("smallskipamount");
    new_constant(newlength_code);
    make_token("medskipamount");
    new_constant(newlength_code);
    make_token("bigskipamount");
    new_constant(newlength_code);
    make_token("floatsep");
    new_constant(newlength_code);
    make_token("textfloatsep");
    new_constant(newlength_code);
    make_token("intextsep");
    new_constant(newlength_code);
    make_token("dblfloatsep");
    new_constant(newlength_code);
    make_token("dbltextfloatsep");
    new_constant(newlength_code);
    // commands like \newcounter\foo
    shorthand_gdefine(count_def_code, "m@ne", 20);
    counter_boot("FancyVerbLine", ""); // hard-coded 21
    counter_boot("enumi", "");
    counter_boot("enumii", "");
    counter_boot("enumiii", "");
    counter_boot("enumiv", "");
    counter_boot("enumv", "");
    counter_boot("enumvi", "");
    counter_boot("enumvii", "");
    counter_boot("enumviii", "");
    counter_boot("enumix", "");
    counter_boot("enumx", "");
    counter_boot("Enumi", "");
    counter_boot("Enumii", "");
    counter_boot("Enumiii", "");
    counter_boot("Enumiv", "");
    counter_boot("Enumv", "");
    counter_boot("Enumvi", "");
    counter_boot("Enumvii", "");
    counter_boot("Enumviii", "");
    counter_boot("Enumix", "");
    counter_boot("Enumx", "");
    counter_boot("tocdepth", ""); // This is 42 ...
    counter_boot("footnote", "");
    counter_boot("part", "");
    counter_boot("chapter", "");
    counter_boot("section", "chapter");
    counter_boot("subsection", "section");
    counter_boot("subsubsection", "subsection");
    counter_boot("paragraph", "subsubsection");
    counter_boot("subparagraph", "paragraph");
    counter_boot("mpfootnote", "");
    counter_boot("table", "");
    counter_boot("figure", "");
    counter_boot("subfigure", "figure");
    counter_boot("equation", "");
    equation_ctr_pos = allocation_table[newcount_code] + count_reg_offset;
    counter_boot("parentequation", "");

    // \newcount
    make_token("c@bottomnumber");
    new_constant(newcount_code);
    make_token("c@topnumber");
    new_constant(newcount_code);
    make_token("@tempcnta");
    new_constant(newcount_code);
    make_token("@tempcntb");
    new_constant(newcount_code);
    make_token("c@totalnumber");
    new_constant(newcount_code);
    make_token("c@dbltopnumber");
    new_constant(newcount_code);
    make_token("interfootnotelinepenalty");
    new_constant(newcount_code);
    make_token("interdisplaylinepenalty");
    new_constant(newcount_code);
    // \newtoks
    make_token("@temptokena");
    new_constant(newtoks_code);
    // \newbox
    make_token("@tempboxa");
    new_constant(newbox_code);
    make_token("voidb@x");
    new_constant(newbox_code);
    // \newif
    make_token("ifin@");
    M_newif();
    make_token("if@tempswa");
    M_newif();

    // Define now some other macros
    new_prim("lq", "`");
    new_prim("rq", "'");
    new_prim("lbrack", "[");
    new_prim("rbrack", "]");
    new_prim("xscale", "1.0");
    new_prim("yscale", "1.0");
    new_prim("xscaley", "0.0");
    new_prim("yscalex", "0.0");
    new_prim("@height", "height");
    new_prim("@depth", "depth");
    new_prim("@width", "width");
    new_prim("@plus", "plus");
    new_prim("@minus", "minus");
    new_prim("I", "I");
    new_prim("J", "J");
    new_prim("textfraction", ".2");
    new_prim("floatpagefraction", ".5");
    new_prim("dblfloatpagefraction", ".5");
    new_prim("bottomfraction", ".3");
    new_prim("dbltopfraction", ".7");
    new_prim("topfraction", ".7");
    new_prim("space", " ");
    hash_table.eval_let("@headercr", "space");
    new_prim("cl@page", "");
    new_prim("refname", "Bibliography");
    new_prim("footcitesep", ", ");
    new_prim("citepunct", ", ");
    new_prim("encodingdefault", "T1");
    new_prim("familydefault", "cmr");
    new_prim("seriesdefault", "m");
    new_prim("shapedefault", "n");
    new_prim("fmtname", "Tralics");
    new_prim("@vpt", "5");
    new_prim("@vipt", "6");
    new_prim("@viipt", "7");
    new_prim("@viiipt", "8");
    new_prim("@ixpt", "9");
    new_prim("@xpt", "10");
    new_prim("@xipt", "10.95");
    new_prim("@xiipt", "12");
    new_prim("@xivpt", "14.4");
    new_prim("@xviipt", "17.28");
    new_prim("@xxpt", "20.74");
    new_prim("@xxvpt", "24.88");
    new_prim("baselinestretch", "1");
    new_prim("arraystretch", "1");
    new_prim("NAT@open", "(");
    new_prim("NAT@sep", ";");
    new_prim("NAT@close", ")");
    new_prim("NAT@cmt", ", ");
    {
        TokenList L;
        L.push_back(Token(letter_t_offset, char32_t('&')));
        new_prim("amp", L);
    }
    more_bootstrap();
    LineList L;
    L.insert("%% Begin bootstrap commands for latex");
    eqtb_int_table[uchar('@')].val = 11; // this is \makeatletter
    // initialise counters, dimen etc
    L.insert("\\@flushglue = 0pt plus 1fil");
    L.insert("\\hideskip =-1000pt plus 1fill");
    L.insert("\\smallskipamount=3pt plus 1pt minus 1pt\n");
    L.insert("\\medskipamount=6pt plus 2pt minus 2pt\n");
    L.insert("\\bigskipamount=12pt plus 4pt minus 4pt\n");
    L.insert(R"(\z@=0pt\p@=1pt\m@ne=-1 \fboxsep = 3pt %)");
    L.insert(R"(\c@page=1 \fill = 0pt plus 1fill \setcounter{tocdepth}{2})");
    L.insert("\\paperheight=297mm\\paperwidth=210mm");
    L.insert("\\jot=3pt\\maxdimen=16383.99999pt");
    // Other definitions
    L.insert(R"(\def\newfont#1#2{\font#1=#2\relax})");
    L.insert(R"(\def\symbol#1{\char #1\relax})");
    L.insert(R"({\catcode`\&=13\global\def&{\char38 }}%)");
    L.insert(R"(\catcode`\^^L=13 \outer\def^^L{\par})");
    L.insert(R"(\def~{\nobreakspace}\def^^a0{\nobreakspace})");
    L.insert(R"(\newenvironment{cases}{\left\{\begin{array}{ll}}{\end{array}\right.}%)");
    L.insert(R"(\def\markboth#1#2{\gdef\@themark{{#1}}\mark{{#1}{#2}}})");
    L.insert(R"(\def\markright#1{\expandafter\markboth\@themark{#1}})");
    L.insert(R"(\def\stretch#1{\z@ \@plus #1fill\relax})");
    L.insert(R"(\theoremstyle{plain}\theoremheaderfont{\bfseries})");
    L.insert(R"(\def\@namedef#1{\expandafter\def\csname #1\endcsname})");
    L.insert(R"(\def\@nameuse#1{\csname #1\endcsname})");
    L.insert(R"(\def\@arabic#1{\number #1})");
    L.insert(R"(\def\@roman#1{\romannumeral#1})");
    L.insert(R"(\def\@Roman#1{\Romannumeral#1})");
    L.insert(R"(\def\@nomathswiii#1#2{\@nomathsws#1\@nomathswm #2\@nomathswe})");
    L.insert(R"(\def\@nomathswiv#1#2#3{\@nomathswiii{#1}{\noexpand#3{#2}}})");
    L.insert(R"(\def\mod#1{\@nomathswiv{~\@mod\;#1}{#1}{\mod}})");
    L.insert(R"(\def\bmod{\@nomathswiii{\;\@mod\;}{\noexpand\bmod}})");
    L.insert(R"(\def\pmod#1{\@nomathswiv{\pod{\@mod\; #1}}{#1}\pmod})");
    L.insert(R"(\def\pod#1{\ifinner\mkern8mu\else\mkern18mu\fi(#1)})");
    L.insert(R"(\def\LaTeXe{\LaTeX2$\epsilon$})");
    L.insert(R"(\def\date#1{\xmlelt{date}{#1}})");
    L.insert(R"(\def\enspace{\kern.5em })");
    L.insert(R"(\def\@makeother#1{\catcode`#1=12\relax})");
    L.insert(R"(\def\@makeactive#1{\catcode`#1=13\relax})");
    L.insert("\\def\\root#1\\of{\\@root{#1}}\n");
    L.insert(R"(\long\def\g@addto@macro#1#2{\begingroup\toks@\expandafter{#1#2}\xdef#1{\the\toks@}\endgroup})");
    L.insert(R"(\long\def\addto@hook#1#2{#1\expandafter{\the#1#2}})");
    L.insert(R"(\def\x@tag#1{\qquad\mathrm{(#1)}})");
    L.insert(R"(\def\y@tag#1{\qquad\mathrm{#1}})");
    L.insert(R"(\def\@x@tag#1{\formulaattribute{tag}{(#1)}})");
    L.insert(R"(\def\@y@tag#1{\formulaattribute{tag}{#1}})");
    L.insert(R"(\def\eqref#1{(\ref{#1})})");
    L.insert(R"(\def\@carcube#1#2#3#4\@nil{#1#2#3})");
    L.insert(R"(\long\def\@thirdofthree#1#2#3{#3})");
    L.insert(R"(\def\on@line{ on input line \the\inputlineno})");
    L.insert(R"(\def\pagenumbering#1{\xmlelt{pagestyle}{\XMLaddatt{numbering}{#1}}})");
    L.insert(R"(\def\pagestyle#1{\xmlelt{pagestyle}{\XMLaddatt{style}{#1}}})");
    L.insert(R"(\def\thispagestyle#1{\xmlelt{pagestyle}{\XMLaddatt{this-style}{#1}}})");
    L.insert(R"(\def\thesubfigure{\thefigure.\@arabic\c@subfigure})");
    L.insert(R"(\def\@filedoterr#1{\error{Wrong dots in graphic file #1}})");
    L.insert(R"(\def\theEnumi{\arabic{Enumi}})");
    L.insert(R"(\def\theEnumii{\theEnumi.\arabic{Enumii}})");
    L.insert(R"(\def\theEnumiii{\theEnumii.\arabic{Enumiii}})");
    L.insert(R"(\def\theEnumiv{\theEnumiii.\arabic{Enumiv}})");
    L.insert(R"(\def\theEnumv{\theEnumiv.\arabic{Enumiv}})");
    L.insert(R"(\def\labelenumi{(\theenumi)})");
    L.insert(R"(\def\labelenumii{(\theenumii)})");
    L.insert(R"(\def\labelenumiii{(\theenumiii)})");
    L.insert(R"(\def\labelenumiv{(\theenumiv)})");
    L.insert(R"(\def\labelenumv{(\theenumv)})");
    L.insert(R"(\def\labelenumvi{(\theenumvi)})");
    L.insert(R"(\def\labelenumvii{(\theenumvii)})");
    L.insert(R"(\def\labelenumviii{(\theenumviii)})");
    L.insert(R"(\def\labelenumix{(\theenumix)})");
    L.insert(R"(\def\labelenumx{(\theenumx)})");
    L.insert(R"(\def\bordermatrix#1{{\let\cr\\)");
    L.insert("\\begin{bordermatrix }#1\\end{bordermatrix }}}");
    L.insert(R"(\def\incr@eqnum{\refstepcounter{equation}})");
    L.insert(R"(\def\@@theequation{\theparentequation\alph{equation}})");

    if (!everyjob_string.empty()) L.insert(everyjob_string, true); // is this converted ?
    L.insert("%% End bootstrap commands for latex");
    init(L);
    translate0();

    eqtb_int_table[uchar('@')].val = 12; // this is \makeatother
    TokenList ejob                 = toks_registers[everyjob_code].val;
    back_input(ejob);
}

void Parser::more_bootstrap() {
    initialise_font(); // Define current font
    TokenList L;
    Token     T, T1;
    T = hash_table.locate("space");
    L.push_back(T);
    L.push_back(T);
    L.push_back(T);
    L.push_back(T);
    new_prim("@spaces", L);
    {
        auto A    = uchar(' ');
        auto Bval = T.eqtb_loc();
        eq_define(A, Hashtab::the_eqtb()[Bval].val, true);
        A    = '#';
        Bval = hash_table.locate("#").eqtb_loc();
        eq_define(A, Hashtab::the_eqtb()[Bval].val, true);
        A    = '_';
        Bval = hash_table.locate("_").eqtb_loc();
        eq_define(A, Hashtab::the_eqtb()[Bval].val, true);
        A    = '\r'; // eqtbloc of active end-of-line (^^M)
        Bval = hash_table.par_token.eqtb_loc();
        eq_define(A, Hashtab::the_eqtb()[Bval].val, true);
    }
    L.clear();
    L.push_back(hash_table.par_token);
    new_prim("endsloppypar", L);
    L.push_back(hash_table.locate("sloppy"));
    new_prim("sloppypar", L);
    L.clear();
    L.brace_me();
    T = hash_table.locate("hbox");
    L.push_front(T);
    new_prim("null", L);
    L.clear();
    L.push_back(T);
    L.push_back(Token(letter_t_offset, 't'));
    L.push_back(Token(letter_t_offset, 'o'));
    new_prim("hb@xt@", L);
    L.clear();
    L.push_back(hash_table.locate("@arabic"));
    L.push_back(hash_table.locate("c@page"));
    new_prim("thepage", L);
    new_primx("labelitemi", "textbullet");
    // Original code was : \normalfont\bfseries \textendash
    new_primx("labelitemii", "textendash");
    new_primx("labelitemiii", "textasteriskcentered");
    new_primx("labelitemiv", "textperiodcentered");
    new_primx("guillemets", "guillemotleft");
    new_primx("endguillemets", "guillemotright");
    new_primx("@nnil", "@nil");
    L.clear();
    T  = hash_table.locate("@makeother");
    T1 = hash_table.locate("do");
    TokenList L1;
    Token     w;

    auto ADD_TO_BOTH = [&](String s) {
        w = hash_table.locate(s);
        L.push_back(T);
        L.push_back(w);
        L1.push_back(T1);
        L1.push_back(w);
    };

    ADD_TO_BOTH(" ");
    ADD_TO_BOTH("\\");
    ADD_TO_BOTH("$");
    ADD_TO_BOTH("&");
    ADD_TO_BOTH("#");
    ADD_TO_BOTH("^");
    ADD_TO_BOTH("_");
    ADD_TO_BOTH("%");
    ADD_TO_BOTH("~");
    L1.push_back(T1);
    L1.push_back(hash_table.locate("{"));
    L1.push_back(T1);
    L1.push_back(hash_table.locate("}"));
    new_prim("@sanitize", L);
    new_prim("dospecials", L1);
    T = hash_table.locate("@makeactive");
    L.clear();
    L.push_back(T);
    L.push_back(hash_table.locate(" "));
    new_prim("obeyspaces", L);
    L.clear();
    L.push_back(T);
    L.push_back(hash_table.locate("\r")); // define ^^M
    L.push_back(hash_table.let_token);
    L.push_back(Token(eqtb_offset + '\r'));
    L.push_back(hash_table.par_token);
    new_prim("obeylines", L);
    L.clear();
    L.push_back(T);
    L.push_back(hash_table.locate(" ")); // define ^^M
    L.push_back(hash_table.let_token);
    L.push_back(Token(eqtb_offset + ' '));
    L.push_back(hash_table.locate("nobreakspace"));
    new_prim("@vobeyspaces", L);
}

// Fills the catcode table. Catcodes are at the start of eqtb_int_table
void Parser::make_catcodes() {
    for (unsigned i = 0; i < nb_characters; i++) {
        set_catcode(i, other_catcode);
        eqtb_int_table[i].level = 1;
    }
    for (unsigned i = 'a'; i <= 'z'; i++) set_catcode(i, letter_catcode);
    for (unsigned i = 'A'; i <= 'Z'; i++) set_catcode(i, letter_catcode);
    set_catcode('\\', 0);
    set_catcode('{', 1);
    set_catcode('}', 2);
    set_catcode('$', 3);
    set_catcode('&', 4);
    set_catcode('\r', 5);
    set_catcode('\n', 12);
    set_catcode('#', 6);
    set_catcode('^', 7);
    set_catcode('_', 8);
    set_catcode('\t', 10);
    set_catcode(' ', 10);
    set_catcode(160, 13); // non breaking space
    set_catcode('~', 13);
    set_catcode('%', 14);
    for (unsigned i = 0; i < nb_shortverb_values; i++) old_catcode[i] = eqtb_int_table[i].val;
    set_catcode('@', 11);
}

// This is the main bootstrap code
void Parser::boot() {
    boot_time();
    boot_accents();
    boot_verbatim();
    boot_xspace();
    boot_uclc();
    make_catcodes();
    make_names();
    new_array_object.boot(this);
    make_constants();
}

// This implements \a', or \a{'}, expansion is \', empty in case of an error
// cur_tok is \a, in case of error
void Parser::E_accent_a() {
    if (tracing_macros()) {
        Logger::finish_seq();
        the_log << "{" << cur_tok << "}\n";
    }
    TokenList y = read_arg();
    if (!token_ns::has_a_single_token(y)) {
        parse_error("wanted a single token as argument to \\a");
        return;
    }
    Token t = token_ns::get_unique(y);
    if (!t.is_a_char()) { // latex says \csname\string #1 \endcsname
        parse_error(err_tok, "Bad syntax of \\a, argument not a character ", t, "", "bad accent");
        return;
    }
    auto Y = Token(t.chr_val() + single_offset);
    token_from_list(Y);
    if (cur_cmd_chr.cmd != accent_cmd) {
        parse_error(err_tok, "Bad syntax of \\a, argument is ", t, "", "bad accent");
        return;
    }
    back_input();
}

// This implements \'e, or \'{a} or \a{'}{e}, \href{\~grimm}
// or \'{\= u}, a double accent
// cur_tok, cur_cmd_chr explain what to do.
void Parser::E_accent() {
    if (tracing_macros()) {
        Logger::finish_seq();
        the_log << "{accent " << cur_tok << "}\n";
    }
    unsigned acc_code = cur_cmd_chr.chr;
    if (global_in_url && acc_code == '~') {
        if (tracing_macros()) {
            Logger::finish_seq();
            the_log << "{\\~ gives ~ }\n";
        }
        back_input(Token(other_t_offset, '~'));
        return;
    }
    String msg1 = "Error in accent, command = ";
    String msg2 = "bad accent";
    Token  tfe  = cur_tok;
    // Fetch the accent
    TokenList y = read_arg();
    if (y.empty()) {
        err_buf = msg1 + tfe.tok_to_str();
        err_buf += (acc_code == '~' ? "\n\\~{} is the wrong way to put a tilde in an URL" : "\nThings like {\\'{}} are a bit strange");
        signal_error(err_tok, msg2);
        return;
    }
    unsigned acc_code2 = 0; // will be set in the double acc case.
    Token    tfe2;          // Second token for error.
    if (y.size() >= 2) {
        token_from_list(y.front()); // Get the meaning of the start of y
        // case \'{\a'e};
        if (cur_cmd_chr.cmd == a_cmd) {
            y.pop_front(); // discard the \a
            accent_ns::special_acc_hack(y);
            if (y.empty()) {
                parse_error("Internal error, lost argument of \\a");
                return;
            }
            Token t = y.front(); // get the '
            if (!t.is_a_char()) {
                parse_error(err_tok, "Bad syntax of \\a, argument not a character ", t, "", msg2);
                return;
            }
            y.pop_front(); // discard the accent
            tfe2 = Token(t.chr_val() + single_offset);
            token_from_list(tfe2);
            if (cur_cmd_chr.cmd != accent_cmd) {
                parse_error(err_tok, "Bad syntax of \\a,  argument is ", t, "", msg2);
                return;
            }
        } else if (cur_cmd_chr.cmd == accent_cmd) {
            tfe2 = cur_tok;
            y.pop_front();
        } else {
            parse_error(err_tok, msg1, tfe, "\nWanted a single token", msg2);
            return;
        }
        acc_code2 = cur_cmd_chr.chr;
        accent_ns::special_acc_hack(y);
        if (y.size() != 1) {
            parse_error(err_tok, msg1, tfe, "\nWanted a single token", msg2);
            return;
        }
    }
    Token Y = token_ns::get_unique(y);
    token_from_list(Y);
    unsigned int achar = cur_cmd_chr.chr;
    if (achar <= 8)
        achar = 8; // make these invalid
    else if (achar == 0xc6)
        achar = 1;
    else if (achar == 0xe6)
        achar = 2;
    else if (achar == 0xc5)
        achar = 3;
    else if (achar == 0xe5)
        achar = 4;
    else if (achar == 0xd8)
        achar = 5;
    else if (achar == 0xf8)
        achar = 6;
    if (cur_cmd_chr.cmd == cst1_cmd && achar == i_code)
        achar = 'i';
    else if ((cur_cmd_chr.cmd == specchar_cmd) || cur_cmd_chr.is_letter_other()) {
    } else {
        err_buf = msg1 + tfe.tok_to_str();
        if (acc_code2 != 0) err_buf += tfe2.tok_to_str();
        if (Y.is_invalid())
            err_buf += "\nEnd of data reached while scanning argument";
        else if (!cur_tok.not_a_cmd())
            err_buf += "\nLetter needed instead of " + cur_tok.tok_to_str();
        else
            err_buf += "\nLetter needed";
        signal_error(err_tok, msg2);
        return;
    }
    Token res;
    bool  spec = false;
    if (achar < nb_accents) {
        if (acc_code2 != 0) {
            int acc3_code = combine_accents(acc_code, acc_code2);
            res           = accent_ns::fetch_double_accent(to_signed(achar), acc3_code);
        } else {
            res = fetch_accent(achar, acc_code);
            if (res.is_null() && achar < 128) {
                if (std::isalpha(static_cast<char>(achar)) != 0) {
                    spec = true; // T. Bouche veut une erreur
                    res  = fetch_accent(0, acc_code);
                } else
                    res = special_double_acc(achar, acc_code);
            }
        }
    }
    if (res.is_null()) {
        std::string s = achar >= 128                                  ? "a non 7-bit character"
                        : std::isalpha(static_cast<char>(achar)) != 0 ? "letter"
                        : std::isdigit(static_cast<char>(achar)) != 0 ? "digit"
                                                                      : "non-letter character";
        err_buf       = msg1 + tfe.tok_to_str();
        if (acc_code2 != 0) err_buf += tfe2.tok_to_str();
        err_buf += "\nCannot put accent on " + s;
        if (achar < 128) {
            err_buf += " ";
            err_buf += char(achar);
        }
        signal_error(err_tok, msg2);
        return;
    }
    TokenList expansion;
    expansion.push_back(res);
    if (spec) {
        expansion.push_front(Y);
        expansion.brace_me();
        expansion.push_front(hash_table.composite_token);
    }

    if (tracing_macros()) {
        Logger::finish_seq();
        the_log << "{accent on " << uchar(achar) << " -> " << expansion << "}\n";
    }
    back_input(expansion);
}

// \cite@one{foot}{Knuth}{p25}.
// Translates to <cit><ref target="xx">p25</ref></cit>
// and \cite@simple{Knuth} gives <ref target="xx"/>
void Parser::T_cite_one() {
    bool  is_simple = cur_cmd_chr.chr != 0;
    Token T         = cur_tok;
    flush_buffer();
    std::string type = is_simple ? "" : fetch_name0_nopar();
    cur_tok          = T;
    auto ref         = std::string(fetch_name0_nopar());
    Xml *arg         = is_simple ? nullptr : xT_arg_nopar();
    // signal error after argument parsing
    if (bbl.too_late) {
        parse_error("Citation after loading biblio?");
        return;
    }
    auto *res = new Xml(make_cit_ref(type, ref));
    if (is_simple) {
        flush_buffer();
        the_stack.add_last(res);
        return;
    }
    leave_v_mode();
    TokenList L     = get_mac_value(hash_table.cite_type_token);
    auto      xtype = std::string(fetch_name1(L));
    L               = get_mac_value(hash_table.cite_prenote_token);
    auto prenote    = std::string(fetch_name1(L));
    if (arg != nullptr) res->add_tmp(gsl::not_null{arg});
    the_stack.add_last(new Xml(the_names["cit"], res));
    if (!type.empty()) the_stack.add_att_to_last(the_names["rend"], type);
    if (!xtype.empty()) the_stack.add_att_to_last(the_names["citetype"], xtype);
    if (!prenote.empty()) the_stack.add_att_to_last(the_names["prenote"], prenote);
}

// This adds a marker, a location where to insert the bibliography
// The marker can be forcing or not. If it is not forcing, and a forcing
// marker has already been inserted, nothing happens.
void Parser::add_bib_marker(bool force) {
    Bibliography &T = the_bibliography;
    if (!force && T.location_exists()) return;
    Xml *mark = new Xml(std::string(), nullptr);
    Xml *Foo  = new Xml(std::string(), mark);
    the_stack.add_last(Foo);
    T.set_location(mark, force);
}

// Translation of \bibliographystyle{foo}
// We remember the style; we could also have a command, bibtex:something
// or program:something
void Parser::T_bibliostyle() {
    auto          Val = fetch_name0_nopar();
    Bibliography &T   = the_bibliography;
    if (Val.starts_with("bibtex:")) {
        if (Val[7] != 0) T.set_style(Val.substr(7));
        T.set_cmd("bibtex " + get_job_name());
    } else if (Val.starts_with("program:"))
        T.set_cmd(Val.substr(8) + " " + get_job_name() + ".aux");
    else
        T.set_style(Val);
}

// Translation of \bibliography{filename}. We add a marker.
// We have a list of file names, they are stored in the
// bibliography data structure
void Parser::T_biblio() {
    flush_buffer();
    auto list = fetch_name0_nopar();
    add_bib_marker(false);
    for (const auto &w : split_commas(list)) {
        if (w.empty()) continue;
        the_bibliography.push_back_src(w);
    }
}

void Parser::after_main_text() {
    the_bibliography.stats();
    if (the_bibliography.has_cmd())
        create_aux_file_and_run_pgm();
    else {
        the_bibliography.dump_bibtex();
        the_bibtex.work();
    }
    init(bbl.lines);
    if (!lines.empty()) {
        Xml *res = the_stack.temporary();
        the_stack.push1(the_names["biblio"]);
        AttList &L = the_stack.get_att_list(3);
        the_stack.cur_xid().add_attribute(L, true);
        translate0();
        the_stack.pop(the_names["biblio"]);
        the_stack.pop(the_names["argument"]);
        the_stack.document_element()->insert_bib(res, the_bibliography.location);
    }
    finish_color();
    finish_index();
    check_all_ids();
}

// Translation of \end{thebibliography}
void Parser::T_end_the_biblio() {
    leave_h_mode();
    the_stack.pop(the_names["thebibliography"]);
}

// Translation of \begin{thebibliography}
// The bibliography contains \bibitem commands.
// Thus we can safely remove two optional arguments
// before and after the mandatory one.
void Parser::T_start_the_biblio() {
    ignore_optarg();
    ignore_arg(); // longest label, ignored
    ignore_optarg();
    TokenList L1;
    L1.push_back(hash_table.refname_token);
    auto a = fetch_name1(L1);
    the_stack.set_arg_mode();
    auto name = std::string(a);
    the_stack.set_v_mode();
    the_stack.push(the_names["thebibliography"], new Xml(name, nullptr));
}

// \cititem{foo}{bar} translates \cititem-foo{bar}
// If the command with the funny name is undefined then translation is
// <foo>bar</foo>
// Command has to be in Bib mode, argument translated in Arg mode.
void Parser::T_cititem() {
    auto a = fetch_name0_nopar();
    finish_csname("cititem-" + a);
    see_cs_token();
    if (cur_cmd_chr.cmd != relax_cmd) {
        back_input();
        return;
    }
    mode m = the_stack.get_mode();
    need_bib_mode();
    the_stack.set_arg_mode();
    auto name = std::string(a);
    the_stack.push(name, new Xml(name, nullptr));
    T_arg();
    the_stack.pop(name);
    the_stack.set_mode(m);
    the_stack.add_nl();
}

// case of \omitcite{foo}. reads an argument, puts it in the omit mist
// and logs it
void Parser::T_omitcite() {
    flush_buffer();
    std::string s = sT_arg_nopar();
    omitcite_list.push_back(s);
    Logger::finish_seq();
    the_log << "{\\omitcite(" << int(omitcite_list.size());
    the_log << ") = " << s << "}\n";
}

// We start with a function that fetches optional arguments
// In prenote we put `p. 25' or nothing, in type one of year, foot, refer, bar
// as an istring, it will be normalised later.
void Parser::T_cite(subtypes sw, TokenList &prenote, std::string &type) {
    if (sw == nocite_code) {
        type = std::string(fetch_name_opt());
    } else if (sw == natcite_code) {
        if (auto L = read_optarg_nopar()) prenote.append(*L); // is really the post note
    } else {
        // we can have two optional arguments, prenote is last
        if (auto L1 = read_optarg_nopar()) {
            if (auto L2 = read_optarg_nopar()) {
                prenote.append(*L2);
                type = std::string(fetch_name1(*L1));
            } else
                prenote = *L1;
        }
    }
}

// You can say \cite{foo,bar}. This is the same as
// \cite@one{}{foo}{}\cite@one{}{foo}{}, empty lists are replaced by the
// optional arguments.  The `prenote' applies only to the first entry,
// and the `type' to all entries. If you do not like like, do not use commas.
// The natbib package proposes a postnote, that applies only to the last.
// We construct here a token list, and push it back.

// The command is fully handled in the \nocite case. The `prenote' is ignored.
// For the special entry `*', the type is ignored too.
void Parser::T_cite(subtypes sw) {
    Token T         = cur_tok;
    bool  is_natbib = sw == natcite_code;
    if (sw == natcite_e_code) {
        flush_buffer();
        the_stack.pop(the_names["natcit"]);
        return;
    }
    if (is_natbib) {
        leave_v_mode();
        the_stack.push1(the_names["natcit"]);
    }
    TokenList res;
    TokenList prenote;
    if (is_natbib) {
        if (auto x = nT_optarg_nopar(); x && !x->empty()) the_stack.add_att_to_last(the_names["citetype"], *x);
        if (auto o = read_optarg()) res.append(*o);
        if (!res.empty()) res.push_back(hash_table.space_token);
        res.push_front(hash_table.locate("NAT@open"));
    }
    cur_tok = T;
    std::string type;
    T_cite(sw, prenote, type); // reads optional arguments
    Token sep    = sw == natcite_code ? hash_table.locate("NAT@sep") : hash_table.cite_punct_token;
    cur_tok      = T;
    auto  List   = fetch_name0_nopar();
    int   n      = 0;
    Token my_cmd = is_natbib ? hash_table.citesimple_token : hash_table.citeone_token;
    for (const auto &cur : split_commas(List)) {
        if (cur.empty()) continue;
        if (sw == nocite_code) {
            if (cur == "*")
                the_bibliography.set_nocite();
            else
                the_bibliography.find_citation_item(type, std::string(cur), true);
        } else {
            if (n > 0) res.push_back(sep);
            TokenList tmp;
            res.push_back(my_cmd);
            if (!is_natbib) {
                tmp = token_ns::string_to_list(type, true);
                res.splice(res.end(), tmp);
            }
            tmp = token_ns::string_to_list(cur, true);
            res.splice(res.end(), tmp);
            if (!is_natbib) {
                res.push_back(hash_table.OB_token);
                if (n == 0) res.splice(res.end(), prenote);
                res.push_back(hash_table.CB_token);
            }
            n++;
        }
    }
    if (is_natbib) {
        if (!prenote.empty()) res.push_back(hash_table.locate("NAT@cmt"));
        res.splice(res.end(), prenote);
        res.push_back(hash_table.locate("NAT@close"));
        res.push_back(hash_table.end_natcite_token);
    }
    if (tracing_commands()) {
        Logger::finish_seq();
        the_log << T << "->" << res << ".\n";
    }
    back_input(res);
}

// Flag true for bibitem, \bibitem[opt]{key}
// false in the case of \XMLsolvecite[id][from]{key}
void Parser::solve_cite(bool user) {
    Token       T = cur_tok;
    bool        F = true;
    size_t      n = 0;
    std::string from;
    if (user) {
        implicit_par(zero_code);
        the_stack.add_last(new Xml(the_names["bibitem"], nullptr));
        if (auto ukey = nT_optarg_nopar()) Xid(the_stack.get_xid()).get_att()[the_names["bibkey"]] = *ukey;
        n = the_stack.get_xid();
    } else {
        F    = remove_initial_star();
        n    = read_elt_id(T);
        from = std::string(fetch_name_opt());
    }
    cur_tok  = T;
    auto key = std::string(fetch_name0_nopar());
    if (user) insert_every_bib();
    if (n == 0) return;
    Xid           N  = Xid(n);
    Bibliography &B  = the_bibliography;
    size_t        nn = 0;
    if (F)
        nn = B.find_citation_star(from, key);
    else
        nn = *B.find_citation_item(from, key, true);
    CitationItem &CI = B.citation_table[nn];
    if (CI.is_solved()) {
        err_buf = "Bibliography entry already defined " + encode(key);
        the_parser.signal_error(the_parser.err_tok, "bad solve");
        return;
    }
    AttList &AL    = N.get_att();
    auto *   my_id = AL.lookup(the_names["id"]);
    if (my_id != nullptr) {
        if (CI.id.empty())
            CI.id = *my_id;
        else {
            err_buf = "Cannot solve (element has an Id) " + encode(key);
            the_parser.signal_error(the_parser.err_tok, "bad solve");
            return;
        }
    } else
        AL[the_names["id"]] = CI.get_id();
    CI.solved = N;
}

// \bpers[opt-full]{first-name}{von-part}{last-name}{jr-name}
// note that Tralics generates an empty von-part
void Parser::T_bpers() {
    int e              = nb_errs;
    unexpected_seen_hi = false;
    auto        A      = nT_optarg_nopar();
    std::string a      = nT_arg_nopar();
    std::string b      = nT_arg_nopar();
    std::string c      = nT_arg_nopar();
    std::string d      = nT_arg_nopar();
    if (unexpected_seen_hi && e != nb_errs) log_and_tty << "maybe you confused Publisher with Editor\n";
    need_bib_mode();
    the_stack.add_newid0("bpers");
    if (A && !A->empty()) the_stack.add_att_to_last(the_names["full_first"], *A);
    if (!d.empty()) the_stack.add_att_to_last(the_names["junior"], d);
    the_stack.add_att_to_last(the_names["nom"], c);
    if (!b.empty()) the_stack.add_att_to_last(the_names["particule"], b);
    the_stack.add_att_to_last(the_names["prenom"], a);
}

// case \bibitem
void Parser::T_bibitem() {
    flush_buffer();
    leave_h_mode();
    leave_v_mode();
    solve_cite(true);
    skip_initial_space_and_back_input();
}

// this is the same as \bibitem[]{foo}{}
void Parser::T_empty_bibitem() {
    flush_buffer();
    std::string w;
    std::string a;
    std::string b  = sT_arg_nopar();
    std::string id = the_bibtex.exec_bibitem(b);
    if (id.empty()) return;
    leave_v_mode();
    the_stack.push1(the_names["citation"]);
    the_stack.implement_cit(b, id, a, w);
    the_stack.pop(the_names["citation"]);
}

// Translation of \citation{key}{userid}{id}{from}{type}[alpha-key]
void Parser::T_citation() {
    flush_buffer();
    std::string a  = sT_arg_nopar();
    std::string b1 = special_next_arg();
    std::string b2 = sT_arg_nopar();
    std::string c  = sT_arg_nopar();
    std::string d  = nT_arg_nopar();
    the_stack.add_nl();
    the_stack.push1(the_names["citation"]);
    the_stack.add_att_to_last(the_names["type"], d);
    the_stack.implement_cit(b1, std::string(b2), a, c);
    the_stack.set_bib_mode();
    ignore_optarg();
    insert_every_bib();
}

void Parser::insert_every_bib() {
    TokenList everybib = toks_registers[everybibitem_code].val;
    if (everybib.empty()) return;
    if (tracing_commands()) {
        Logger::finish_seq();
        the_log << "{<everybibitem> " << everybib << "}\n";
    }
    back_input(everybib);
}

auto Parser::last_att_list() -> AttList & { return the_stack.get_top_id().get_att(); }

// Implements \newcolumntype{C}[opt]{body}
// as \newcommand\cmd[opt]{body}
// Here \cmd is some internal name, stored in nct_tok[C]
void Parser::T_newcolumn_type() {
    unsigned c   = 0;
    bool     bad = read_token_arg(cur_tok);
    if (!bad) {
        Token T = cur_tok;
        if (T.is_a_char()) c = T.char_val();
        if (!(c > 0 && c < nb_newcolumn)) {
            parse_error("Argument to \\newcolumntype is not a 7bit character");
            c = 0;
        }
    }
    Buffer B;
    B.append("newcolumtype@");
    B.push_back(static_cast<char>(c)); // special hack if c=0 !
    cur_tok = hash_table.locate(B);
    new_array_object.add_a_type(static_cast<uchar>(c), cur_tok);
    back_input();
    M_newcommand(rd_always); // definition is local
}

// Assume that T is the token defined and stored by the procedures above
// This returns -1 in case something strange happened (like redefinition)
// returns the number of arguments of the function otherwise.
auto Parser::nct_aux(Token T, TokenList &body) -> std::optional<size_t> {
    see_cs_token(T);
    if (!cur_cmd_chr.is_user()) return {}; // bad
    const Macro &X = mac_table.get_macro(cur_cmd_chr.chr);
    if (!(X.type == dt_empty || X.type == dt_normal)) return {};
    body = X.body;
    return X.nbargs;
}

// This piece of code expands all the \newcolumntype commands
// There is a special case: nct_size=0,  case where nothing is
// defined, and nothing has to be done. Otherwise, we loop over all
// characters. If there is one substitution, we try again.
// If for some reason, the character is invalid, we remove it from the table.
void Parser::expand_nct(TokenList &L) {
    bool action = true;
    if (tracing_commands()) {
        Logger::finish_seq();
        the_log << "array preamble at start: " << L << "\n";
    }
    if (!new_array_object.has_a_nct()) return;
    int max_iter = max_newcolumn_loops;
    while (action) {
        action = false;
        max_iter--;
        if (max_iter <= 0) {
            parse_error("array preamble expansion: Infinite loop?");
            return;
        }
        for (uchar i = 1; i < nb_newcolumn; i++) {
            const uchar c = i;
            if (!new_array_object.nct_exists(c)) continue;
            Token     T = new_array_object.nct_token(c);
            TokenList body;
            auto      n = nct_aux(T, body);
            if (!n) {
                new_array_object.remove_a_type(c);
                continue;
            }
            if (L.expand_nct(*n, c, max_iter, body)) {
                action = true;
                if (tracing_commands()) the_log << "array preamble after " << c << ": " << L << "\n";
            }
        }
    }
}

// Start a row. If a=-1, it is the first row
void Parser::start_a_row(long a, const std::string &s) {
    Xid prev_row(2); // dummy id as default value
    {
        Xml *V = the_stack.top_stack()->last_addr();
        if (V != nullptr) prev_row = V->id;
    }
    bool initial_hline = false;
    if (!s.empty()) prev_row.add_attribute(the_names["row_spaceafter"], s);
    for (;;) {
        remove_initial_space_and_back_input(); // get first non-space xtoken
        symcodes S = cur_cmd_chr.cmd;
        if (S == ifnextchar_cmd) {
            get_token();
            T_ifnextchar(true);
            continue;
        }
        if (S == hline_cmd) {
            auto     guard = SaveErrTok(cur_tok);
            subtypes c     = cur_cmd_chr.chr;
            get_token();
            int w = T_hline_parse(c);
            if (w == 2) T_cline();
            if (w == 1) { // a hline
                if (a < 0)
                    initial_hline = true;
                else
                    prev_row.add_bottom_rule();
            }
            continue;
        }
        if (S == end_cmd) return;
        the_stack.push1(the_names["row"]);
        if (initial_hline) the_stack.cur_xid().add_top_rule();
        start_a_cell(false);
        return;
    }
}

// If started is true, we have already started one
void Parser::start_a_cell(bool started) {
    if (!started) {
        the_stack.push_pop_cell(push_only);
        push_level(bt_cell);
    }
    remove_initial_space_and_back_input();
    symcodes S = cur_cmd_chr.cmd;
    if (S == unimp_cmd && cur_cmd_chr.chr == omit_code) {
        get_token();
        the_stack.mark_omit_cell();
    } else if (S == end_cmd) { // has to be \end{tabular}
        the_stack.mark_omit_cell();
    } else if (S == multicolumn_cmd) {
        the_stack.mark_omit_cell();
        get_token();
        unprocessed_xml.remove_last_space();
        Xid cid = the_stack.get_top_id();
        new_array_object.run(cid, false);
    } else {
        TokenList L = the_stack.get_u_or_v(true);
        if (tracing_commands() && !L.empty()) {
            Logger::finish_seq();
            the_log << "{template u-part " << L << "}\n";
        }
        back_input(L);
    }
}

void Parser::finish_a_cell(Token T, const std::string &a) {
    flush_buffer();
    the_stack.remove_last_space();
    if (!a.empty()) {
        saved_dim = a;
        T         = hash_table.crwithargs_token;
    }
    back_input(T);
    if (the_stack.is_omit_cell()) return;
    TokenList L = the_stack.get_u_or_v(false);
    if (tracing_commands() && !L.empty()) {
        Logger::finish_seq();
        the_log << "{template v-part " << L << "}\n";
    }
    back_input(L);
}

// This is \begin{tabular}. Should be in an environment (not necessarily)
void Parser::T_start_tabular(subtypes c) {
    std::string x = the_names[c == zero_code ? "tabular" : "tabular*"];
    if (the_stack.is_float())
        leave_v_mode();
    else if (the_stack.is_frame("fbox")) {
    } else
        leave_h_mode();
    M_let_fast(hash_table.par_token, hash_table.empty_token, false);
    the_stack.push1(x, the_names["table"]);
    the_stack.add_att_to_last(the_names["rend"], the_names["inline"]);
    Xid id = the_stack.get_xid();
    if (c != 0) { // case of tabular*
        Token     T         = cur_tok;
        TokenList L         = read_arg();
        ScaledInt tab_width = dimen_from_list(T, L);
        if (!(tab_width == ScaledInt{0})) id.add_attribute(the_names["table_width"], std::string(tab_width));
        get_token(); // eat the relax
        if (!cur_cmd_chr.is_relax()) back_input();
    }
    {
        auto pos = get_ctb_opt(); // Lamport does not mention c, who cares
        if (pos) id.add_attribute(the_names["vpos"], the_names[*pos]);
    }
    new_array_object.run(id, true);
    the_stack.set_array_mode(); // Is the mode ok ?
    start_a_row(-1);
}

// implements \hline, \cline{p} or \hlinee[p]{w}{w}{w}
// p is a sequence of two integers, separated by a dash
// sets cline_first and cline_last;
// fills errbuf and returns a code in case of error

auto Parser::scan_pair_ints(Token T, TokenList &L) -> bool {
    back_input(hash_table.relax_token);
    back_input(L);
    cline_first = scan_int(T);
    if (get_token()) {
        errbuf = fmt::format("Error in {} after first integer", T);
        return true;
    }
    if (cur_tok == hash_table.relax_token) {
        errbuf = fmt::format("Error in {} after first integer", T);
        return true;
    }
    cline_last = scan_int(T);
    read_until(hash_table.relax_token);
    if (1 <= cline_first && cline_first <= cline_last) return false;
    errbuf = fmt::format("Bad range in {}: {}-{}", T, cline_first, cline_last);
    return true;
}

// Now a function that parses the arguments, and returns a status
// 0:none, 1: hline, 2: cline,
// may set in_hlinee and hlinee_width

auto Parser::T_hline_parse(subtypes c) -> int {
    in_hlinee = false;
    Token T   = cur_tok;
    if (c == one_code) { // cline
        TokenList arg = read_arg();
        if (scan_pair_ints(T, arg)) {
            parse_error(errbuf);
            return 0;
        }
        return 2;
    }
    if (c != two_code) return 1;
    TokenList cols = read_arg();
    int       rt   = 1;
    if (!cols.empty()) {
        rt = 2;
        if (scan_pair_ints(T, cols)) {
            parse_error(errbuf);
            ignore_arg();
            ignore_arg();
            ignore_arg();
            return 0; // read the mandatory arguments before returning
        }
    }
    have_above  = false;
    have_below  = false;
    TokenList L = read_arg();
    if (!L.empty()) {
        ScaledInt tab_width = dimen_from_list(T, L);
        if (!(tab_width == ScaledInt{0})) {
            have_above   = true;
            hlinee_above = std::string(tab_width);
        }
    }
    L = read_arg();
    if (!L.empty()) {
        ScaledInt tab_width = dimen_from_list(T, L);
        if (!(tab_width == ScaledInt{0})) {
            have_below   = true;
            hlinee_below = std::string(tab_width);
        }
    }
    L                   = read_arg();
    ScaledInt tab_width = dimen_from_list(T, L);
    in_hlinee           = true;
    hlinee_width        = std::string(tab_width);
    return rt;
}

// Case when the command is found outside the tabular loop.
// Is this legal? we parse arguments the check again
void Parser::T_hline(subtypes c) {
    Token T   = cur_tok;
    int   res = T_hline_parse(c);
    if (the_stack.in_array_mode() && the_stack.is_frame("array")) {
        if (res == 1)
            the_stack.T_hline();
        else if (res == 2)
            T_cline();
    } else
        parse_error(err_tok, "misplaced ", T, " should occur in tables only", "Current command should occur in tables only");
}

// This is executed when we see \end{tabular}
// If true, the \end{tabular} is not executed.
// and we have to push back the `\end{tabular}' tokens
auto Parser::false_end_tabular(const std::string &s) -> bool {
    if (the_stack.is_frame("cell")) {
        TokenList L = token_ns::string_to_list(s, true);
        back_input(L);
        back_input(hash_table.locate("end"));
        finish_a_cell(hash_table.cr_token, std::string());
        return true;
    }
    return false;
}

// This is the code of \end{tabular}
void Parser::T_end_tabular(subtypes c) {
    the_stack.pop(the_names[c == 0 ? "tabular" : "tabular*"]);
    the_stack.delete_table_atts();
    if (!the_stack.is_float()) leave_v_mode();
}

//
void Parser::T_endv() {
    if (the_stack.is_frame("cell")) {
        flush_buffer();
        the_stack.finish_cell(-1);
        the_stack.push_pop_cell(pop_only);
        pop_level(bt_cell);
        start_a_cell(false);
    } else
        parse_error("Bad endv token");
}

// This is done when we see a \\.
void Parser::T_cr() {
    flush_buffer();
    std::string s;
    if (cur_cmd_chr.chr == crwithargs_code) s = saved_dim;
    if (!the_stack.is_frame("cell")) {
        parse_error("bad \\cr");
        return;
    }
    the_stack.finish_cell(0);
    the_stack.push_pop_cell(pop_only);
    pop_level(bt_cell);
    the_stack.pop(the_names["row"]);
    start_a_row(0, s);
}

// Case of \multispan; fully expandable
void Parser::E_multispan() {
    Token omit = hash_table.locate("omit");
    Token span = hash_table.locate("span");
    auto  n    = scan_int(cur_tok);
    back_input(omit);
    while (n > 1) {
        back_input(span);
        back_input(omit);
        --n;
    }
}

[[nodiscard]] auto Parser::translate03() -> bool {
    auto guard  = SaveErrTok(cur_tok);
    auto [x, c] = cur_cmd_chr;

    if (x == underscore_catcode && global_in_load) return translate_char(cur_cmd_chr), true;
    if (auto res = Symcode::get(x).call(c)) return *res;

    undefined_mac();
    return true;
}

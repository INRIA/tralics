// Tralics, a LaTeX to XML translator.
// Copyright INRIA/apics/marelle (Jose' Grimm) 2006-2011

// This software is governed by the CeCILL license under French law and
// abiding by the rules of distribution of free software.  You can  use,
// modify and/ or redistribute the software under the terms of the CeCILL
// license as circulated by CEA, CNRS and INRIA at the following URL
// "http://www.cecill.info".
// (See the file COPYING in the main directory for details)

// Tralics, math part II

#include "tralics/Logger.h"
#include "tralics/Math.h"
#include "tralics/MathF.h"
#include "tralics/MathP.h"
#include "tralics/Parser.h"
#include "tralics/globals.h"
#include "tralics/util.h"
#include <algorithm>
#include <fmt/format.h>

enum { pbm_empty, pbm_start, pbm_end, pbm_att, pbm_att_empty };
static Buffer                 mathml_buffer;
static Buffer                 att_buffer;
extern std::array<Xml *, 128> single_chars;
extern bool                   cmi_verbose;

namespace {
    // This is the list of valid exponents and the conversions.
    auto Buffer_special_exponent(const std::string &B) -> std::optional<std::string> {
        if (B == "th") return "th";
        if (B == "st") return "st";
        if (B == "rd") return "rd";
        if (B == "nd") return "nd";
        if (B == "e" || B == "ieme" || B == "eme" || B == "i\303\250me" || B == "\303\250me") return "e";
        if (B == "ier" || B == "er") return "er";
        if (B == "iemes" || B == "i\303\250mes" || B == "es") return "es";
        if (B == "\303\250re" || B == "re") return "re";
        return {};
    }

    // This handles the case of i{\`e}me. Removes initial font change,
    // looks at letters. If OK, return a valid XML exponent.
    auto special_exponent(const_math_iterator L, const_math_iterator E) -> Xml * {
        if (L == E) return nullptr;
        if (L->cmd == mathfont_cmd || L->cmd == fontsize_cmd) ++L;
        if (L == E) return nullptr;
        if (L->cmd == mathfont_cmd || L->cmd == fontsize_cmd) ++L;
        Buffer B;
        while (L != E) {
            if (L->is_char())
                B.push_back(L->get_char());
            else if (L->is_e_grave())
                B.push_back(char32_t(0350U));
            else
                return nullptr;
            ++L;
        }
        auto expo = Buffer_special_exponent(B);
        return expo ? new Xml(*expo) : nullptr;
    }

    // Returns the number of arguments of the command.
    auto nb_args_for_cmd(subtypes c) -> int {
        if (c >= first_maccent_code && c <= last_maccent_code) return 1;
        switch (c) {
        case mathchoice_code: return 4;
        case operatorname_code: return 1;
        case operatornamestar_code: return 1;
        case overline_code: return 1;
        case underline_code: return 1;
        case stackrel_code: return 2;
        case underset_code: return 2;
        case overset_code: return 2;
        case accentset_code: return 2;
        case underaccent_code: return 2;
        case frac_code: return 2;
        case overbrace_code: return 1;
        case underbrace_code: return 1;
        case undertilde_code: return 1;
        case mathmi_code: return 1;
        case mathmn_code: return 1;
        case mathmo_code: return 1;
        case mathci_code: return 1;
        case mathcn_code: return 1;
        case mathcsymbol_code: return 1;
        case math_attribute_code: return 2;
        case thismath_attribute_code: return 2;
        case formula_attribute_code: return 2;
        case mathlabel_code: return 2;
        case dfrac_code: return 2;
        case tfrac_code: return 2;
        case cfrac_code: return 2;
        case binom_code: return 2;
        case dbinom_code: return 2;
        case tbinom_code: return 2;
        case sqrt_code: return 1;
        case root_code: return 1;
        case genfrac_code: return 2;
        case multicolumn_code: return 3;
        case qopname_code: return 3;
        case mathbox_code: return 1;
        case multiscripts_code: return 1;
        case phantom_code: return 1;
        case hphantom_code: return 1;
        case vphantom_code: return 1;
        case smash_code: return 1;
        case boxed_code: return 1;
        default: return -1;
        }
    }
} // namespace

// This prints the whole MathP list on the stream

auto operator<<(std::ostream &fp, const MathP &X) -> std::ostream & {
    for (const auto &x : X) fp << x;
    return fp;
}

// Consider the expression \int_0^\infty f(x+y) dx = |z|b
// This will be translated to a list of 13 elements, the first one being
// \int_0^\infty. This element is marked big, and software like Amaya
// show the bars and parentheses with the same height as the integral.
// We want to add some <mrow>s in order to avoid this.

// The next function will return a list of the form
// 0b 2l 4B 6r 9R 10m 12m 13b.
// Here b means big (the integral, and there is a special marker at the end),
// l r m stand for small_l, small_r and small_m (the opening parenthesis,
// the closing parenthesis, and the bar that can be open or close).
// the letters B and R stand for binary and relation (the plus and the equal)

// Our problem is to add some <mrow> so that the small elements (l,r,m)
// will not change size in presence of a big element. The next function returns
// an empty list if there is no problem (no small or no big).
// Returns an empty list in the case $(\frac12)$.

void Math::find_paren0(MathP &aux) const {
    int  unbalance  = 0;
    bool seen_big   = false;
    bool inner_big  = false;
    bool seen_small = false;
    int  i          = 0;
    auto L          = begin();
    auto E          = end();
    int  nb_pairs   = 0;
    while (L != E) { // create the list of indices
        if (L->cmd == math_xml_cmd) {
            math_types t    = L->get_xmltype();
            bool       keep = false;
            switch (t) {
            case mt_flag_small_l:
                nb_pairs++;
                unbalance++;
                keep       = true;
                seen_small = true;
                break;
            case mt_flag_small_r:
                unbalance--;
                keep       = true;
                seen_small = true;
                break;
            case mt_flag_small_m:
                keep       = true;
                seen_small = true;
                break;
            case mt_flag_bin: keep = true; break;
            case mt_flag_rel: keep = true; break;
            case mt_flag_big:
                if (unbalance <= 0) inner_big = true;
                seen_big = true;
                keep     = true;
                break;
            default: break;
            }
            if (keep) aux.push_back({i, t});
        }
        ++L;
        i++;
    }
    if (seen_big && nb_pairs > 1) inner_big = true;
    if (!inner_big || !seen_small) { // only small objects, or no parens
        aux.clear();
    } else
        aux.push_back({i, mt_flag_big}); // add the end marker.
}

// The next function takes a math list, and returns a list of positions
// where to insert the <mrow> and </mrow>. In the case where find_paren0
// returns an empty list, there is nothing to do. Otherwise, the list is
// split into sublists, with the big as delimitors.
// For each sublist, if there is no small, nothing has to be done.
// After that, remove_binrel is called. In the example, we start with
// 0b 2l 4B 6r 9R 10m 12m 13b.
// The first sublist is empty, and the second will be
// start=0, 2l 6r 9R 10m 12m 13B
// (the start=0 means that the list starts after 0, the final 13B
// indicates that the list ends before the 13).
// Retval is false if rec, true otherwise
auto Math::find_parens(MathQList &res, bool verbose) const -> bool {
    MathP aux;
    find_paren0(aux);
    if (aux.empty()) return true;
    if (verbose) log_file << "MF: After find paren0 " << aux << "\n";
    if (aux.find_paren_rec(res)) {
        if (verbose) log_file << "MF: rec " << res << "\n";
        return false;
    }
    res.clear();
    int start = -1;
    int end   = -1;
    while (!aux.empty()) {
        if (aux.find_paren_matched1()) {
            aux.find_paren_matched2(res);
            if (verbose) the_log << "MF: matched " << aux_buffer << "\n";
            return true;
        }
        MathP content = aux.find_big(end);
        if (!content.has_small()) { // no small, look at next big
        } else {
            content.remove_binrel();               // remove useless bin/rel in content
            content.push_back({end, mt_flag_rel}); // add end marker
            if (verbose) log_file << "MF: sublist start=" << start << ' ' << content << "\n";
            content.find_paren2(start, res, verbose);
        }
        start       = end;
        int seen_d1 = 0, seen_d2 = 0;
        if (aux.is_lbr(seen_d1, seen_d2)) {
            if (verbose) log_file << "MF: LBR " << seen_d1 << ' ' << seen_d2 << "\n";
            res.push_back({seen_d1, seen_d2});
            return true;
        }
    }
    return true;
}

// --------------------------------------------------------------------

// This calls find_parens, which indicates where we should add some <mrow>
// and replaces some values. Uses a class for this
// Returns true if the object is big, false if small.
auto Math::finish_translate1(bool vb) -> bool {
    if (empty()) return false;
    MathF M;
    bool  retval = false;
    for (;;) {
        M.reset_MathF();
        bool final = find_parens(M.aux, vb);
        if (add_fence(final, M)) retval = true;
        if (final) return retval;
    }
}

// final is true for the final pass: calls remove_prefix when pushing in
auto Math::add_fence(bool final, MathF &M) -> bool {
    int i = 0;       // current index in the list
    M.reset_MathF(); // state is now true
    M.change_state();
    bool ret_val     = false;
    bool after_dummy = false;
    while (!empty()) {
        math_types cur_type = front().get_xmltype();
        if (cur_type == mt_flag_big) {
            ret_val = true;
            if (M.in_mrow) M.t_big = true;
        }
        if (after_dummy) {
            after_dummy = false;
            Xml *xval   = front().remove_prefix();
            if ((xval != nullptr) && xval->is_element() &&
                (xval->has_name_of("msup") || xval->has_name_of("msub") || xval->has_name_of("msubsup"))) {
                M.pop_last(xval);
            }
        }
        if (cur_type == mt_flag_dummy && (final || M.in_mrow)) after_dummy = true;
        if (M.next_change == i) {
            Xml *xval = front().remove_prefix();
            if (cur_type == mt_flag_dummy) {
                if (M.in_mrow) xval = nullptr;
                // elsestd::cout<< "bad dummy\n"; no error ??
            }
            if (xval != nullptr) M.push_in_t(xval);
            M.handle_t();
            M.change_state();
            if (M.next_change == i) {
                M.handle_t();
                M.change_state();
            }
        } else if (cur_type == mt_flag_dummy) {
            if (!(final || M.in_mrow)) M.push_in_res(front());
        } else if (!M.in_mrow) {
            if (final)
                M.push_in_res(front().remove_prefix());
            else
                M.push_in_res(front());
        } else
            M.push_in_t(front().remove_prefix());
        pop_front();
        ++i;
    }
    M.finish(*this);
    return ret_val;
}

// --------------------------------------------------------------------

// Case no mathml wanted. The result is $\alpha _3$

// This converts a delimiter position into a Latex command
void math_ns::insert_delimiter(del_pos k) {
    Buffer &B = mathml_buffer;
    switch (k) {
    case del_open_ket: B.append("&lt;"); return;
    case del_close_ket: B.append("&gt;"); return;
    case del_dot: B.append("."); return;
    case del_open_par: B.append("("); return;
    case del_close_par: B.append(")"); return;
    case del_close_bra: B.append("]"); return;
    case del_open_bra: B.append("["); return;
    case del_vert: B.append("|"); return;
    case del_Vert: B.append("\\Vert "); return;
    case del_rangle: B.append("\\rangle "); return;
    case del_langle: B.append("\\langle "); return;
    case del_rbrace: B.append("\\rbrace "); return;
    case del_lbrace: B.append("\\lbrace "); return;
    case del_rceil: B.append("\\rceil "); return;
    case del_lceil: B.append("\\lceil "); return;
    case del_rfloor: B.append("\\rfloor "); return;
    case del_lfloor: B.append("\\lfloor "); return;
    case del_rgroup: B.append("\\rgroup "); return;
    case del_lgroup: B.append("\\lgroup "); return;
    case del_rmoustache: B.append("\\moustache "); return;
    case del_lmoustache: B.append("\\moustache "); return;
    case del_uparrow: B.append("\\uparrow "); return;
    case del_downarrow: B.append("\\downarrow "); return;
    case del_updownarrow: B.append("\\updownarrow "); return;
    case del_Uparrow: B.append("\\Uparrow "); return;
    case del_Downarrow: B.append("\\Downarrow "); return;
    case del_Updownarrow: B.append("\\Updownarrow "); return;
    default: B.append("\\bad.");
    }
}

void math_ns::insert_delimiter_t(del_pos k) {
    Buffer &B = att_buffer;
    switch (k) {
    case del_open_ket: B.append("&lt;"); return;
    case del_close_ket: B.append("&gt;"); return;
    case del_dot: B.append("."); return;
    case del_open_par: B.append("("); return;
    case del_close_par: B.append(")"); return;
    case del_close_bra: B.append("]"); return;
    case del_open_bra: B.append("["); return;
    case del_vert: B.append("|"); return;
    case del_Vert: B.append("Vert"); return;
    case del_rangle: B.append("rangle"); return;
    case del_langle: B.append("langle"); return;
    case del_rbrace: B.append("rbrace"); return;
    case del_lbrace: B.append("lbrace"); return;
    case del_rceil: B.append("rceil"); return;
    case del_lceil: B.append("lceil"); return;
    case del_rfloor: B.append("rfloor"); return;
    case del_lfloor: B.append("lfloor"); return;
    case del_rgroup: B.append("rgroup"); return;
    case del_lgroup: B.append("lgroup"); return;
    case del_rmoustache: B.append("moustache"); return;
    case del_lmoustache: B.append("moustache"); return;
    case del_uparrow: B.append("uparrow"); return;
    case del_updownarrow: B.append("updownarrow"); return;
    case del_downarrow: B.append("downarrow"); return;
    case del_Uparrow: B.append("Uparrow"); return;
    case del_Downarrow: B.append("Downarrow"); return;
    case del_Updownarrow: B.append("Updownarrow"); return;
    default: B.append("bad.");
    }
}

// This inserts something like \alpha, with a space at the end when needed
// if the boolean space is false, no space is added at the end.
void Buffer::push_back_math_token(const CmdChr &x, bool space) {
    if (x.cmd > 16) {
        push_back('\\');
        auto s = x.name();
        push_back_math_aux(s);
        if (!space) return;
        if (s[0] == 0) return;
        if (s[1] == 0 && (std::isalpha(s[0]) == 0)) return;
        push_back(' ');
    } else
        append_with_xml_escaping(x.char_val());
}

// This inserts something like <alpha>,
// Depending on the type we produce <math/> <math> or </math>
// We must take

void Buffer::push_back_math_tag(const CmdChr &x, int type) {
    if (x.cmd > 16) {
        auto s = x.name();
        push_back_math_tag(s, type);
    } else { // Let's hope no tag needed here
        if (type == pbm_end) return;
        append_with_xml_escaping(x.char_val());
    }
}

// We must consider the case where the command is \& or the like
// We produce <elt name='&amp;'> otherwise
void Buffer::push_back_math_tag(std::string s, int type) {
    auto n  = s.size();
    bool ok = true; // true if letter
    for (size_t i = 0; i < n; i++) {
        auto c = s[i];
        if (std::isalpha(c) == 0) {
            if (s == "@root") {
                s = "root";
                break;
            }
            ok = false;
            break;
        }
    }
    push_back('<');
    if (type == pbm_end) push_back('/');
    if (ok)
        append(s);
    else {
        append("elt");
        if (type != pbm_end) {
            append(" name='");
            for (size_t i = 0; i < n; i++) push_back_xml_char(s[i]);
            append("'");
        }
    }
    if (type == pbm_att || type == pbm_att_empty) append(att_buffer);
    if (type == pbm_empty || type == pbm_att_empty) push_back('/');
    push_back('>');
}

// Case of a string, we check if the string can be inserted directly.
void Buffer::push_back_math_aux(std::string s) {
    auto n  = s.size();
    bool ok = true;
    for (size_t i = 0; i < n; i++) {
        auto c = s[i];
        if (c == '<' || c == '>' || c == '&' || static_cast<uchar>(c) < 32) {
            ok = false;
            break;
        }
    }
    if (ok)
        append(s);
    else
        for (size_t i = 0; i < n; i++) push_back_xml_char(s[i]);
}

// Not inline because of a bug of the Mac Compiler gcc4.2.1

auto MathElt::get_fml_subtype() const -> subtypes { return math_to_sub(get_lcmd()); }

// Handles the case of a command like \sqrt, \frac
void MathElt::cv_noML_special() const {
    subtypes c = get_fml_subtype();
    CmdChr   Val(special_math_cmd, c);
    Math &   L = get_list();
    if (c == formula_attribute_code || c == thismath_attribute_code) {
        static Buffer atb;
        std::string   s1 = L.get_arg1().convert_this_to_string(atb);
        std::string   s2 = L.get_arg2().convert_this_to_string(atb);
        auto          A  = std::string(s1);
        auto          B  = std::string(s2);
        math_ns::add_attribute_spec(A, B);
        return;
    }
    mathml_buffer.push_back_math_token(Val, false);
    int n = nb_args_for_cmd(c);
    if (n == -1) mathml_buffer.append("unknown");
    if (c == mathbox_code) {
        mathml_buffer.append("{");
        mathml_buffer.append(L.saved);
        mathml_buffer.append("}");
    }
    if (c == mathmi_code || c == mathmo_code || c == mathmn_code || c == mathnothing_code || c == mathci_code || c == mathcn_code ||
        c == mathcsymbol_code || c == multiscripts_code || c == mathbox_code) {
        auto X = L.begin();
        auto Y = L.end();
        ++X;
        for (;;) {
            if (X == Y) break;
            mathml_buffer.append("[");
            X->get_list().convert_math_noML0();
            ++X;
            mathml_buffer.append("='");
            if (X == Y) {
            } // Should we signal an error ?
            else {
                X->get_list().convert_math_noML0();
                ++X;
            }
            mathml_buffer.append("']");
        }
    }
    if ((c == smash_code || c == cfrac_code) && !L.empty()) {
        if (!L.get_arg1().empty()) {
            mathml_buffer.push_back('[');
            L.get_arg1().convert_math_noML0();
            mathml_buffer.push_back(']');
        }
        L.pop_front();
    }
    if (c == root_code) {
        mathml_buffer.append(" ");
        if (!L.empty()) {
            L.get_arg1().convert_math_noML0();
            L.pop_front();
            mathml_buffer.append(" \\of ");
        }
    }
    if (c == genfrac_code) {
        if (L.empty()) return;
        auto k = del_pos(L.front().chr);
        if (k == del_dot)
            mathml_buffer.append("{}");
        else
            math_ns::insert_delimiter(k);
        L.pop_front();
        if (L.empty()) return;
        k = del_pos(L.front().chr);
        if (k == del_dot)
            mathml_buffer.append("{}");
        else
            math_ns::insert_delimiter(k);
        L.pop_front();
        auto sz = L.front().payload;
        mathml_buffer += '{' + sz + '}';
        L.pop_front();
        if (L.front().cmd == style_cmd)
            mathml_buffer.format("{}", math_ns::style_level(L.front().chr));
        else
            mathml_buffer.append("{}");
        L.pop_front();
    }
    if (c == xleftarrow_code || c == xrightarrow_code) {
        n = 1;
        if (!L.empty()) {
            if (!L.get_arg1().empty()) {
                mathml_buffer.push_back('[');
                L.get_arg1().convert_math_noML0();
                L.pop_front();
                mathml_buffer.push_back(']');
            }
        }
    }
    while (n > 0) {
        n--;
        mathml_buffer.push_back('{');
        if (!L.empty()) {
            L.get_arg1().convert_math_noML0();
            L.pop_front();
        }
        mathml_buffer.push_back('}');
    }
}

// Handles the case of a command like \sqrt, \frac
void MathElt::cv_noMLt_special() const {
    subtypes c = get_fml_subtype();
    CmdChr   Val(special_math_cmd, c);
    mathml_buffer.push_back_math_tag(Val, pbm_start);
    cv_noMLt_special0();
    mathml_buffer.push_back_math_tag(Val, pbm_end);
}

void MathElt::cv_noMLt_special0() const {
    subtypes c = get_fml_subtype();
    int      n = nb_args_for_cmd(c);
    Math &   L = get_list();
    if (n == -1) mathml_buffer.append("unknown");
    if (c == mathbox_code) {
        mathml_buffer.append("<name>");
        mathml_buffer.append(L.saved);
        mathml_buffer.append("</name>");
    }
    if (c == mathmi_code || c == mathmo_code || c == mathmn_code || c == mathnothing_code || c == mathci_code || c == mathcn_code ||
        c == mathcsymbol_code || c == multiscripts_code || c == mathbox_code) {
        auto X = L.begin();
        auto Y = L.end();
        ++X;
        mathml_buffer.pop_back();
        for (;;) {
            if (X == Y) break;
            mathml_buffer.append(" ");
            X->get_list().convert_math_noMLt0();
            ++X;
            mathml_buffer.append("='");
            if (X == Y) {
            } // Should we signal an error ?
            else {
                X->get_list().convert_math_noMLt0();
                ++X;
            }
            mathml_buffer.append("'");
        }
        mathml_buffer.push_back('>'); // re-insert
    }
    if ((c == smash_code || c == cfrac_code) && !L.empty()) {
        if (!L.get_arg1().empty()) {
            mathml_buffer.append("<oarg>");
            L.get_arg1().convert_math_noMLt0();
            mathml_buffer.append("</oarg>");
        }
        L.pop_front();
    }
    if (c == root_code) {
        if (!L.empty()) {
            mathml_buffer.append("<oarg>");
            L.get_arg1().convert_math_noMLt0();
            L.pop_front();
            mathml_buffer.append("</oarg>");
        }
    }
    if (c == genfrac_code) {
        if (L.empty()) return;
        mathml_buffer.pop_back();
        auto k = del_pos(L.front().chr);
        if (k != del_dot) {
            att_buffer.clear();
            math_ns::insert_delimiter_t(k);
            mathml_buffer += " left='" + att_buffer + "'";
        }
        L.pop_front();
        if (!L.empty()) {
            k = del_pos(L.front().chr);
            if (k != del_dot) {
                att_buffer.clear();
                math_ns::insert_delimiter_t(k);
                mathml_buffer += " right='" + att_buffer + "'";
            }
            L.pop_front();
        }
        if (!L.empty()) {
            att_buffer = L.front().payload;
            if (!att_buffer.empty()) mathml_buffer += " size='" + att_buffer + "'";
            L.pop_front();
        }
        if (!L.empty()) {
            if (L.front().cmd == style_cmd) mathml_buffer.format(" style='{}'", math_ns::style_level(L.front().chr));
            L.pop_front();
        }
        mathml_buffer.push_back('>');
    }
    if (c == xleftarrow_code || c == xrightarrow_code) {
        n = 1;
        if (!L.empty()) {
            if (!L.get_arg1().empty()) {
                mathml_buffer.append("<oarg>");
                L.get_arg1().convert_math_noML0();
                L.pop_front();
                mathml_buffer.append("</oarg>");
            }
        }
    }
    // optimize this
    if (n == 1 && !L.empty()) {
        L.get_arg1().convert_math_noMLt0();
        L.pop_front();
        --n;
    }
    while (n > 0) {
        n--;
        mathml_buffer.append("<arg>");
        if (!L.empty()) {
            L.get_arg1().convert_math_noMLt0();
            L.pop_front();
        }
        mathml_buffer.append("</arg>");
    }
}

// Handles the case of a group
void MathElt::cv_noML_list() const {
    Math &         X = get_list();
    math_list_type T = get_lcmd();
    switch (T) {
    case math_open_cd:
        mathml_buffer.push_back('{');
        X.convert_math_noML0();
        mathml_buffer.push_back('}');
        return;
    case math_dollar_cd:
        mathml_buffer.push_back('$');
        X.convert_math_noML0();
        mathml_buffer.push_back('$');
        return;
    case math_LR_cd: X.convert_math_noML0(); return;
    case math_env_cd:
        mathml_buffer += "\\begin{" + X.get_name() + "}";
        X.convert_math_noML0();
        mathml_buffer += "\\end{" + X.get_name() + "}";
        return;
    case math_hbox_cd:
        mathml_buffer += "\\" + X.get_name() + "{";
        X.convert_math_noML0();
        mathml_buffer += "}";
        return;
    default: mathml_buffer.format("bad group{}", T);
    }
}

// Handles the case of a group
void MathElt::cv_noMLt_list() const {
    Math &         X = get_list();
    math_list_type T = get_lcmd();
    switch (T) {
    case math_open_cd:
        mathml_buffer.append("<arg>");
        X.convert_math_noMLt0();
        mathml_buffer.append("</arg>");
        return;
    case math_dollar_cd:
        mathml_buffer.append("<math>");
        X.convert_math_noMLt0();
        mathml_buffer.append("</math>");
        return;

    case math_LR_cd: X.convert_math_noMLt0(); return;
    case math_env_cd:
        mathml_buffer += "<env name='" + X.get_name() + "'>";
        X.convert_math_noMLt0();
        mathml_buffer += "</env>";
        return;
    case math_hbox_cd:
        mathml_buffer += "<" + X.get_name() + ">";
        X.convert_math_noMLt0();
        mathml_buffer += "</" + X.get_name() + ">";
        return;
    default: mathml_buffer.append("bad group");
    }
}

// Define these two macros, because of similar code
#define SELF_INSERT_CASES                                                                                                                  \
    case space_catcode:                                                                                                                    \
    case other_catcode:                                                                                                                    \
    case hat_catcode:                                                                                                                      \
    case underscore_catcode:                                                                                                               \
    case alignment_catcode:                                                                                                                \
    case char_given_cmd:                                                                                                                   \
    case math_given_cmd

#define NORMAL_CASES                                                                                                                       \
    case cst1_cmd:                                                                                                                         \
    case mathordb_cmd:                                                                                                                     \
    case mathord_cmd:                                                                                                                      \
    case mathbin_cmd:                                                                                                                      \
    case mathrel_cmd:                                                                                                                      \
    case mathinner_cmd:                                                                                                                    \
    case mathbetween_cmd:                                                                                                                  \
    case mathopen_cmd:                                                                                                                     \
    case mathop_cmd:                                                                                                                       \
    case mathopn_cmd:                                                                                                                      \
    case mathclose_cmd:                                                                                                                    \
    case big_cmd:                                                                                                                          \
    case mathspace_cmd:                                                                                                                    \
    case backslash_cmd:                                                                                                                    \
    case nonscript_cmd:                                                                                                                    \
    case style_cmd:                                                                                                                        \
    case math_comp_cmd:                                                                                                                    \
    case math_font_cmd:                                                                                                                    \
    case multicolumn_cmd:                                                                                                                  \
    case limits_cmd:                                                                                                                       \
    case over_cmd:                                                                                                                         \
    case ref_cmd

// This inserts a general token.
void MathElt::cv_noML() {
    switch (cmd) {
    SELF_INSERT_CASES:
        mathml_buffer.append_with_xml_escaping(get_char());
        return;
    case letter_catcode:
        if (get_char() == '&')
            mathml_buffer.push_back('&');
        else
            mathml_buffer.append_with_xml_escaping(get_char());
        return;
    NORMAL_CASES:
        mathml_buffer.push_back_math_token(*this, true);
        return;
    case mathfont_cmd: {
        size_t c = chr;
        if (c >= 15) c = 0;
        auto w = the_names.mml(c);
        if (w.empty()) return;
        mathml_buffer.append("\\");
        mathml_buffer.append(encode(w));
        mathml_buffer.push_back(' ');
        return;
    }
    case left_cmd: // left or right
    case right_cmd:
        mathml_buffer.push_back_math_token(*this, false);
        math_ns::insert_delimiter(del_pos(chr));
        return;
    case special_math_cmd: // \frac{}{}
        cv_noML_special();
        return;
    case math_list_cmd: // \left...\right
        cv_noML_list();
        return;
    case relax_cmd: {
        static Buffer rb;
        rb.clear();
        rb.push_back(Token(get_font()));
        if (rb == "\\relax") return;
        mathml_buffer.append(rb);
        mathml_buffer.push_back(' ');
    }
        return;
    case hspace_cmd:
        mathml_buffer.push_back_math_token(*this, false);
        mathml_buffer.push_back('{');
        mathml_buffer.push_back(ScaledInt(get_font()), glue_spec_pt);
        mathml_buffer.push_back('}');
        return;
    default: mathml_buffer.push_back_math_token(*this, true); return;
    }
}

// This inserts a general token.
void MathElt::cv_noMLt() {
    switch (cmd) {
    SELF_INSERT_CASES:
        mathml_buffer.append_with_xml_escaping(get_char());
        return;
    case letter_catcode:
        if (get_char() == '&')
            mathml_buffer.push_back('&');
        else
            mathml_buffer.append_with_xml_escaping(get_char());
        return;
    NORMAL_CASES:
        mathml_buffer.push_back_math_tag(*this, pbm_empty);
        return;
    case mathfont_cmd: {
        size_t c = chr;
        if (c >= 15) c = 0;
        auto w = the_names.mml(c);
        if (w.empty()) return;
        mathml_buffer += "<font name='" + encode(w) + "'/>"; // \todo why not go through all the machinery here?
        return;
    }
    case left_cmd: // left or right
    case right_cmd:
        att_buffer = " del='";
        math_ns::insert_delimiter_t(del_pos(chr));
        att_buffer += "'";
        mathml_buffer.push_back_math_tag(*this, pbm_att_empty);
        return;
    case special_math_cmd: // \frac{}{}
        cv_noMLt_special();
        return;
    case math_list_cmd: // \left...\right
        cv_noMLt_list();
        return;
    case relax_cmd: {
        auto T = Token(get_font());
        auto x = T.hash_loc();
        auto s = hash_table[x];
        mathml_buffer.push_back_math_tag(s, pbm_empty);
        return;
    }
    case hspace_cmd: {
        String name = chr == 1 || chr == 3 ? "vspace" : "hspace";
        if (chr >= 2) att_buffer = " star='true'";
        mathml_buffer.push_back_math_tag(name, chr < 2 ? pbm_start : pbm_att);
        mathml_buffer.push_back(ScaledInt(get_font()), glue_spec_pt);
        mathml_buffer.push_back_math_tag(name, pbm_end);
        return;
    }
    default: mathml_buffer.push_back_math_tag(*this, pbm_empty); return;
    }
}

// This converts the whole formula. There are some funny exceptions.
void Math::convert_math_noML0() {
    while (!empty()) {
        MathElt cur = front();
        pop_front();
        auto cmd = cur.cmd;
        if (cmd == mathfont_cmd && (empty() || front().cmd == mathfont_cmd)) continue;
        cur.cv_noML();
    }
}

// Same with tags

void Math::convert_math_noMLt0() {
    while (!empty()) {
        MathElt cur = front();
        pop_front();
        auto cmd = cur.cmd;
        if (cmd == mathfont_cmd && (empty() || front().cmd == mathfont_cmd)) continue;
        if (!empty() && (cmd == hat_catcode || cmd == underscore_catcode)) {
            cur = front();
            pop_front();
            std::string s = cmd == hat_catcode ? "superscript" : "subscript";
            mathml_buffer += "<" + s + ">";
            cur.cv_noMLt();
            mathml_buffer += "</" + s + ">";
            continue;
        }
        cur.cv_noMLt();
    }
}

// Main function. Converts the buffer into XML, adds attributes.
// If spec is true, we produce <in/> otherwise \in
auto Math::convert_math_noML(bool spec) -> Xml * {
    mathml_buffer.clear();
    if (spec)
        convert_math_noMLt0();
    else
        convert_math_noML0();
    return new Xml(the_names["texmath"], new Xml(std::string(mathml_buffer)));
}

// --------------------------------------------------
// Special hacks.
// This is to make the other file smaller

// True if the list contains only digits, that are put in the buffer.
auto Math::only_digits(Buffer &B) const -> bool {
    if (empty()) return false;
    for (const auto &L : *this) {
        if (!L.is_digit()) { return false; }
        B.push_back(L.get_char());
    }
    return true;
}

// Put as many characters as possible in the buffer.
// Returns true if everything was inserted.
// If rec is true, the argument can contain groups; In the case the buffer
// is not reset !!
auto Math::chars_to_mb(Buffer &B, bool rec) const -> bool {
    if (!rec) B.clear();
    auto L = begin();
    auto E = end();
    for (;;) {
        if (L == E) return true;
        // DEBUG   std::cout<< "cmd:" << B << ".\n";
        CmdChr w = *L;
        if (w.is_space() || w.is_letter() || w.is_other()) {
            char32_t c = w.char_val();
            if (c == '<')
                B.append("&lt;");
            else if (c == '>')
                B.append("&gt;");
            else
                B.push_back(c);
        } else if (w.cmd == cst1_cmd && w.chr == sharp_code)
            B.append("#");
        else if (w.cmd == cst1_cmd && w.chr == underscore_code)
            B.append("_");
        else if (w.cmd == cst1_cmd && w.chr == amp_code)
            B.append("&amp;");
        else if (w.cmd == char_given_cmd)
            B.append_with_xml_escaping(w.char_val());
        else if (w.cmd == relax_cmd)
            continue;
        else if (rec && w.cmd == math_list_cmd && L->get_font() == subtypes(math_open_cd)) {
            if (!L->get_list().chars_to_mb(B, true)) return false;
        } else {
            Logger::finish_seq();
            log_and_tty << "First invalid token in math-to-string cmd=" << w.cmd << " chr=" << w.chr << "\n";
            std::cout << "\n";
            return false;
        }
        ++L;
    }
}

// Slightly modified procedure.
auto Math::chars_to_mb1(Buffer &B) const -> bool {
    B.clear();
    auto L = begin();
    auto E = end();
    for (;;) {
        if (L == E) return true;
        CmdChr w = *L;
        if (w.is_space()) {
        } // spaces are ignored
        else if (w.is_letter() || w.is_other())
            B.push_back(w.char_val());
        else if (w.cmd == cst1_cmd && w.chr == comma_code)
            B.append(" ");
        else if (w.cmd == relax_cmd)
            continue;
        else if (w.cmd == mathspace_cmd)
            B.append(" ");
        else
            return false;
        ++L;
    }
}

// Yet another procedure. Reads the dimension in a command like \above
// Something like -1,2cm  is OK
auto Math::chars_to_mb3() -> std::string {
    Buffer B;
    int    bc   = 0; // unit size
    int    sz   = 0; // current size
    bool   dot  = false;
    bool   sign = false;
    for (;;) {
        if (empty()) break;
        if (!front().is_char()) break;
        char32_t C = front().get_char();
        pop_front();
        if (C == '+' || C == '-') {
            if ((sz != 0) || (bc != 0)) {
                sz = 0;
                break;
            }
            if (C == '-') sign = !sign;
            continue;
        }
        if (C == ',') C = char32_t('.');
        if (C == '.') {
            if (dot || (bc != 0)) {
                sz = 0;
                break;
            }
            if (sz == 0) {
                if (sign) B += "-";
                B += "0";
            }
            B.push_back(C);
            dot = true;
            sz++;
            continue;
        }
        if (std::isdigit(static_cast<int>(C)) != 0) {
            if (bc != 0) {
                sz = 0;
                break;
            }
            if (sz == 0 && sign) B += "-";
            B.push_back(C);
            sz++;
        } else { // unit
            B.push_back(C);
            bc++;
            if (bc == 2) break;
        }
    }
    if (sz == 0 || bc != 2) {
        Buffer tmp;
        tmp.clear();
        tmp += "Error scanning width, so far got '" + B + "'";
        the_parser.parse_error(the_parser.err_tok, tmp, "bad dimension");
        B = "0pt";
    }
    return std::move(B);
}

// Procedure called in case of errors
void Buffer::show_uncomplete(String m) {
    the_parser.signal_error(m);
    if (empty())
        log_and_tty << "No character found\n";
    else
        log_and_tty << "So far, got " << *this << "\n";
    clear();
    append("error");
}

// Converts a whole math list into a string. May signal an error.
// In this case, the result is `error'.
auto Math::convert_this_to_string(Buffer &B) const -> std::string {
    B.clear();
    if (!chars_to_mb(B, true)) B.show_uncomplete("Bad character in conversion to string");
    return B;
}

// Use of the alternate command
auto Math::convert_opname() const -> std::string {
    Buffer &B = aux_buffer;
    if (!chars_to_mb1(B)) B.show_uncomplete("Bad character in conversion to string");
    return B;
}

// Case of \begin{foo}[bar]{gee}ETC, spaces are not yet removed
// This removes the optional argument of a list.
// If argument star is true, removes a *
// The value is in math_buffer, but not used at all.
void Math::remove_opt_arg(bool star) {
    Buffer &B = aux_buffer;
    skip_initial_space();
    if (star && !empty() && front().is_star()) {
        pop_front();
        skip_initial_space();
    }
    if (empty()) return;
    if (!front().is_bracket()) return;
    pop_front();
    while (!empty() && front().is_char()) {
        char32_t c = front().get_char();
        pop_front();
        if (c == ']') return;
        B.push_back(c);
    }
    the_parser.signal_error("missing ]");
}

// Case of \begin{foo}[bar]{gee}ETC, spaces are not yet removed
// Returns the value of a required argument, as a string.
// Here it can be "gee"; this is a sublist.
auto Math::remove_req_arg() -> std::string {
    skip_initial_space();
    if (empty() || !front().is_list()) {
        the_parser.signal_error("missing argument");
        return "error";
    }
    const Math &L = front().get_list(); // the sublist containing the argument
    pop_front();
    return L.convert_this_to_string(aux_buffer);
}

// --------------------------------------------------

// The code in this section tries to replace $10^{i\`eme}$, or absurdities
// like that by 10\textsuperscript{e}.

// This realises the \textsuperscript, given the translation of the argument
auto math_ns::make_sup(Xml *xval) -> Xml * {
    Xml *tmp = fonts1("sup");
    tmp->push_back_unless_nullptr(xval);
    return tmp;
}

// This is the first step. Assumes OK false, res=0.
// Leaves OK false, and sets res to the translation of the formula) if only
// digits given.
// In case X^Y, where Y is the tail (can be a group), and X is formed
// of digits, OK is true, res the value of X.

void Math::special2(bool &ok, Xml *&res) const {
    Buffer &B = aux_buffer;
    B.clear();
    for (auto L = begin(); L != end(); ++L) {
        if (L->cmd == hat_catcode) {
            ++L;                    // skip over hat
            if (L == end()) return; // a final hat should not appear
            ++L;
            if (L != end()) return; // hack only in case single object after hat
            ok = true;
            break;
        }
        if (L->is_digit())
            B.push_back(L->get_char());
        else if (L->is_list()) {
            if (!L->get_list().only_digits(B)) return;
        } else
            return;
    }
    if (!B.empty()) res = new Xml(std::string(B));
}

// This handles the exponent. The case 10^e and 10^o is handled
// elsewhere. We consider here only 10^{something}
auto MathElt::special3() const -> Xml * {
    if (!is_list()) return nullptr;
    const_math_iterator L1;
    const_math_iterator E1;
    get_list().is_font_cmd1_list(L1, E1);
    return special_exponent(L1, E1);
}

// Assumes that \textrm{toto}  gives {\rm toto} (wrong!)
// Contructs iterators to begin/end of the list, unless the list is
// a single \hbox, case where the sublist is used.
void Math::is_font_cmd1_list(const_math_iterator &B, const_math_iterator &E) {
    B = begin();
    E = end();
    if (B == E) return;
    ++B;
    bool ok = false;
    if (B != E) ok = true;
    --B;
    if (ok) return;
    if (front().is_hbox()) {
        Math &X = front().get_list();
        B       = X.begin();
        E       = X.end();
    }
}

// True if it is a group containing \grave{e}
auto MathElt::is_e_grave() const -> bool {
    if (cmd != special_math_cmd && get_font() != grave_code) return false;
    Math &  A = get_list();
    Buffer &B = mathml_buffer; // not aux_buffer !!
    if (!A.get_arg1().chars_to_mb(B, false)) return false;
    if (B.single_char() != 'e') return false;
    return true;
}

// This is the main function.
auto Math::special1() const -> Xml * {
    bool ok = false;
    Xml *U{nullptr};
    special2(ok, U);
    if (!ok) return U;
    const MathElt &W = back();
    Xml *          xval{nullptr};
    if (W.cmd == letter_catcode && W.get_char() == 'o')
        xval = math_ns::get_builtin(xml_o_loc);
    else if (W.cmd == letter_catcode && W.get_char() == 'e')
        xval = math_ns::get_builtin(xml_e_loc);
    else {
        xval = W.special3();
        if (xval == nullptr) return nullptr;
    }
    xval = math_ns::make_sup(xval);
    if (U == nullptr) return xval;
    Xml *res = new Xml(the_names["temporary"], nullptr);
    res->push_back_unless_nullptr(U);
    res->push_back_unless_nullptr(xval);
    return res;
}

// ------------------------------------------------------------

// Math font handler.
// \mathcal{xy} gives \cal xy\oldfont, where \oldfont is the current font
// stored somewhere in eqtb. This does not interpret the fonts;
// It justs reads the tokens, and backinputs them
void Parser::TM_fonts() {
    static std::vector<Token> table;
    if (table.empty()) {
        table.push_back(hash_table.locate("mml@font@normal"));
        table.push_back(hash_table.locate("mml@font@upright"));
        table.push_back(hash_table.locate("mml@font@bold"));
        table.push_back(hash_table.locate("mml@font@italic"));
        table.push_back(hash_table.locate("mml@font@bolditalic"));
        table.push_back(hash_table.locate("mml@font@script"));
        table.push_back(hash_table.locate("mml@font@boldscript"));
        table.push_back(hash_table.locate("mml@font@fraktur"));
        table.push_back(hash_table.locate("mml@font@doublestruck"));
        table.push_back(hash_table.locate("mml@font@boldfraktur"));
        table.push_back(hash_table.locate("mml@font@sansserif"));
        table.push_back(hash_table.locate("mml@font@boldsansserif"));
        table.push_back(hash_table.locate("mml@font@sansserifitalic"));
        table.push_back(hash_table.locate("mml@font@sansserifbolditalic"));
        table.push_back(hash_table.locate("mml@font@monospace"));
    }
    size_t T    = 0;
    bool   bold = is_pos_par(atmathversion_code);
    if (cur_cmd_chr.cmd == math_font_cmd) {
        switch (cur_cmd_chr.chr) {
        case cal_code:
            back_input(table[bold ? math_f_bold_script : math_f_script]);
            back_input(hash_table.nomathsw0_token);
            return;
        case mathtt_code: T = math_f_monospace; break;
        case mathcal_code: T = bold ? math_f_bold_script : math_f_script; break;
        case mathbf_code: T = math_f_bold; break;
        case mathrm_code: T = math_f_upright; break;
        case mathfrak_code: T = bold ? math_f_bold_fraktur : math_f_fraktur; break;
        case mathit_code: T = bold ? math_f_bold_italic : math_f_italic; break;
        case mathnormal_code: T = bold ? math_f_bold : math_f_normal; break;
        case mathbb_code: T = math_f_doublestruck; break;
        case mathsf_code: T = bold ? math_f_bold_sansserif : math_f_sansserif; break;
        default: T = size_t(-1);
        }
    } else { // this is a textfont
        switch (cur_cmd_chr.chr) {
        case rm_family_code: T = math_f_upright; break;
        case sf_family_code: T = bold ? math_f_bold_sansserif : math_f_sansserif; break;
        case tt_family_code: T = math_f_monospace; break;
        case bf_series_code: T = math_f_bold; break;
        case it_shape_code:
            T = bold ? math_f_bold_italic : math_f_italic;
            break;
            //    case sc_shape_code:
        case sl_shape_code:
        case normalfont_code: T = math_f_normal; break;
        default: T = size_t(-1);
        }
    }
    if (cur_cmd_chr.cmd == oldfont_cmd || cur_cmd_chr.cmd == noargfont_cmd) {
        if (T != size_t(-1)) {
            back_input(table[T]);
            back_input(hash_table.nomathsw0_token);
        }
        return;
    }
    TokenList L = read_arg();
    auto      c = to_unsigned(eqtb_int_table[math_font_pos].val);
    back_input(table[c]);
    back_input(hash_table.nomathsw0_token);
    L.push_back(hash_table.nomathsw1_token);
    L.push_back(Token(other_t_offset, '}'));
    L.push_back(hash_table.endgroup_token);
    L.push_front(Token(other_t_offset, '{'));
    L.push_front(hash_table.nomathsw1_token);
    L.push_front(hash_table.begingroup_token);
    back_input(L);
    if (T != size_t(-1)) {
        back_input(table[T]);
        back_input(hash_table.nomathsw0_token);
    }
}

// Convert the character c  into <mi>c</mi>
auto math_ns::mk_mi(char32_t c) -> Xml * {
    aux_buffer.clear();
    aux_buffer.append_with_xml_escaping(c);
    Xml *x = new Xml(std::string(aux_buffer));
    return new Xml(the_names["mi"], x);
}

// Converts a letter with a into into <mi mathvariant='foo'>X</mi>
// Assumes 2<=font<=14 and 'a'<=c<='z' || 'A'<=c<='Z'
auto math_ns::mk_mi(uchar c, size_t font) -> Xml * {
    Xml *x = single_chars[c];
    Xml *y = new Xml(the_names["mi"], x);
    y->add_att(the_names["mathvariant"], the_names.cstf(font));
    return y;
}

// True if this can form a sequence of characters to put in a <mi>
auto MathElt::maybe_seq() const -> bool {
    if (cmd != letter_catcode) return false;
    if (get_font() == 0) return false;
    auto c = chr;
    return c < 128 && (std::isalpha(char(uchar(c))) != 0);
}

// True is this can form a sequence of characters to put in a <mi>
// with the same font as F
auto MathElt::maybe_seq(subtypes f) const -> bool {
    if (cmd != letter_catcode) return false;
    if (get_font() != f) return false;
    auto c = chr;
    return c < 128 && (std::isalpha(char(uchar(c))) != 0);
}

// True is this can form a sequence of digits to put in a <mn>
auto MathElt::maybe_iseq() const -> bool {
    if (cmd != other_catcode) return false;
    auto c = chr;
    return c < 128 && (std::isdigit(uchar(c)) != 0);
}

// True is this can form a sequence of characters to put in a <mn>
// with the same font as F
auto MathElt::maybe_iseq(subtypes f) const -> bool {
    if (cmd != other_catcode) return false;
    if (get_font() != f) return false;
    auto c = chr;
    return c < 128 && (std::isdigit(uchar(c)) != 0);
}

// Converts a character sequence; first char W already removed from
// the list
auto Math::convert_char_seq(const MathElt &W) -> MathElt {
    subtypes f = W.get_font();
    auto     w = eqtb_int_table[mathprop_ctr_code].val;
    Xml *    res{nullptr};
    Buffer & B = aux_buffer;
    B.clear();
    if (f == 1) B.push_back(' ');
    bool     spec = (f == 1) || ((w & (1 << f)) != 0);
    unsigned c    = W.chr;
    if (spec)
        B.push_back(char(uchar(c)));
    else
        B.append(math_chars[uchar(c)][f]);
    for (;;) {
        if (empty()) break;
        if (!front().maybe_seq(f)) break;
        c = front().chr;
        if (spec)
            B.push_back(char(uchar(c)));
        else
            B.append(math_chars[uchar(c)][f]);
        pop_front();
    }
    if (f == 1) B.push_back(' ');
    res = new Xml(std::string(B));
    res = new Xml(the_names["mi"], res);
    if (f > 1 && spec) res->add_att(the_names["mathvariant"], the_names.cstf(f));
    return MathElt(res, mt_flag_small);
}

// Converts a character sequence; first char W already removed from
// the list; if bool true reads some chars, otherwise,
// reads only a single one.
auto Math::convert_char_iseq(const MathElt &W, bool multiple) -> MathElt {
    subtypes f = W.get_font();
    Buffer & B = aux_buffer;
    B.clear();
    unsigned c = W.chr;
    B.push_back(char(uchar(c)));
    if (multiple)
        for (;;) {
            if (empty()) break;
            if (!front().maybe_iseq(f)) break;
            c = front().chr;
            B.push_back(char(uchar(c)));
            pop_front();
        }
    Xml *res = new Xml(std::string(B));
    res      = new Xml(the_names["mn"], res);
    if (f > 1) res->add_att(the_names["mathvariant"], the_names.cstf(f));
    return MathElt(res, mt_flag_small);
}

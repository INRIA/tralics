// Tralics, a LaTeX to XML translator.
// Copyright INRIA/apics/marelle (Jose' Grimm) 2002, 2004, 2007-2015

// This software is governed by the CeCILL license under French law and
// abiding by the rules of distribution of free software.  You can  use,
// modify and/ or redistribute the software under the terms of the CeCILL
// license as circulated by CEA, CNRS and INRIA at the following URL
// "http://www.cecill.info".
// (See the file COPYING in the main directory for details)

// This file contains a big part of the Tralics translator

#include "tralics/ColSpec.h"
#include "tralics/Logger.h"
#include "tralics/MainClass.h"
#include "tralics/SaveAux.h"
#include "tralics/Saver.h"
#include <fmt/format.h>
#include <fmt/ostream.h>

#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif

void readline_newprompt(std::string s); // in readline.cpp, but only used here

namespace {
    Buffer current_head;
    Xml *  unfinished_par{nullptr};
    Token  T_theequation, T_theparentequation, T_at_theparentequation;

    const std::string list[] = {"sup", "sub", "oldstyle", "caps", "hl", "so", "st", "ul"};

#ifdef _MSC_VER
    inline void txsleep(unsigned i) { Sleep(1000 * i); }
#else
    inline void txsleep(unsigned i) { sleep(i); }
#endif

    auto st_bool(bool x) -> std::string { return x ? the_names["true"] : the_names["false"]; }

    // Returns the number of environments
    auto nb_env_on_stack() -> int {
        auto n = the_save_stack.size();
        int  k = 0;
        for (size_t i = n; i > 0; i--) {
            SaveAuxBase *p = the_save_stack[i - 1].get();
            if (p->type == st_env) ++k;
        }
        return k;
    }

    // Reads the tokens, converts them to dimension.
    auto dimen_attrib(ScaledInt A) -> std::string {
        Buffer B;
        B.push_back(A, glue_spec_empty);
        auto i = B.size();
        if (i > 0 && B[i - 1] == '0') {
            B.pop_back();
            i--;
        }
        if (i > 0 && B[i - 1] == '.') B.pop_back();
        return std::string(B);
    }

    // This prints a control sequence value on the log file.
    // Used when tracing a command (catcode not 11 nor 12)
    // used in the case {\let\x\y}, after the closing brace.
    // It it's not a char, it's a command, with a plain ASCII name.
    void print_cmd_chr(CmdChr X) {
        String a = X.special_name();
        auto   b = X.name();
        the_log << "\\" << b;
        if (a != nullptr) the_log << " " << a;
    }
} // namespace

namespace translate_ns {
    auto find_color(const std::string &model, const std::string &value) -> std::string;
} // namespace translate_ns

// This code translates everything, until end of file.
void Parser::translate_all() {
    unprocessed_xml.clear();
    while (!get_x_token()) {
        if (tracing_commands()) translate02();
        if (!translate03()) throw EndOfData();
    }
    flush_buffer();
}

// Same, with a test that unprocessed_xml is empty
// In general, the loop in translate_all will stop because
// restricted is true, and token list is exhausted
void Parser::translate0() {
    if (!unprocessed_xml.empty()) missing_flush();
    translate_all();
}

// Main function: translates a token list.
// Result is added to the current element on the stack.
void Parser::T_translate(TokenList &X) {
    if (X.empty()) return;
    SaveState s(TL, restricted);
    restricted = true;
    TL.swap(X);
    if (!unprocessed_xml.empty()) missing_flush();
    translate_all();
}

// This prints the command to translate. The case of a space is special
// Letters are printed elsewhere
void Parser::translate02() const {
    if (cur_cmd_chr.is_space()) {
        if (!the_stack.in_v_mode()) Logger::out_single_char(char32_t(' '));
        return;
    }
    if (cur_cmd_chr.is_letter_other()) return;
    Logger::finish_seq();
    the_log << "{";
    print_cmd_chr(cur_cmd_chr);
    the_log << "}\n";
}

// Flushes the buffer, creating a new xml element that will be added
// to the main tree.
void Parser::flush_buffer1() {
    if (tracing_commands()) {
        Logger::finish_seq();
        the_log << "{Text:" << unprocessed_xml.convert_to_log_encoding() << "}\n";
    }
    the_stack.add_last_string(unprocessed_xml);
    unprocessed_xml.clear();
}

// as above, does nothing if the buffer is empty,
void Parser::flush_buffer() {
    if (!unprocessed_xml.empty()) flush_buffer1();
}

// Same, but removes final newline. When TeX sees an empty line, it generates
// 2 tokens: a space and \par. Here we remove the space.
void Parser::flush_buffer0() {
    if (!unprocessed_xml.empty()) flush_buffer1();
    the_stack.remove_last_space();
}

// Quits vertical mode. Said otherwise, opens a new paragraph.
void Parser::leave_v_mode() {
    flush_buffer();
    if (the_stack.in_v_mode()) ileave_v_mode();
}

auto Parser::ileave_v_mode() -> Xid {
    Xid res = the_stack.push_par(cur_centering());
    if (unfinished_par != nullptr) {
        res.add_attribute(unfinished_par->id.get_att(), true);
        unfinished_par = nullptr;
    }
    return res;
}

// Quits horizontal mode. Said otherwise finishes a paragraph.
void Parser::leave_h_mode() {
    if (the_stack.in_h_mode()) {
        unfinished_par = nullptr;
        flush_buffer0();
        the_stack.pop(the_names["cst_p"]);
        Xml *p = the_stack.top_stack()->back_or_nullptr();
        if ((p != nullptr) && p->is_empty_p()) the_stack.top_stack()->pop_back();
        the_stack.add_nl();
    }
}

// translates {foo}
void Parser::T_arg() {
    TokenList L = read_arg();
    T_translate(L);
}

// translates {foo} in a group
void Parser::T_arg_local() {
    push_level(bt_local);
    TokenList L = read_arg();
    T_translate(L);
    pop_level(bt_local);
}

// translates [foo]
void Parser::T_optarg() {
    auto L = read_optarg().value_or(TokenList{});
    T_translate(L);
}

// Translates a list of token in argument mode. Returns the value.
auto Parser::translate_list(TokenList &L) -> Xml * {
    Xml *res = the_stack.temporary();
    T_translate(L);
    the_stack.pop(the_names["argument"]);
    return res;
}

// Same, but reads the list of tokens.
auto Parser::xT_arg_nopar() -> Xml * {
    TokenList L = read_arg_nopar();
    return translate_list(L);
}

// Same, but returns a string
auto Parser::sT_arg_nopar() -> std::string { return xT_arg_nopar()->convert_to_string(); }

// translates a token list, result is a string
auto Parser::sT_translate(TokenList &L) -> std::string { return translate_list(L)->convert_to_string(); }
// Return an Id location for the next argument
auto Parser::nT_arg_nopar() -> std::string {
    std::string s = sT_arg_nopar();
    return std::string(s);
}

// Return 0 if the argument is empty or does not exist.
auto Parser::xT_optarg_nopar() -> Xml * {
    auto L = read_optarg().value_or(TokenList{});
    return L.empty() ? nullptr : translate_list(L);
}

// Hacked version of sT_arg_nopar.
auto Parser::special_next_arg() -> std::string {
    auto guard1 = SaveCatcode('~', other_catcode);
    auto guard2 = SaveCatcode('#', active_catcode);
    auto guard3 = InLoadHandler();
    auto guard4 = InUrlHandler();
    return sT_arg_nopar();
}

// Returns next optional argument as a string
auto Parser::sT_optarg_nopar() -> std::string {
    Xml *res = xT_optarg_nopar();
    if (res == nullptr) return "";
    return res->convert_to_string();
}

// Returns next optional argument as an attribute value.
// return 0 if the argument is empty or does not exist.
auto Parser::nT_optarg_nopar() -> std::optional<std::string> {
    auto L = read_optarg_nopar().value_or(TokenList{});
    if (L.empty()) return {};
    Xml *       x = translate_list(L);
    std::string y = x->convert_to_string();
    return std::string(y);
}

// First argument of minipage should be [c] [t] or [b]
// Third arg allows [s]
// Second argument of \makebox \framebox should be [c] [l] or [r] or [s]

auto Parser::get_ctb_opt() -> std::optional<std::string> {
    auto L = read_optarg_nopar().value_or(TokenList{});
    if (L.empty()) return {};
    Token t = token_ns::get_unique(L);
    if (t.is_null()) return {};
    if (t.cmd_val() != letter_catcode) return {};
    auto c = t.val_as_letter();
    if (c == 'c') return "c";
    if (c == 's') return "s";
    if (c == 't') return "t";
    if (c == 'b') return "b";
    if (c == 'l') return "l";
    if (c == 'r') return "r";
    return {};
}

// Nodes: tblr, or 2 letters
auto Parser::get_trees_opt() -> std::optional<std::string> {
    auto L = read_optarg_nopar().value_or(TokenList{});
    if (L.empty()) return {};
    Token t1, t2;
    token_ns::get_unique(L, t1, t2);
    if (t1.is_null()) return {};
    if (t1.cmd_val() != letter_catcode) return {};
    auto c = t1.val_as_letter();
    if (t2.is_null()) {
        if (c == 'l') return "l";
        if (c == 'r') return "r";
        if (c == 't') return "t";
        if (c == 'b') return "b";
        return {};
    }
    if (t2.cmd_val() != letter_catcode) return {};
    auto c2 = t2.val_as_letter();
    if (c != 't' && c != 'b') return {};
    if (c2 != 'l' && c2 != 'r') return {};
    if (c == 't') return c2 == 'l' ? "tl" : "tr";
    return c2 == 'l' ? "bl" : "br";
}

// In the case where the font has changed and we are in text mode, we call
// check_font.
void Parser::font_has_changed1() {
    if (!the_stack.in_v_mode() && !cur_font.is_ok()) {
        flush_buffer();
        the_stack.check_font();
    }
}

// This is done whenever we see an explicit font change.
void Parser::font_has_changed() {
    cur_font.pack();
    save_font();
    font_has_changed1();
    cur_font.update_old();
}

// Case of a command like \rmfamily.
void Parser::see_font_change(subtypes c) {
    cur_font.see_font_change(c);
    font_has_changed();
}

// Case of a command like \small.
void Parser::translate_font_size() {
    cur_font.change_size(cur_cmd_chr.chr);
    font_has_changed();
}

// \rm is : \normalfont\rmfamily (in latex, it changes also math fonts)
void Parser::old_font() {
    cur_font.kill();
    see_font_change(cur_cmd_chr.chr);
}

// \textit{x} is coded as {\itshape x}
void Parser::arg_font(subtypes c) {
    flush_buffer();
    before_mac_arg();
    push_level(bt_brace);
    see_font_change(c);
}

// \textsuperscript, \textsubscript, etc.
void Parser::T_fonts(const std::string &x) {
    leave_v_mode();
    Xml *res = fonts1(x);
    the_stack.push(the_names["fonts"], res);
    T_arg();
    the_stack.pop(the_names["fonts"]);
}

void Parser::T_fonts(subtypes c) { T_fonts(list[c]); }

// \indent or \noindent. If we are in a <p>,  and the <p> is empty, then we
// add noindent=true or noindent=false, unless there is already a noindent
// attribute. If we are in a <p>, cur par is not empty, we first close the <p>
// if we are not in <p>, we open a new paragraph.
void Parser::implicit_par(subtypes c) {
    bool noindent = c == zero_code;
    flush_buffer();
    if (the_stack.in_h_mode() && cur_centering() != 1) {
        Xml *cp = the_stack.get_cur_par();
        if (cp == nullptr) {
            parse_error("Invalid \\par command");
            return;
        }
        if (cp->par_is_empty()) {
            cp->id.add_attribute(the_names["noindent"], st_bool(noindent), false);
            return;
        }
    }
    finish_par_cmd(noindent, std::string());
}

void Parser::finish_par_cmd(bool noindent, const std::string &xs) {
    leave_h_mode();
    auto k  = cur_centering();
    Xid  id = ileave_v_mode();
    if (!xs.empty()) id.add_attribute(the_names["space_before"], xs);
    if (k != 1) id.add_attribute(the_names["noindent"], st_bool(noindent));
}

// Translation of \par
void Parser::T_par1() {
    if (the_stack.in_v_mode()) return;
    if (the_stack.in_no_mode()) return;
    std::string frame = the_stack.first_frame();
    if (frame == the_names["hbox"]) return;
    flush_buffer();
    if (the_stack.in_h_mode()) {
        Xml *cp = the_stack.get_cur_par();
        if (cp == nullptr) {
            parse_error("Invalid \\par command: paragraph not started");
        } else if (cp->par_is_empty()) {
            unfinished_par = cp;
            the_stack.pop(the_names["cst_p"]);
            Xml *tp = the_stack.top_stack();
            if (tp->back_or_nullptr() == cp)
                tp->pop_back();
            else
                cp->name = std::string();
            return;
        }
    }
    leave_h_mode();
}

// Translation of \\[dim], u is the att_val of the dimension
void Parser::T_par1(const std::string &u) {
    flush_buffer();
    finish_par_cmd(true, u);
}

// User function that adds some element. Is leave_v_mode necessary ?
// if w generates <foo/> otherwise <foo>...</foo>
void Parser::T_xmlelt(subtypes w) {
    flush_buffer();
    std::string s   = sT_arg_nopar();
    Xml *       res = new Xml(std::string(s), nullptr);
    if (w != 0U) {
        if (w == two_code) res->id = size_t(-1); // XML comment
        flush_buffer();
        the_stack.add_last(res);
        return;
    }
    leave_v_mode();
    auto name = std::string("xmlelt");
    the_stack.push(name, res);
    T_arg();
    the_stack.pop(name);
}

// Special case where the XML name and the frame name are the same
// Note the additional braces
void Parser::T_arg1(const std::string &y) {
    the_stack.push1(y);
    TokenList L = read_arg();
    L.brace_me();
    T_translate(L);
    the_stack.pop(y);
}

// Case of an item in a list...
void Parser::T_item(subtypes c) {
    leave_h_mode();
    unfinished_par = nullptr;
    the_stack.pop_if_frame(the_names["item"]);
    skip_initial_space_and_back_input();
    std::string att = T_item_label(c);
    the_stack.add_nl();
    the_stack.push1(the_names["item"]);
    if (!att.empty()) the_stack.add_att_to_last(the_names["labelitem"], att);
    the_stack.add_new_anchor();
    the_stack.set_v_mode();
    skip_initial_space_and_back_input();
    finish_par_cmd(true, std::string());
}

// c=1 in the case of \@item (new scheme)
auto Parser::T_item_label(unsigned c) -> std::string {
    TokenList L;
    bool      opt = cur_tok.is_open_bracket();
    if (opt) L = read_optarg_nopar().value_or(TokenList{});
    std::string list_ctr = the_parser.eqtb_string_table[1].val;
    if (!list_ctr.empty()) refstepcounter(list_ctr, false);
    if (!opt) {
        Token t = hash_table.itemlabel_token;
        token_from_list(t);
        if (!cur_cmd_chr.is_relax())
            L.push_back(t);
        else
            return std::string();
    }
    L.brace_me(); // \item[\bf x] puts only x in \bf
    the_stack.push1(the_names["labelitem"]);
    the_stack.set_arg_mode();
    T_translate(L);
    the_stack.pop(the_names["labelitem"]);
    if (!((c != 0) || get_cur_env_name() == "enumerate")) return std::string();
    Xml *res      = the_stack.remove_last();
    res->name     = std::string("");
    std::string w = res->convert_to_string();
    return std::string(w);
}

// Case of \glo{item}{some text} inside an glossaire env.
void Parser::T_glo() {
    leave_h_mode(); // mode should be no_mode now.
    mode w = the_stack.get_mode();
    the_stack.set_arg_mode();
    T_arg1(the_names["gloitem"]);
    the_stack.push1(the_names["item"]);
    the_stack.set_v_mode();
    T_arg();
    leave_h_mode();
    the_stack.pop(the_names["item"]);
    the_stack.set_mode(w);
}

// Case of \chapter, \section, etc. Here Y is the title, and y the index
// (it is 7 for old module, 8 for new module).
// Since there are no more old modules  7 is replaced by 8
// We have opened the current XML element,
// we have to allow for a label and read the title.
void Parser::start_paras(int y, const std::string &Y, bool star) {
    bool module_p = y == 7 || y == 8;
    if (!star || module_p) {
        if (y == 0)
            the_stack.add_new_anchor_spec();
        else
            the_stack.add_new_anchor();
    }
    Xml *opt{nullptr};
    if (module_p)
        create_label("mod:" + Y, the_stack.get_cur_id()); // should be the id above
    else
        opt = xT_optarg_nopar();
    the_stack.set_arg_mode();
    the_stack.push1(the_names["head"]);
    Xml *     title = the_stack.top_stack();
    TokenList L     = read_arg();
    if (module_p) check_module_title(L);
    L.brace_me();
    T_translate(L);
    current_head.clear();
    title->put_in_buffer(current_head);
    the_stack.pop(the_names["head"]);
    if (opt != nullptr) the_stack.add_last(new Xml(the_names["alt_section"], opt));
    the_stack.add_nl();
    the_stack.set_v_mode();
    static int first_print_level = 10;
    if (module_p) first_print_level = 0;
    auto YY = current_head.convert_to_log_encoding();
    if (y <= first_print_level) {
        first_print_level = y;
        spdlog::info("Translating section command {}: {}.", Y, YY);
    }
    Logger::finish_seq();
    the_log << "Translating " << Y << ": " << YY << ".\n";
}

// An error is signaled if the title of the module is empty
void Parser::check_module_title(TokenList &L) {
    static int ctr = 0;
    ++ctr;
    token_ns::remove_initial_spaces(L);
    if (!L.empty()) return;
    signal_error("Empty module name replaced!");
    L = token_ns::string_to_list(ctr == 1 ? "Overall Objectives" : "Introduction", false);
}

// Translates \frontmatter, \mainmatter, \backmatter
void Parser::T_matter(subtypes c) {
    leave_h_mode();
    the_stack.end_module();
    the_stack.add_nl();
    std::string t = "mainmatter";
    if (c == frontmatter_code)
        t = "frontmatter";
    else if (c == backmatter_code)
        t = "backmatter";
    the_stack.push1(the_names["module"], the_names[t]);
    chapter_has_star = (c == frontmatter_code || c == backmatter_code);
}

// Translates \section, \chapter, etc.
void Parser::T_paras(subtypes x) {
    if (x == toplevel_sec_code) {
        TokenList L = read_arg();
        if (!L.empty()) {
            token_from_list(L.front());
            sectionning_offset = section_code;
            if (cur_cmd_chr.cmd == section_cmd) sectionning_offset = cur_cmd_chr.chr;
            if (sectionning_offset == chapter_code) Xid(1).add_attribute(the_names["chapters"], the_names["true"]);
            if (sectionning_offset == part_code) Xid(1).add_attribute(the_names["part"], the_names["true"]);
        }
        return;
    }
    int y = int(x) - int(sectionning_offset);
    if (x == endsec_code) {
        y           = 0;
        TokenList L = read_arg();
        if (!L.empty()) {
            token_from_list(L.front());
            if (cur_cmd_chr.cmd == section_cmd) y = int(cur_cmd_chr.chr) - int(sectionning_offset);
        }
    }
    if (y < 0) y = 0;
    if (y > 6) y = 6;
    std::string Y = the_names.cstdiv(to_unsigned(y));
    leave_h_mode();
    the_stack.para_aux(y); // this pops the stack...
    the_stack.add_nl();
    if (x == endsec_code) return;
    the_stack.push1(Y, the_names.npdiv(to_unsigned(y)));
    bool star = remove_initial_star();
    if (chapter_has_star && x == chapter_code) star = true;
    if (star)
        last_att_list()[the_names["rend"]] = the_names["nonumber"];
    else {
        if (x == part_code)
            refstepcounter("part", false);
        else if (x == chapter_code)
            refstepcounter("chapter", false);
        else if (x == section_code)
            refstepcounter("section", false);
        else if (x == subsection_code)
            refstepcounter("subsection", false);
        else if (x == subsubsection_code)
            refstepcounter("subsubsection", false);
        else if (x == paragraph_code)
            refstepcounter("paragraph", false);
        else if (x == subparagraph_code)
            refstepcounter("subparagraph", false);
        else
            star = true;
    }
    start_paras(y, Y, star);
}

// Translates \ref or \pageref
void Parser::T_ref(bool is_ref) {
    std::string a = special_next_arg();
    the_stack.add_newid0("ref");
    Xid X = the_stack.get_xid();
    X.add_ref(a);
    if (!is_ref) X.add_attribute(the_names["rend"], the_names["page"]);
}

// \begin or \end of subequations
void Parser::T_subequations(bool start) {
    static bool in_subequations = false;
    if (!start) { // easy part: \global\c@equation=\c@parentequation
        if (!in_subequations) {
            parse_error("Illegal \\endsubequations");
            return;
        }
        in_subequations = false;
        auto v          = eqtb_int_table[equation_ctr_pos + 1].val;
        word_define(equation_ctr_pos, v, true);
        state = state_S;
        return;
    }
    if (in_subequations) {
        parse_error("Illegal nesting of subequations environment");
        return;
    }
    in_subequations = true;
    leave_h_mode();
    static bool tokscreated = false;
    if (!tokscreated) {
        tokscreated            = true;
        T_theequation          = hash_table.locate("theequation");
        T_theparentequation    = hash_table.locate("theparentequation");
        T_at_theparentequation = hash_table.locate("@@theequation");
    }
    the_stack.add_newid0("anchor");
    refstepcounter("equation", true);
    back_input(hash_table.CB_token);
    back_input(T_theequation);
    back_input(hash_table.OB_token);
    TokenList theparent;
    read_mac_body(theparent, true, 0);
    new_macro(theparent, T_theparentequation);
    auto v = eqtb_int_table[equation_ctr_pos].val;
    word_define(equation_ctr_pos + 1, v, true); // par = cur
    word_define(equation_ctr_pos, 0, true);     // cur = 0
    M_let_fast(T_theequation, T_at_theparentequation, false);
}

auto Parser::scan_anchor(bool &h) -> std::string {
    static int anchor_id = 0;
    anchor_id++;
    h             = remove_initial_star();
    std::string s = sT_optarg_nopar();
    if (!s.empty()) return s;
    mac_buffer = fmt::format("{}", anchor_id);
    return mac_buffer;
}

// \label \anchor or \anchorlabel
void Parser::T_label(subtypes c) {
    if (c == 0) { // case of label
        std::string a = special_next_arg();
        create_label(a, the_stack.get_cur_id());
        return;
    }
    bool        h  = 0;
    std::string s  = scan_anchor(h);
    std::string id = the_stack.add_anchor(s, h);
    if (c == 2) {
        std::string a = special_next_arg();
        create_label(a, id);
    }
}

// Commands for the float package
void Parser::T_float(subtypes c) {
    switch (c) {
    case 0: //@float
    case 1: // @dblfloat
    {
        Buffer &    B    = Tbuf;
        std::string sarg = sT_arg_nopar();
        auto        arg  = std::string(sarg);
        auto        opt  = nT_optarg_nopar();
        if (!opt) {
            B = "fps@" + sarg;
            expand_no_arg(B);
            opt = nT_arg_nopar();
        }
        word_define(incentering_code, 1, false);
        leave_h_mode();
        the_stack.push1(the_names["float"]);
        the_stack.add_att_to_last(the_names["place"], *opt);
        if (c == 1) the_stack.add_att_to_last(the_names["starred"], the_names["true"]);
        the_stack.add_att_to_last(the_names["type"], arg);
        B = "fname@" + sarg;
        expand_no_arg(B);
        opt = nT_arg_nopar();
        the_stack.add_att_to_last(the_names["name"], *opt);
        refstepcounter(sarg, true);
        B = "@float@every@" + sarg;
        back_input(hash_table.locate(B));
        back_input(hash_table.locate("the"));
        B = "fst@" + sarg;
        back_input(hash_table.locate(B));
        the_stack.set_v_mode();
        return;
    }
    case 2: // float@end
    case 3: // float@dblend
        leave_h_mode();
        if (!the_stack.top_stack()->has_name_of("float")) parse_error("no float on stack");
        the_stack.pop(the_names["float"]);
        the_stack.add_nl();
        return;
    default: return;
    }
}

// Subfigure. Should appear only in a figure env.
void Parser::T_subfigure() {
    leave_v_mode();
    the_stack.push1(the_names["subfigure"]);
    refstepcounter("subfigure", true);
    the_stack.set_arg_mode();
    the_stack.push1(the_names["leg"]);
    T_optarg();
    the_stack.pop(the_names["leg"]);
    {
        auto guard = SaveCatcode('_', 13); // allow underscore in the file name (needed ?)
        T_arg1(the_names["texte"]);
    }
    the_stack.pop(the_names["subfigure"]);
}

// Case of &. Works only inside a table (math code is elsewhere).
void Parser::T_ampersand() {
    if (the_stack.is_frame("cell"))
        finish_a_cell(hash_table.endv_token, std::string());
    else if (the_stack.is_frame2("hanl")) {
        LC();
        unprocessed_xml += "&amp;"; // hack...
        return;
    } else
        parse_error(err_tok, "Unexpected &", "Unexpected ampersand");
}

// This interprets \newline.
void Parser::T_newline() {
    if (the_stack.is_frame("cst_p"))
        T_par1(std::string());
    else if (the_stack.in_v_mode())
        return;
    else
        parse_error("Bad use of \\newline");
}

// This interprets \\.
void Parser::T_backslash() {
    Token T = cur_tok;
    flush_buffer();
    remove_initial_star();
    std::string a = get_opt_dim(T);
    if (the_stack.is_frame("head"))
        back_input(hash_table.headercr_token);
    else if (the_stack.is_frame("cst_p")) {
        T_par1(a);
        remove_initial_space_and_back_input(); // Needed because we are in hmode
    } else if (the_stack.is_frame("cell"))
        finish_a_cell(hash_table.cr_token, a);
    else if (the_stack.in_v_mode())
        return;
    else if (the_stack.is_omit_cell())
        back_input(hash_table.space_token);
    else
        parse_error("Bad use of \\\\");
}

void Parser::T_grabenv() {
    Token     cmd = get_r_token();
    TokenList value;
    skip_initial_space_and_back_input();
    if (nb_env_on_stack() == 0) {
        parse_error("\\grabenv must be used in an environment");
    } else
        grab_env(value);
    new_macro(value, cmd, true);
}

// This is the code of \begin{motscle}
void Parser::T_keywords() {
    leave_h_mode();
    the_stack.push1(the_names["keywords"]);
    the_stack.add_nl();
    the_stack.set_no_mode();
    for (;;) {
        TokenList v;
        bool      seen_end = grab_env_comma(v);
        token_ns::remove_first_last_space(v);
        if (!v.empty() && v.back().is_dot()) v.pop_back();
        the_stack.push1(the_names["term"]);
        the_stack.set_arg_mode();
        T_translate(v);
        the_stack.pop(the_names["term"]);
        the_stack.add_nl();
        if (seen_end) break;
    }
}

// Handle the case of an argument of \includegraphics
void Parser::no_extension(AttList &AL, const std::string &s) {
    long k  = -1;
    bool ok = true;
    Tbuf.clear();
    Tbuf.append(s);
    size_t i  = 0;
    size_t ii = 0;
    for (;;) {
        if (Tbuf[i] == '.' && Tbuf[i + 1] == '.' && Tbuf[i + 2] == '/') {
            i += 3;
            continue;
        }
        if (Tbuf[i] == '.' && Tbuf[i + 1] == '/') {
            if (ii == 0) ii = 2; // compatibility
            i += 2;
            continue;
        }
        break;
    }
    if (Tbuf[i] == '.') ok = false;
    for (;; i++) {
        if (Tbuf[i] == 0) {
            if (k == to_signed(i) - 1) ok = false;
            break;
        }
        if (Tbuf[i] == '.') {
            if (k == -1)
                k = to_signed(i);
            else {
                ok = false;
                break;
            }
        }
    }
    if (!ok) {
        TokenList L = Tbuf.str_toks11(false);
        L.brace_me();
        back_input(L);
        back_input(hash_table.locate("@filedoterr"));
    }
    if (ok && k > 0) {
        AL[the_names["file_extension"]] = Tbuf.substr(to_unsigned(k) + 1);
        Tbuf.resize(to_unsigned(k));
    }
    enter_file_in_table(Tbuf.substr(ii), ok);
    AL[the_names["file"]] = Tbuf;
}

void Parser::default_bp(Buffer &B, Token T, TokenList &val) {
    val.push_back(Token(letter_t_offset, 'b'));
    val.push_back(Token(letter_t_offset, 'p'));
    dimen_from_list0(T, val);
    B.push_back(cur_val.get_dim_val(), glue_spec_pt);
}

// chr =0 for \includegraphics, 1 for \psfig
void Parser::includegraphics(subtypes C) {
    bool   ic   = C == 0;
    String bkey = "Bad key in argument of of includegraphics";
    String bval = "Bad value in argument of includegraphics";
    Token  T    = cur_tok;
    remove_initial_star();
    Token       comma  = hash_table.comma_token;
    Token       equals = Token(other_t_offset, '=');
    TokenList   W, val, key;
    std::string file_name_2;
    {
        auto guard = InLoadHandler();
        if (ic) {
            W = read_optarg().value_or(TokenList{});
            flush_buffer();
            file_name_2 = sT_arg_nopar();
        } else
            W = read_arg();
    }
    expand_no_arg("Gin@keys");
    {
        TokenList K = read_arg();
        W.splice(W.begin(), K);
    }
    leave_v_mode();
    AttList &AL = the_stack.add_newid0("figure");
    if (ic) no_extension(AL, file_name_2);
    Buffer &B = tpa_buffer;
    while (!W.empty()) {
        token_ns::split_at(comma, W, val);
        token_ns::remove_initial_spaces(val);
        if (val.empty()) continue;
        token_ns::split_at(equals, val, key);
        token_ns::remove_first_last_space(key);
        if (key.empty()) continue;
        token_ns::remove_first_last_space(val);
        std::string skey = list_to_string_c(key, bkey);
        B.clear();
        if (!ic && (skey == "file" || skey == "figure")) {
            std::string sval = list_to_string_c(val, bval);
            no_extension(AL, sval);
            ic = true;
        } else if (skey == "keepaspectratio" || skey == "clip" || skey == "draft" || skey == "hiresbb") {
            std::string V    = "true";
            std::string sval = list_to_string_c(val, bval);
            if (sval == "false") V = "false";
            if (skey == "clip")
                AL[the_names["clip"]] = the_names[V];
            else
                AL[skey] = the_names[V];
        } else if (skey == "type" || skey == "ext" || skey == "read" || skey == "command" || skey == "origin" || skey == "scale" ||
                   skey == "angle") {
            std::string sval = list_to_string_c(val, bval);
            if (skey == "angle" && sval == "0") continue;
            std::string p = skey == "scale" ? the_names["scale"] : skey == "angle" ? the_names["angle"] : std::string(skey);
            AL[p]         = sval;
        } else if (skey == "width" || skey == "height" || skey == "totalheight") {
            std::string N = skey == "height" ? "height" : skey == "width" ? "width" : "totalwidth";
            ScaledInt   s = dimen_from_list(T, val);
            B.push_back(s, glue_spec_pt);
            AL[the_names[N]] = B;
        } else if (skey == "natwidth" || skey == "natheight" || skey == "bbllx" || skey == "bblly" || skey == "bburx" || skey == "bbury") {
            default_bp(B, T, val);
            AL[skey] = B;
        } else if (skey == "bb" || skey == "viewport" || skey == "trim") {
            TokenList aux;
            auto      SP = Token(space_token_val);
            for (int i = 0; i < 4; i++) {
                token_ns::remove_initial_spaces(val);
                val.push_back(SP);
                token_ns::split_at(SP, val, aux);
                default_bp(B, T, aux);
                if (i < 3) B.push_back(' ');
            }
            AL[skey] = B;
        } else
            invalid_key(T, skey, val);
    }
    AL[the_names["rend"]] = the_names["inline"];
}

void Parser::T_epsfbox() {
    size_t    xdim_pos = 11; //  \epsfxsize hard-coded
    size_t    ydim_pos = 12; // \epsfysize hard-coded
    ScaledInt xdim     = eqtb_dim_table[xdim_pos].val;
    ScaledInt ydim     = eqtb_dim_table[ydim_pos].val;
    flush_buffer();
    std::string y;
    {
        auto guard = InLoadHandler();
        y          = sT_arg_nopar();
    }
    AttList &res = the_stack.add_newid0("figure");
    no_extension(res, y);
    res[the_names["rend"]] = the_names["inline"];
    if (!(xdim == ScaledInt{0})) res[the_names["width"]] = std::string(xdim);
    if (!(ydim == ScaledInt{0})) res[the_names["height"]] = std::string(ydim);
    dim_define(xdim_pos, ScaledInt(0), false); // reset to 0
    dim_define(ydim_pos, ScaledInt(0), false);
}

// \hspace or \vspace. Expands to \hskip argument\relax.
void Parser::T_hspace(subtypes c) {
    Token t = cur_tok;
    remove_initial_star();
    scan_glue(it_glue, t, false);
    append_glue(t, cur_val.get_glue_width(), c == 1);
}

// Code of \vspace, or \vskip, after we have fetched the dimension.
void Parser::append_glue(Token T, ScaledInt dimen, bool vert) {
    if (!vert) {
        auto dim = dimen.value;
        leave_v_mode();
        auto k = ((dim >> 16) + 2) / 4;
        while (k > 0) {
            k--;
            process_char(char32_t(0xA0U));
        }
        return;
    }
    // Now we have a \vskip.
    if (the_stack.in_h_mode()) {
        flush_buffer();
        Xml *cp = the_stack.get_cur_par();
        if (cp == nullptr) {
            parse_error("Expected a p element on the stack"); //  this is bad.
            return;
        }
        if (cp->par_is_empty()) {
            add_vspace(T, dimen, cp->id);
            return;
        }
    }
    leave_h_mode(); // start a new par if needed.
    if (!the_stack.in_v_mode()) return;
    Xid cp = ileave_v_mode();
    add_vspace(T, dimen, cp);
}

// Find a color in the stack, returns the id;
// May add a new item to the stack
auto translate_ns::find_color(const std::string &model, const std::string &value) -> std::string {
    auto n = all_colors.size();
    for (size_t i = 0; i < n; i++)
        if (all_colors[i].compare(model, value)) return all_colors[i].get_id();
    all_colors.emplace_back("", model, value);
    return all_colors[n].get_id();
}

// case of \color{red} or \color[rgb]{1,0,0}
// In the first case, we look at \color@red if this is a command
// with code color_md, subtype N, its color N-offset in the table.
auto Parser::scan_color(const std::string &opt, const std::string &name) -> std::string {
    if (opt.empty() || opt == "named") { // case \color{red} or \color[named]{Red}
        Buffer &B = tpa_buffer;
        B         = "\\color@" + name;
        token_from_list(hash_table.locate(B));
        if ((cur_cmd_chr.cmd == color_cmd) && (cur_cmd_chr.chr >= color_offset)) {
            auto k = cur_cmd_chr.chr - color_offset;
            if (k < all_colors.size()) return all_colors[k].get_id();
        }
        parse_error(err_tok, "Undefined color ", name, "undefined color");
        return std::string();
    }
    return translate_ns::find_color(opt, name);
}

// Implements color and variants (code = color_cmd)
void Parser::T_color(subtypes c) {
    flush_buffer();
    if (c == normalcolor_code) {
        cur_font.set_color(std::string());
        font_has_changed();
        return;
    }
    if (c == color_code) {
        std::string opt  = sT_optarg_nopar();
        std::string name = sT_arg_nopar();
        std::string C    = scan_color(opt, name);
        cur_font.set_color(C);
        font_has_changed();
        return;
    }
    if (c == pagecolor_code) {
        std::string opt         = sT_optarg_nopar();
        std::string name        = sT_arg_nopar();
        std::string C           = scan_color(opt, name);
        AttList &   res         = the_stack.add_newid0("pagecolor");
        res[the_names["color"]] = C;
    }
    if (c == colorbox_code) {
        std::string opt  = sT_optarg_nopar();
        std::string name = sT_arg_nopar();
        std::string C    = scan_color(opt, name);
        Xml *       mbox = internal_makebox();
        mbox->id.add_attribute(the_names["color"], C);
        return;
    }
    if (c == fcolorbox_code) {
        std::string opt   = sT_optarg_nopar();
        std::string name1 = sT_arg_nopar();
        std::string name2 = sT_arg_nopar();
        std::string C1    = scan_color(opt, name1);
        std::string C2    = scan_color(opt, name2);
        Xml *       mbox  = internal_makebox();
        mbox->id.add_attribute(the_names["color"], C1);
        mbox->id.add_attribute(the_names["color2"], C2);
        return;
    }
    if (c == definecolor_code) {
        std::string name  = sT_arg_nopar();
        std::string model = sT_arg_nopar();
        std::string value = sT_arg_nopar();
        tpa_buffer        = "\\color@" + name;
        Token C           = hash_table.locate(tpa_buffer);
        if (!Hashtab::the_eqtb()[C.eqtb_loc()].val.is_undef()) log_and_tty << "Redefining color " << name << "\n";
        if (model == "named") {
            // case \definecolor{myred}{named}{red}
            // is \global\let\color@myred = \color@red
            tpa_buffer = "\\color@" + value;
            Token T    = hash_table.locate(tpa_buffer);
            M_let_fast(C, T, true);
            return;
        }
        // case \definecolor{myred}{rgb}{1,2,3}
        // is \global\let \color@myred \colorN
        auto n = all_colors.size();
        all_colors.emplace_back(name, model, value);
        CmdChr v(color_cmd, subtypes(n + color_offset));
        eq_define(C.eqtb_loc(), v, true);
        if (tracing_assigns()) {
            Logger::finish_seq();
            the_log << "{Globally changing " << C << " into color number " << n << "}\n";
        }
        return;
    }
    if (c >= color_offset) {
        auto k = c - color_offset;
        if (k < all_colors.size()) {
            std::string C = all_colors[k].get_id();
            cur_font.set_color(C);
            font_has_changed();
        }
    }
}

// Add the given dimension as spacebefore value to the paragraph x.
void Parser::add_vspace(Token T, ScaledInt dimen, Xid x) {
    AttList &L = x.get_att();
    auto *   K = L.lookup(the_names["space_before"]);
    if (K != nullptr) {
        TokenList La = token_ns::string_to_list(*K, false);
        list_to_glue(it_glue, T, La);
        dimen += cur_val.get_glue_width();
    }
    auto k = std::string(dimen);
    x.add_attribute(the_names["space_before"], k, true);
}

auto Parser::internal_makebox() -> Xml * {
    leave_v_mode();
    the_stack.push1(the_names["mbox"]);
    Xml *     mbox = the_stack.top_stack();
    TokenList d    = read_arg();
    d.brace_me();
    T_translate(d);
    the_stack.pop(the_names["mbox"]);
    return mbox;
}

// Translates \makebox and \mbox, \text
void Parser::T_mbox(subtypes c) {
    Token       T = cur_tok;
    std::string ipos;
    std::string iwidth;
    if (c == makebox_code) {
        flush_buffer();
        skip_initial_space_and_back_input();
        if (cur_tok.is_open_paren()) {
            T_makebox(false, T);
            return;
        }
        iwidth = get_opt_dim(T);
        auto x = get_ctb_opt();
        if (x) ipos = the_names[*x];
    }
    Xml *mbox = internal_makebox();
    if (!(ipos.empty() && iwidth.empty())) {
        if (!ipos.empty()) mbox->id.add_attribute(the_names["box_pos"], ipos);
        mbox->id.add_attribute(the_names["box_width"], iwidth);
        return;
    }
    // Hack the box
    Xml *u = mbox->single_non_empty();
    if ((u != nullptr) && u->has_name_of("figure")) mbox->kill_name();
    if (std::none_of(mbox->begin(), mbox->end(), [](Xml *t) { return t->is_element(); })) mbox->kill_name();
    if (mbox->only_hi()) mbox->kill_name();
}

// This translates \caption or \footnote
void Parser::T_cap_or_note(bool cap) {
    leave_v_mode();
    std::string name = cap ? "scaption" : "footnote";
    push_level(bt_local);
    word_define(incentering_code, 0, false);
    the_stack.push1(the_names[name]);
    Xml *opt{nullptr};
    Xml *note{nullptr};
    if (cap) { // case of Caption
        opt  = xT_optarg_nopar();
        note = the_stack.top_stack();
        the_stack.set_v_mode();
        T_arg();
        leave_h_mode();
    } else {             // case of footnote, locally redefines fonts
        ignore_optarg(); // is this OK ?
        the_stack.add_att_to_last(the_names["place"], the_names["foot_position"]);
        refstepcounter("footnote", true);
        note        = the_stack.top_stack();
        FontInfo sv = cur_font;
        cur_font.kill();
        cur_font.set_color(std::string());
        cur_font.change_size(6);
        font_has_changed();
        the_stack.set_v_mode();
        T_arg();
        leave_h_mode();
        cur_font = sv;
        font_has_changed();
    }
    the_stack.pop(the_names[name]);
    if (opt != nullptr) the_stack.add_last(new Xml(the_names["alt_caption"], opt));
    pop_level(bt_local);
    if (the_main.footnote_hack) note->remove_par_bal_if_ok();
}

void Parser::T_makebox(bool framed, Token C) {
    std::string A, B;
    T_twodims(A, B, C);
    std::string oarg = sT_optarg_nopar();
    leave_v_mode();
    the_stack.push1(the_names["box"]);
    AttList &cur = last_att_list();
    if (framed) cur[the_names["framed"]] = the_names["true"];
    if (!oarg.empty()) cur[the_names["box_pos"]] = oarg;
    cur[the_names["height"]] = B;
    cur[the_names["width"]]  = A;
    T_arg_local();
    the_stack.pop(the_names["box"]);
}

// Implements \sbox and \savebox
void Parser::T_save_box(bool simple) {
    Token     T = cur_tok;
    TokenList L = read_arg();
    back_input(hash_table.equals_token);
    back_input(L);
    auto i = scan_reg_num();
    scan_optional_equals();
    skip_initial_space_and_back_input();
    //  leave_v_mode();
    flush_buffer();
    if (!simple && cur_tok.is_open_paren()) {
        T_makebox(false, T);
    } else {
        std::optional<std::string> ipos;
        std::optional<std::string> iwidth;
        if (!simple) {
            iwidth = get_opt_dim(T);
            auto x = get_ctb_opt();
            if (x) ipos = the_names[*x];
        }
        the_stack.push1(the_names["mbox"]);
        the_stack.set_arg_mode();
        Xml *     mbox = the_stack.top_stack();
        TokenList d    = read_arg();
        d.brace_me();
        T_translate(d);
        the_stack.pop(the_names["mbox"]);
        if (ipos && ipos->empty()) ipos.reset();       // \todo this is ugly
        if (iwidth && iwidth->empty()) iwidth.reset(); // \todo this is ugly
        if (ipos) mbox->id.add_attribute(the_names["box_pos"], *ipos);
        if (iwidth) mbox->id.add_attribute(the_names["box_width"], *iwidth);
    }
    box_end(the_stack.remove_last(), i);
}

void Parser::T_picture() {
    flush_buffer();
    the_stack.push1(the_names["picture"]);

    AttList &cur = last_att_list();
    the_stack.set_arg_mode();
    std::string A, B;
    Token       C = cur_tok;
    T_twodims(A, B, C);
    cur[the_names["height"]] = B;
    cur[the_names["width"]]  = A;
    skip_initial_space_and_back_input();
    if (cur_tok.is_open_paren()) {
        T_twodims(A, B, C);
        cur[the_names["ypos"]] = B;
        cur[the_names["xpos"]] = A;
    }
}

void Parser::T_fbox_dash_box() {
    Token T = cur_tok;
    flush_buffer();
    TokenList   a = read_arg();
    std::string A = token_list_to_att(a, T, false);
    std::string B, C;
    T_twodims(B, C, T);
    std::string oarg = sT_optarg_nopar();
    the_stack.push1(the_names["pic-dashbox"]);
    Xid cur_id = the_stack.get_top_id();
    if (!oarg.empty()) cur_id.add_attribute(the_names["box_pos"], std::string(oarg));
    cur_id.add_attribute(the_names["height"], C);
    cur_id.add_attribute(the_names["width"], B);
    cur_id.add_attribute(the_names["dashdim"], A);
    T_arg_local();
    the_stack.pop(the_names["pic-dashbox"]);
}

void Parser::T_fbox_rotate_box() {
    flush_buffer();
    std::string val = nT_arg_nopar();
    leave_v_mode();
    the_stack.push1(the_names["rotatebox"]);
    the_stack.get_top_id().add_attribute(the_names["rotate_angle"], val);
    T_arg_local();
    the_stack.pop(the_names["rotatebox"]);
}

// \fbox{\includegraphics{...}} is special.
// This implements \fbox, \framebox and \scalebox
void Parser::T_fbox(subtypes cc) {
    Token T = cur_tok;
    flush_buffer();
    if (cc == raisebox_code) {
        T_raisebox();
        return;
    }
    std::string                iscale;
    std::optional<std::string> ipos;
    std::optional<std::string> iwidth;

    if (cc == framebox_code) { // case of \framebox
        skip_initial_space_and_back_input();
        if (cur_tok.is_open_paren()) {
            T_makebox(true, T);
            return;
        }
        iwidth = get_opt_dim(T);
        auto x = get_ctb_opt();
        if (x) ipos = the_names[*x];
    }
    if (cc == scalebox_code) {
        flush_buffer();
        iscale = nT_arg_nopar();
        iwidth = nT_optarg_nopar();
    }
    leave_v_mode();
    the_stack.push1(the_names[cc == scalebox_code ? "scalebox" : "fbox"]);
    Xml *cur = the_stack.top_stack(); // will contain the argument.
    T_arg_local();
    the_stack.pop(the_names[cc == scalebox_code ? "scalebox" : "fbox"]);
    Xml *    aux = cur->single_non_empty();
    AttList &AL  = cur->id.get_att();
    if (cc == scalebox_code) {
        if ((aux != nullptr) && aux->has_name(the_names["figure"])) {
            aux->id.add_attribute(the_names["scale"], iscale, true);
            if (iwidth) aux->id.add_attribute(std::string("vscale"), *iwidth);
            cur->kill_name();
        } else {
            AL[the_names["box_scale"]] = iscale;
            if (iwidth) AL["vscale"] = *iwidth; // \toto perhaps the_names for consistency
        }
        return;
    }
    if (iwidth && iwidth->empty()) iwidth.reset(); // \todo That is ugly
    if (ipos && ipos->empty()) ipos.reset();       // \todo That is ugly
    if ((aux != nullptr) && aux->has_name(the_names["figure"])) {
        aux->id.add_attribute(the_names["framed"], the_names["true"]);
        cur->kill_name();
    } else {
        AL[the_names["fbox_rend"]] = the_names["boxed"];
        if (ipos) AL[the_names["box_pos"]] = *ipos;
        if (iwidth) AL[the_names["box_width"]] = *iwidth;
    }
}

// Returns <xref url='v'>val</xref>
void Parser::new_xref(Xml *val, std::string v, bool err) {
    the_stack.add_last(new Xml(the_names["xref"], val));
    the_stack.add_att_to_last(the_names["url"], std::string(v));
    if (err && (v.empty() || v[0] == '(')) parse_error("Invalid URL value");
}

// Translates \url
void Parser::T_url() {
    bool no_hack = remove_initial_star();
    leave_v_mode();
    auto      guard1 = SaveCatcode('~', other_catcode);
    auto      guard2 = SaveCatcode('&', active_catcode);
    auto      guard3 = SaveCatcode('#', active_catcode);
    auto      guard4 = InLoadHandler();
    auto      guard5 = InUrlHandler();
    TokenList X      = read_arg();
    if (!X.empty()) {
        Token T = X.front();
        token_from_list(T);
    }
    TokenList Y;
    bool      in_href = the_stack.is_frame2("hanl"); // \todo what is hanl?
    if (!in_href) {
        Y = X;
        Y.brace_me();
    }
    X.push_front(hash_table.urlfont_token);
    if (in_href) {
        if (!no_hack) X.url_hack();
        X.brace_me();
        T_translate(X);
    } else {
        std::string x = translate_list(Y)->convert_to_string();
        if (!no_hack) X.url_hack();
        X.brace_me();
        Xml *y = translate_list(X);
        new_xref(y, x, true);
    }
}

// Grabs the text of the URL. This does nothing special with ~.
// Argument is translated in a group.
auto Parser::T_hanl_text() -> Xml * {
    push_level(bt_local);
    Xml *val = xT_arg_nopar();
    pop_level(bt_local);
    return val;
}

// This fetches the URL.
auto Parser::T_hanl_url() -> Xml * {
    auto guard  = InUrlHandler();
    auto guard2 = InLoadHandler();
    Xml *B      = xT_arg_nopar();
    return B;
}

// Translates \href or \htmladdnormallink
// c is 0 for \htmladdnormallink, 2 for \href
void Parser::T_hanl(subtypes c) {
    leave_v_mode();
    the_stack.push(the_names["hanl"], nullptr);
    the_stack.hack_for_hanl();
    Xml *B{nullptr};
    Xml *val{nullptr};
    if (c == 2) {
        B   = T_hanl_url();
        val = T_hanl_text();
    } else {
        ignore_optarg();
        val = T_hanl_text();
        B   = T_hanl_url();
    }
    the_stack.pop(the_names["hanl"]);
    int e              = nb_errs;
    unexpected_seen_hi = false;
    std::string b      = B->convert_to_string();
    bool        failed = e != nb_errs;
    if (unexpected_seen_hi && failed)
        log_and_tty << "you should perhaps use \\Href{\\url{x}}{y}\n"
                    << "  instead of \\Href{y}{\\url{x}}\n";
    new_xref(val, b, !failed);
}

// This should work, whatever the mode...
// If env is true, we grab the content of the env.

auto Parser::special_tpa_arg(const std::string &name, const std::string &y, bool par, bool env, bool has_q) -> Xml * {
    if (y.empty()) {
        TokenList L = read_arg();
        back_input(hash_table.par_token);
        back_input(L);
        return nullptr;
    }
    mode m = the_stack.get_mode();
    the_stack.set_arg_mode();
    bool        has_atts = tpa_buffer.look_at_space(y);
    std::string Y;
    if (!has_atts)
        Y = y;
    else
        Y = tpa_buffer;
    if (par) the_stack.set_v_mode();
    the_stack.push(Y, new Xml(Y, nullptr));
    if (has_q) the_stack.mark_omit_cell();
    Xid cid = the_stack.get_top_id();
    if (has_atts) tpa_buffer.push_back_special_att(cid);
    if (par) ileave_v_mode();
    Buffer &B = tpa_buffer;
    if (!env) {
        B       = name + "@hook";
        cur_tok = hash_table.locate(B);
        if (!Hashtab::the_eqtb()[cur_tok.eqtb_loc()].val.is_undef()) {
            Token     T = cur_tok;
            TokenList L = read_arg();
            L.brace_me();
            L.push_front(T);
            L.brace_me();
            back_input(L);
        }
    }
    bool special_case = false;
    if (!env) scan_left_brace();
    push_tpa();
    B       = name + "@helper";
    cur_tok = hash_table.locate(B);
    if (!Hashtab::the_eqtb()[cur_tok.eqtb_loc()].val.is_undef()) {
        back_input(cur_tok);
        if (!env) special_case = true;
    }
    if (special_case) {
        cur_level++;
        if (tracing_stack()) {
            Logger::finish_seq();
            the_log << "+stack: level + " << cur_level << " (spec)\n";
        }
    }
    try {
        translate_all();
    } catch (EndOfData &) {};
    flush_buffer();
    if (special_case) {
        if (tracing_stack()) {
            Logger::finish_seq();
            the_log << "+stack: level - " << cur_level << " (spec)\n";
        }
        cur_level--;
    }
    if (par) the_stack.pop(the_names["cst_p"]);
    the_stack.pop(Y);
    the_stack.set_mode(m);
    return the_stack.remove_last();
}

auto Parser::tpa_exec(const std::string &cmd) -> Xml * {
    mode m = the_stack.get_mode();
    the_stack.set_arg_mode();
    auto Y = std::string(cmd);
    the_stack.push(Y, new Xml(std::string(), nullptr));
    tpa_buffer = cmd;
    finish_csname(tpa_buffer, cmd);
    get_token();
    TokenList L;
    L.push_back(cur_tok);
    T_translate(L);
    the_stack.pop(Y);
    the_stack.set_mode(m);
    return the_stack.remove_last();
}

// \@reevaluate\foo\bar\gee is \foo\gee\bar\gee
// \@reevaluate\foo\bar{gee} is \foo{gee}\bar{gee}
// \begin{X}\@reevaluate*{foo}{bar} etc \end{X}
//  is \begin{foo} etc \end{foo}\begin{bar} etc \end{bar}
void Parser::T_reevaluate() {
    bool      in_env = remove_initial_star();
    TokenList L1     = read_arg();
    TokenList L2     = read_arg();
    skip_initial_space();
    // check the easy case
    if (!in_env && !cur_cmd_chr.is_open_brace()) {
        back_input(cur_tok);
        back_input(L2);
        back_input(cur_tok);
        back_input(L1);
        return;
    }
    if (in_env) back_input(cur_tok);
    // Now the hard case
    Buffer &B = tpa_buffer;
    B.clear();
    if (!scan_for_eval(B, in_env)) parse_error("reevaluate has problems with end of input");
    if (in_env) {    // unread tokens should be \end{cur}
        get_token(); // the \end token
        std::string s = group_to_string();
        if (is_env_on_stack(s) == nullptr) {
            parse_error(err_tok, "cannot close environment ", s, "bad \\end");
            return;
        }
        cur_tok.kill();
        pop_level(bt_env); // closes current env
    }
    Tbuf.clear();
    L1.reevaluate0(in_env);
    L2.reevaluate0(in_env);
    if (tracing_commands()) {
        Logger::finish_seq();
        the_log << "{Reeval: " << Tbuf << "}\n";
    }
    push_input_stack("(reevaluate)", false, false);
    lines.push_front(Line(-1));
    lines.split_string(Tbuf, 0);
}

// Translates \uppercase, \MakeUpperCase, etc
void Parser::T_case_shift(subtypes c) {
    Token        T        = cur_tok;
    bool         to_upper = (c == 1 || c == 3 || c == 5 || c == 7);
    const size_t offset   = to_upper ? uc_code_offset : lc_code_offset;
    int          k        = to_upper ? +1 : -1;
    Token *      table    = to_upper ? uclc_list.data() : uclc_list.data() + 1;
    bool         ltx      = (c >= 2);
    bool         extended = (c == 4 || c == 5);
    bool         latex3   = (c >= 6);
    TokenList    L        = ltx ? read_arg() : scan_general_text();
    if (ltx && !latex3) {
        push_level(bt_brace);
        //    if(to_upper) {} // definir \i et \j
        if (extended) {
            M_let_fast(hash_table.nocase_e_token, hash_table.nocase_i_token, false);
            M_let_fast(hash_table.cite_e_token, hash_table.cite_i_token, false);
            M_let_fast(hash_table.ref_token, hash_table.ref_i_token, false);
            M_let_fast(hash_table.label_token, hash_table.label_i_token, false);
            M_let_fast(hash_table.ensuremath_token, hash_table.ensuremath_i_token, false);
        }
        for (int i = 0; i < 11; i++) {
            TokenList LL;
            LL.push_back(table[2 * i + k]);
            new_macro(LL, table[2 * i]);
        }
        read_toks_edef(L);
        pop_level(bt_brace);
    }
    if (tracing_commands()) {
        Logger::finish_seq();
        the_log << "{" << T << "(a)->" << L << "}\n";
    }
    auto      P = L.begin();
    auto      E = L.end();
    TokenList res;
    while (P != E) {
        Token a = *P;
        ++P;
        int bl = 0;
        if (a.is_OB_token())
            bl++;
        else if (a.is_CB_token())
            bl--;
        if (extended && bl == 0 && a == hash_table.nocase_e_token) {
            int b = 0;
            for (;;) {
                if (P == E) break;
                a = *P;
                ++P;
                res.push_back(a);
                if (a.is_OB_token())
                    b++;
                else if (a.is_CB_token())
                    b--;
                if (b <= 0) break;
            }
            continue;
        }
        if (extended && bl == 0 && a.is_math_shift()) {
            res.push_back(a);
            for (;;) {
                if (P == E) break;
                a = *P;
                ++P;
                res.push_back(a);
                if (a.is_math_shift()) break; // not very robust
            }
            continue;
        }
        if (a.char_or_active()) {
            auto b  = a.chr_val();
            auto cx = eqtb_int_table[b + offset].val;
            if (cx != 0) {
                res.push_back(Token(a.val - b + to_unsigned(cx)));
                continue;
            }
        }
        res.push_back(a);
    }
    if (tracing_commands()) {
        Logger::finish_seq();
        the_log << "{" << T << "->" << res << "}\n";
    }
    back_input(res);
}

// This parses the arguments A,B in \put(A,B){C}
void Parser::T_twoints(TokenList &A, TokenList &B) {
    Token match(other_t_offset, '(');
    get_token();
    if (cur_tok != match) bad_macro_prefix(cur_tok, match);
    A = read_until_nopar(hash_table.comma_token);
    B = read_until_nopar(Token(other_t_offset, ')'));
}

void Parser::back_input_pt(bool spec) {
    if (spec) {
        back_input(Token(letter_t_offset, 't'));
        back_input(Token(letter_t_offset, 'p'));
    } else
        back_input(hash_table.unitlength_token);
}

auto Parser::token_list_to_dim(TokenList &a, Token C, bool spec) -> ScaledInt {
    back_input_pt(spec);
    back_input(a);
    scan_dimen(false, C);
    return cur_val.get_dim_val();
}

// Same code, but calls dimen_attrib
auto Parser::token_list_to_att(TokenList &a, Token C, bool spec) -> std::string {
    back_input_pt(spec);
    back_input(a);
    scan_dimen(false, C);
    return dimen_attrib(cur_val.get_dim_val());
}

void Parser::T_twoints(std::string &A, std::string &B, Token C) {
    TokenList a;
    TokenList b;
    T_twoints(a, b);
    A = dimen_attrib(ScaledInt(scan_int(a, C) << 16));
    B = dimen_attrib(ScaledInt(scan_int(b, C) << 16));
}

void Parser::T_twodims(std::string &A, std::string &B, Token C) {
    TokenList a;
    TokenList b;
    T_twoints(a, b);
    A = token_list_to_att(a, C, false);
    B = token_list_to_att(b, C, false);
}

auto Parser::get_c_val(Token x) -> std::string {
    back_input(hash_table.CB_token);
    back_input(x);
    back_input(hash_table.OB_token);
    return nT_arg_nopar();
}

void Parser::T_bezier(subtypes c) {
    Token C = cur_tok;
    flush_buffer();
    std::string                a1, a2, b1, b2, c1, c2;
    std::optional<std::string> w;
    {
        TokenList L;
        if (c != 0)
            L = read_optarg().value_or(TokenList{});
        else
            L = read_arg();
        if (!L.empty()) w = token_list_to_att(L, C, true);
    }
    T_twodims(a1, a2, C);
    T_twodims(b1, b2, C);
    T_twodims(c1, c2, C);
    AttList &res = the_stack.add_newid0("bezier");
    if (w) res[the_names["repeat"]] = *w;
    res[the_names["c2"]] = c2;
    res[the_names["c1"]] = c1;
    res[the_names["b2"]] = b2;
    res[the_names["b1"]] = b1;
    res[the_names["a2"]] = a2;
    res[the_names["a1"]] = a1;
}

// put \line \vector \oval
void Parser::T_put(subtypes c) {
    Token C = cur_tok;
    flush_buffer();
    std::string A, B, D;
    std::string x0;
    switch (c) {
    case put_code: x0 = "put"; break;
    case line_code: x0 = "line"; break;
    case vector_code: x0 = "vector"; break;
    case oval_code: x0 = "oval"; break;
    case multiput_code: x0 = "multiput"; break;
    case scaleput_code: x0 = "scaleput"; break;
    case frame_code: x0 = "pic-frame"; break;
    default: x0 = "cst_empty";
    }
    bool star = remove_initial_star();
    if (c == frame_code) {
        the_stack.push1(the_names[x0]);
        the_stack.set_arg_mode();
        T_arg();
        the_stack.pop(the_names[x0]);
        return;
    }
    if (c == multiput_code && star) {
        T_multiput();
        remove_initial_space_and_back_input();
        return;
    }
    if (c == put_code || c == oval_code || c == multiput_code || c == scaleput_code)
        T_twodims(A, B, C);
    else
        T_twoints(A, B, C);
    if (c == oval_code) {
        auto     specs = nT_optarg_nopar();
        AttList &val   = the_stack.add_newid0(x0);
        if (specs) val[the_names["specs"]] = *specs;
        val[the_names["ypos"]] = B;
        val[the_names["xpos"]] = A;
        return;
    }
    if (c != put_code && c != multiput_code && c != scaleput_code) { // line vector
        TokenList L            = read_arg();
        D                      = token_list_to_att(L, C, false);
        AttList &AL            = the_stack.add_newid0(x0);
        AL[the_names["width"]] = D;
        AL[the_names["ydir"]]  = B;
        AL[the_names["xdir"]]  = A;
        return;
    }
    // Case of \put or \multiput \scaleput
    the_stack.push1(the_names[x0]);
    the_stack.set_arg_mode();
    Xid cur_id = the_stack.get_top_id();
    if (c == scaleput_code) {
        AttList &AL              = cur_id.get_att();
        AL[the_names["yscalex"]] = get_c_val(hash_table.yscalex_token);
        AL[the_names["xscaley"]] = get_c_val(hash_table.xscaley_token);
        AL[the_names["yscale"]]  = get_c_val(hash_table.yscale_token);
        AL[the_names["xscale"]]  = get_c_val(hash_table.xscale_token);
    }
    if (c == multiput_code) {
        std::string aa, bb;
        T_twodims(aa, bb, C);
        cur_id.add_attribute(the_names["dy"], bb);
        cur_id.add_attribute(the_names["dx"], aa);
        TokenList L = read_arg();
        D           = token_list_to_att(L, C, true); // integer....
        cur_id.add_attribute(the_names["repeat"], D);
    }
    cur_id.add_attribute(the_names["ypos"], B);
    cur_id.add_attribute(the_names["xpos"], A);
    T_arg();
    the_stack.pop(the_names[x0]);
    if (c == put_code || c == multiput_code) remove_initial_space_and_back_input();
}

void Parser::T_linethickness(subtypes c) {
    flush_buffer();
    std::string C = "linethickness";
    if (c == thicklines_code) C = "thicklines";
    if (c == thinlines_code) C = "thinlines";
    AttList &res = the_stack.add_newid0(C);
    if (c == linethickness_code) res[the_names["size"]] = nT_arg_nopar();
}

void Parser::T_curves(subtypes c) {
    Token C = cur_tok;
    flush_buffer();
    std::string x0 = "cst_empty";
    switch (c) {
    case arc_code: x0 = "arc"; break;
    case bigcircle_code: x0 = "bigcircle"; break;
    case closecurve_code: x0 = "closecurve"; break;
    case curve_code: x0 = "curve"; break;
    case tagcurve_code: x0 = "tagcurve"; break;
    default:;
    }
    the_stack.push1(the_names[x0]);
    the_stack.set_arg_mode();
    Xid  cur_id = the_stack.get_top_id();
    auto specs  = nT_optarg_nopar();
    if (specs && !specs->empty()) cur_id.add_attribute(the_names["curve_nbpts"], *specs);
    TokenList emptyl;
    cur_id.add_attribute(the_names["unit_length"], token_list_to_att(emptyl, C, false));
    if (c == arc_code) {
        std::string aa, bb;
        T_twodims(aa, bb, C);
        cur_id.add_attribute(the_names["ypos"], bb);
        cur_id.add_attribute(the_names["xpos"], aa);
        specs = nT_arg_nopar();
        cur_id.add_attribute(the_names["angle"], *specs);
    } else if (c == bigcircle_code) {
        cur_id.add_attribute(the_names["size"], nT_arg_nopar());
    } else {
        Token match(other_t_offset, '(');
        skip_initial_space();
        if (cur_tok != match) bad_macro_prefix(cur_tok, match);
        TokenList L = read_until_nopar(Token(other_t_offset, ')'));
        T_translate(L);
    }
    the_stack.pop(the_names[x0]);
}

void Parser::T_multiput() {
    Token     C = cur_tok;
    TokenList xpos, ypos;
    T_twoints(xpos, ypos);
    ScaledInt X, Y, Dx, Dy;
    X = token_list_to_dim(xpos, C, false);
    Y = token_list_to_dim(ypos, C, false);
    TokenList dx, dy;
    T_twoints(dx, dy);
    Dx          = token_list_to_dim(dx, C, false);
    Dy          = token_list_to_dim(dy, C, false);
    TokenList w = read_arg();
    back_input(w);
    auto      r = scan_int(C);
    TokenList L = read_arg();
    L.brace_me();
    while (r > 0) {
        TokenList Lc = L;
        back_input(Lc);
        the_stack.push1(the_names["put"]);
        the_stack.set_arg_mode();
        AttList &AL           = last_att_list();
        AL[the_names["ypos"]] = dimen_attrib(Y);
        AL[the_names["xpos"]] = dimen_attrib(X);
        T_arg();
        the_stack.pop(the_names["put"]);
        the_stack.add_nl();
        r--;
        X += Dx;
        Y += Dy;
    }
}

void Parser::T_dashline(subtypes c) {
    Token T = cur_tok;
    flush_buffer();
    std::string x0 = "cst_empty";
    if (c == dashline_code) x0 = "dashline";
    if (c == drawline_code) x0 = "drawline";
    if (c == dottedline_code) x0 = "dottedline";
    if (c == circle_code) { // circle
        x0                    = "circle";
        bool        has_star  = remove_initial_star();
        TokenList   L         = read_arg();
        std::string aa        = token_list_to_att(L, T, false);
        AttList &   AL        = the_stack.add_newid0(x0);
        AL[the_names["size"]] = aa;
        if (has_star) AL[the_names["full"]] = the_names["true"];
        return;
    }
    auto                       A = nT_optarg_nopar();
    std::optional<std::string> B;
    if (c == dashline_code || c == dottedline_code) B = nT_arg_nopar();
    auto C = nT_optarg_nopar();
    the_stack.push1(the_names[x0]);
    the_stack.set_arg_mode();
    AttList &AL = last_att_list();
    if (A) AL[the_names["arg1"]] = *A;
    if (B) AL[the_names["arg2"]] = *B;
    if (C) AL[the_names["arg3"]] = *C;
    for (;;) {
        std::string xpos, ypos;
        skip_initial_space();
        if (cur_tok.is_valid()) back_input();
        if (!cur_tok.is_open_paren()) break;
        T_twodims(xpos, ypos, T);
        AttList &res           = the_stack.add_newid0("point");
        res[the_names["ypos"]] = ypos;
        res[the_names["xpos"]] = xpos;
    }
    the_stack.pop(the_names[x0]);
}

// This is called before we output a character
void Parser::LC() {
    if (the_stack.in_v_mode()) ileave_v_mode();
    if (the_stack.in_no_mode() || the_stack.in_bib_mode()) {
        signal_error("Text found in a mode where no text is allowed");
        if (the_stack.first_frame() == the_names["gloss_type"]) {
            log_and_tty << "Maybe \\glo is missing?\nHope for the best\n";
            the_stack.set_arg_mode();
            return;
        }
        if (the_stack.in_bib_mode()) return;
        the_stack.set_mode(mode_argument);
        the_log << "Bad mode:XML stack is: ";
        the_stack.trace_stack();
        the_log << "\n";
    }
}

void Parser::T_error() {
    if (cur_cmd_chr.chr == 1) {
        parse_error(cur_tok, "Can be used only in preamble: ", cur_tok, "", "noprerr");
        return;
    }
    flush_buffer();
    std::string b = sT_arg_nopar(); // msg
    err_buf       = "Error: " + b;
    signal_error(err_tok, b);
}

// scans an element id, in brackets, default is cur_id
auto Parser::read_elt_id(Token T) -> size_t {
    auto cur   = the_stack.cur_xid().value;
    auto upper = the_stack.get_xid();
    auto n     = scan_special_int_d(T, to_signed(cur));
    if (n > 0 && n <= to_signed(upper)) return to_unsigned(n);
    err_buf = fmt::format("Bad xml id replaced by 0: {}", n);
    signal_error(err_tok, "number too big");
    return 0;
}

// Adds an attribute pair to some element
void Parser::T_xmladdatt(subtypes c) {
    Token T     = cur_tok;
    bool  force = remove_initial_star();
    flush_buffer();
    size_t n     = 0;
    auto   guard = InLoadHandler();

    if (c == addatt_to_cur_code)
        n = the_stack.cur_xid().value;
    else if (c == addatt_to_last_code)
        n = the_stack.get_xid();
    else if (c == addatt_to_doc_code)
        n = 1;
    else if (c == addatt_to_index_code)
        n = get_index_value();
    else
        n = read_elt_id(T);
    cur_tok         = T;
    std::string key = nT_arg_nopar();
    cur_tok         = T;
    std::string val = nT_arg_nopar();
    if (key.empty()) {
        if (!force) return;
        Xml *e = the_stack.elt_from_id(n);
        if (e == nullptr) return;
        e->name = val;
        return;
    }
    if (n != 0) Xid(n).add_attribute(key, val, force);
}

// Returns the value of an attribute or element
auto Parser::get_attval() -> std::string {
    Token T = cur_tok;
    flush_buffer();
    auto        n   = read_elt_id(T);
    std::string key = nT_arg_nopar();
    if (key.empty()) {
        Xml *e = the_stack.elt_from_id(n);
        if (e == nullptr) return "";
        return e->name;
    }
    return Xid(n).has_attribute(key);
}

void Parser::T_define_verbatim_env() {
    std::string a  = group_to_string();
    std::string b  = group_to_string();
    Token       t1 = find_env_token(a, true);
    Token       t2 = find_env_token(b, true);
    M_let_fast(t1, t2, true); // global assignments OK ?
    Token xt1 = find_env_token(a, false);
    Token xt2 = find_env_token(b, false);
    M_let_fast(xt1, xt2, true);
    TokenList L = read_arg();
    Buffer &  B = Tbuf;
    B           = a + "@hook";
    new_macro(L, hash_table.locate(b));
}

// Implements some commands (\ignorespaces, \mark, \penalty, \@@end,
// \message and \errmessage
void Parser::T_specimp(subtypes c) {
    switch (c) {
    case ignorespaces_code: remove_initial_space_and_back_input(); return;
    case mark_code:
    case marks_code: T_mark(subtypes(c)); return;
    case penalty_code: scan_int(cur_tok); return;
    case abort_code: close_all(); exit(0);
    case sleep_code: txsleep(static_cast<unsigned>(scan_int(cur_tok))); return;
    case prompt_code: {
        auto S = string_to_write(write18_slot + 1);
        readline_newprompt(S);
        return;
    }
    case atatend_code:
        log_and_tty << "Terminated with " << cur_tok << " at line  " << get_cur_line() << "\n";
        close_all();
        return;
    case message_code: std::cout << string_to_write(0); return;
    case errmessage_code:
        err_buf = string_to_write(write18_slot + 1);
        signal_error();
        return;
    case discretionary_code:
        scan_left_brace_and_back_input();
        ignore_arg();
        scan_left_brace_and_back_input();
        ignore_arg();
        scan_left_brace_and_back_input();
        return;
    case allowbreak_code:
        flush_buffer();
        the_stack.add_newid0("allowbreak");
        return;
    default:;
    }
}

// Commands that do nothing, just print a message.
void Parser::T_unimp(subtypes c) {
    parse_error(cur_tok, "Unimplemented command ", cur_tok, "", "unimplemented");

    switch (c) {
    case accent_code: extended_chars(scan_27bit_int()); return;
    case delimiter_code:
        scan_int(cur_tok); // no overflow check
        return;
    case insert_code:
        scan_reg_num();
        scan_left_brace_and_back_input();
        return;
    case vadjust_code: scan_left_brace_and_back_input(); return;
    case mathaccent_code:
    case mathchar_code: scan_fifteen_bit_int(); return;
    case radical_code: scan_27bit_int(); return;
    case vcenter_code: begin_box(makebox_location, vbox_code); return;
    default: return;
    }
}

void Parser::need_bib_mode() {
    if (the_stack.in_bib_mode()) return;
    Tbuf = fmt::format("Command {} should occur in bibliographic mode only", err_tok);
    parse_error(err_tok, Tbuf);
}

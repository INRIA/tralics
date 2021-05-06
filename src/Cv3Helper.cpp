#include "tralics/Cv3Helper.h"
#include "tralics/MainClass.h"
#include "tralics/MathDataP.h"
#include "tralics/Parser.h"
#include "tralics/globals.h"

void Cv3Helper::reinit() {
    state       = 2;
    index       = nullptr;
    exponent    = nullptr;
    p           = nullptr;
    ploc        = -1;
    ptype       = mt_flag_small;
    prefix_type = mt_flag_zero;
    special     = 0;
}

// Case where current token is \nonscript. If the mode is D or T, the token
// is removed in the first pass. Otherwise we remove it in the second pass
// together with a space that follows
void Cv3Helper::non_script() {
    pop_front();
    if (!empty()) {
        if (front().cmd == math_xml_cmd && front().get_xmltype() == mt_flag_space) pop_front();
    }
}

void Cv3Helper::find_kernel() {
    if (empty()) {
        state = 3;
        return;
    }
    symcodes C = front().cmd;
    if (C == hat_catcode || C == underscore_catcode) {
        p     = new Xml(the_names["mrow"], nullptr); // Insert <mrow/> before
        state = 1;
        return;
    }
    if (C == nonscript_cmd) {
        non_script();
        return;
    }
    if (C == math_comp_cmd) {
        subtypes q = front().chr;
        switch (q) {
        case between_noad: prefix_type = mt_flag_small_m; break;
        case ord_noad: prefix_type = mt_flag_small; break;
        case op_noad: prefix_type = mt_flag_opD; break;
        case bin_noad: prefix_type = mt_flag_bin; break;
        case rel_noad: prefix_type = mt_flag_rel; break;
        case open_noad: prefix_type = mt_flag_small_l; break;
        case close_noad: prefix_type = mt_flag_small_r; break;
        default: prefix_type = mt_flag_small; break;
        }
        pop_front();
        return;
    }
    if (C != math_xml_cmd) {
        the_parser.signal_error("unexpected obj");
        pop_front();
        return;
    }
    ploc  = static_cast<long>(front().chr);
    ptype = front().get_xmltype();
    if (prefix_type != 0) ptype = prefix_type;
    if (ptype == mt_flag_opD) special = 1;
    if (ptype == mt_flag_opN) special = 2;
    p = front().get_xml_val();
    pop_front();
    state = 1;
}

void Cv3Helper::find_index(math_style cms) {
    if (empty()) {
        state = 2;
        return;
    }
    symcodes C = front().cmd;
    if (C == limits_cmd) {
        size_t k = front().chr; // displaylimits, nolimits, limits
        pop_front();
        if ((index != nullptr) || (exponent != nullptr)) return; // allow x_2\limits^3
        if (special == 0) return;
        if (k == 0) k = cms == ms_D ? 2 : 1;
        special = special & 7; // clear flags
        if (k == 2)
            special += 8;
        else
            special += 16;
        return;
    }
    if (C == nonscript_cmd) {
        non_script();
        return;
    }
    if (C == underscore_catcode && (index == nullptr)) {
        pop_front();
        if (empty()) {
            the_parser.parse_error("Math formula should not finish with _");
        } else if (front().cmd == underscore_catcode || front().cmd == hat_catcode)
            the_parser.parse_error("Two consecutive _ or ^ characters");
        else {
            index = front().remove_prefix();
            pop_front();
        }
        return;
    }
    if (C == hat_catcode && (exponent == nullptr)) {
        pop_front();
        if (empty()) {
            the_parser.parse_error("Math formula should not finish with ^");
        } else if (front().cmd == underscore_catcode || front().cmd == hat_catcode)
            the_parser.parse_error("Two consecutive _ or ^ characters");
        else {
            exponent = front().remove_prefix();
            pop_front();
        }
        return;
    }
    if (C == underscore_catcode) {
        the_parser.parse_error("double subscript");
        pop_front();
        state = 2;
        return;
    }
    if (C == hat_catcode) {
        the_parser.parse_error("double superscript");
        pop_front();
        state = 2;
        return;
    }
    state = 2;
}

// x_1 and \sum_1 produce <msub> or <munder>
// This is a bit complicated
// Special is A+B, A=8(limits) or 16(nolimits)
// B=0 (not a big op), B=1 (\sum, \lim, \mathop), B=2(\int, \sin)
auto Cv3Helper::find_operator(math_style cms) -> std::string {
    int what = 0;
    if (index != nullptr) what++;
    if (exponent != nullptr) what += 2;
    std::string bl = "msub";
    if (what == 3)
        bl = "msubsup";
    else if (what == 2)
        bl = "msup";

    if (special > 16)
        special = 0; //  explicit \nolimits
    else if (special == 2)
        special = 0; // implicit \nolimits
    else if (special == 1 && cms != ms_D)
        special = 0; // implicit \displaylimits

    if (special == 0) return bl;
    if (special > 8) { // \lim\limits_1 : define movablelimits='false'
        Xml *q = p->spec_copy();
        if (q != nullptr) {
            p = q;
            p->add_att(the_names["movablelimits"], the_names["false"]);
        }
    }
    if (what == 3)
        bl = "munderover";
    else if (what == 2)
        bl = "mover";
    else
        bl = "munder";
    return bl;
}

void Cv3Helper::add_kernel(math_style cms) {
    if ((index == nullptr) && (exponent == nullptr)) {
        res.push_back(p, ploc, ptype);
        return;
    }
    auto bl = find_operator(cms);
    if (the_main.prime_hack && exponent == math_data.get_mc_table(27) && bl == "msup") {
        bl       = "mrow";
        exponent = math_ns::get_builtin(varprime_code);
    }
    Xml *tmp = new Xml(the_names[bl], nullptr);
    // case a_b_c. If we do nothing, the mathml interpreter will barf
    if ((p != nullptr) && p->is_element() && (p->has_name_of("msup") || p->has_name_of("msub") || p->has_name_of("msubsup")))
        p = new Xml(the_names["mrow"], p);
    if (ptype == mt_flag_small_l || ptype == mt_flag_small_r || ptype == mt_flag_small_m) {
        res.push_back(p, ploc, ptype);
        Xml *q = p;
        res.push_back(q, -1, mt_flag_dummy);
    }

    // case {\sum}_1
    tmp->add_tmp(gsl::not_null{p});
    tmp->push_back_unless_nullptr(new Xml(std::string(" ")));
    if (index != nullptr) {
        tmp->add_tmp(gsl::not_null{index});
        tmp->push_back_unless_nullptr(new Xml(std::string(" ")));
    }
    if (exponent != nullptr) {
        tmp->add_tmp(gsl::not_null{exponent});
        tmp->push_back_unless_nullptr(new Xml(std::string(" ")));
    }
    p = tmp;
    res.push_back(p, -1, mt_flag_big);
}

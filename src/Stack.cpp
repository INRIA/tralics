#include "tralics/Stack.h"
#include "tralics/Logger.h"
#include "tralics/MainClass.h"
#include "tralics/NameMapper.h"
#include "tralics/Parser.h"
#include "tralics/globals.h"
#include "tralics/util.h"

namespace {
    // For debug. Returns symbolic name of mode
    auto mode_to_string(mode x) -> String {
        switch (x) {
        case mode_argument: return "_t";
        case mode_h: return "_h";
        case mode_v: return "_v";
        case mode_none: return "_n";
        case mode_bib: return "_b";
        case mode_array: return "_a";
        case mode_math: return "_m";
        default: return "_?";
        }
    }

    auto get_cur_label() -> std::string { return the_parser.eqtb_string_table[0].val; }
} // namespace

// Dumps a stack slot
void StackSlot::fulldump(size_t i) const {
    the_log << "level " << i << " entered at line " << line << ", type " << (frame == " " ? "()" : std::string(frame)) << ", mode"
            << mode_to_string(md) << ":\n";
    if (obj != nullptr) the_log << obj << "\n";
}

// This prints a simplified version of the stack.
void StackSlot::dump() const { the_log << " " << (frame == " " ? "()" : std::string(frame)) << mode_to_string(md); }

void Stack::implement_cit(const std::string &b1, const std::string &b2, const std::string &a, const std::string &c) {
    add_att_to_last(the_names["userid"], std::string(b1));
    add_att_to_last(the_names["id"], b2);
    add_att_to_last(the_names["key"], std::string(a));
    add_att_to_last(the_names["from"], std::string(c)); // \todo c is always "year"
}

// This starts or ends a cell, or does both
void Stack::push_pop_cell(int dir) {
    if (dir != push_only) {
        pop(the_names["cell"]);
        add_nl();
    }
    if (dir != pop_only) { push1(the_names["cell"]); }
}

// If the previous fails, we add a row of empty cells,
// This adds a-1 empty cells, then b cells with a bottom_border
void Stack::add_border(long a, long b) {
    push1(the_names["row"]);
    push_pop_cell(push_only);
    if (a != 1) {
        Xid(get_xid()).add_span(a - 1);
        push_pop_cell(push_and_pop);
    }
    Xid(get_xid()).add_span(b);
    Xid(get_xid()).add_bottom_rule();
    push_pop_cell(pop_only);
    pop(the_names["row"]);
}

// This implements \hline, its adds a top attribute to a row
void Stack::T_hline() {
    Xid cid, rid, tid;
    find_cid_rid_tid(cid, rid, tid);
    if (rid.has_attribute(the_names["cell_topborder"]).empty()) rid.add_top_rule();
}

// Increases xid, makes sure that the attribute table is big enough
auto Stack::next_xid(Xml *elt) -> Xid {
    attributes.emplace_back();
    enames.push_back(elt);
    return enames.size() - 1;
}

Stack::Stack() {
    next_xid(nullptr); // 0
    next_xid(nullptr); // 1 <document>
    next_xid(nullptr); // 2 dummy, see Stack::temporary
    next_xid(nullptr); // 3 is for the biblio, see after_main_text
    next_xid(nullptr); // 4 is <tableofcontents/>
    next_xid(nullptr); // 5 is for theindex (see txtrees)
    next_xid(nullptr); // 6 is for the glossary (see txtrees)
}

// Returns the element in the table with id n
// This is at position n in enames
auto Stack::fetch_by_id(size_t n) -> Xml * {
    if (enames.size() <= n) return nullptr;
    return enames[n];
}

// returns a parent of x
auto Stack::find_parent(Xml *x) -> Xml * {
    if (x == nullptr) return nullptr;
    auto k = enames.size();
    // debug: print all parents
    // std::cout<< "Search parent of " << x << "\n";
    // for(int i= 0; i< k; i++) {
    //   if(enames[i] && enames[i]-> is_child (x))
    //    std::cout<< "fount at " << enames[i] << "\n";
    // }
    for (size_t i = xid_boot + 1; i < k; i++) {
        if (enames[i] == nullptr) continue;
        if (enames[i]->is_child(x)) return enames[i];
    }
    return nullptr;
}

// Add A=B as attribute list to last xid.
void Stack::add_att_to_last(const std::string &A, const std::string &B) { get_att_list(get_xid())[A] = B; }

// Add A=B as attribute list to top stack
void Stack::add_att_to_cur(const std::string &A, const std::string &B) { cur_xid().add_attribute(A, B); }

// Add A=B as attribute list to last xid
// (if force is true, ignores old value otherwise new value).
void Stack::add_att_to_last(const std::string &A, const std::string &B, bool force) {
    auto &L = get_att_list(get_xid());
    if (force)
        L[A] = B;
    else
        L.emplace(A, B);
}

// Add A=B as attribute list to top stack
// (if force is true, ignores old value otherwise new value).
void Stack::add_att_to_cur(const std::string &A, const std::string &B, bool force) {
    auto &L = cur_xid().get_att();
    if (force)
        L[A] = B;
    else
        L.emplace(A, B);
}

void Stack::hack_for_hanl() {
    auto ptr = size() - 1;
    if (ptr > 0) { at(ptr).obj = at(ptr - 1).obj; }
    // else ?
}

// Creates an empty element named x, and adds it to the stack.
// returns a reference to the attribute list of the object.
auto Stack::add_newid0(const std::string &x) -> AttList & {
    top_stack()->push_back_unless_nullptr(new Xml(the_names[x], nullptr));
    return Xid(get_xid()).get_att();
}

// Adds x to the tail of the current list.
void Stack::add_last(Xml *x) { top_stack()->push_back_unless_nullptr(x); }

// Adds B to the tail of the current list.
void Stack::add_last_string(const Buffer &B) { top_stack()->add_last_string(B); }

// Implements unhbox, etc.
void Stack::unbox(Xml *x) { top_stack()->unbox(x); }

// Removes a trailing space on the current tree.
void Stack::remove_last_space() { top_stack()->remove_last_space(); }

// This adds a newline to the XML.
void Stack::add_nl() { top_stack()->add_nl(); }

// Returns the slot of the first non-empty frame
// It is assumed that the first frame is "document", hence never empty.
auto Stack::first_non_empty() const -> const StackSlot & {
    auto k = size() - 1;
    while (at(k).frame == " ") k--;
    return at(k);
}

// Returns the name of the first non-empty frame.
auto Stack::first_frame() const -> std::string {
    const StackSlot &X = first_non_empty();
    return X.frame;
}

// Returns the first p on the stack.
// Too bad that the frame AND the element have to be called p.
auto Stack::get_cur_par() const -> Xml * {
    const StackSlot &X     = first_non_empty();
    std::string      pname = the_names["cst_p"];
    if (X.frame == pname && X.obj->is_element() && X.obj->has_name(pname)) return X.obj;
    return nullptr;
}

// In case the current element happens to be a P,
// and has no rend attribute, then add one to it (can be center, can also
// be flush left, etc.)
void Stack::add_center_to_p() const {
    Xml *x = get_cur_par();
    if (x == nullptr) return;
    x->id.get_att().emplace(the_names["rend"], the_names.center(cur_centering()));
}

auto Stack::is_frame(const std::string &s) const -> bool { return first_frame() == the_names[s]; }

// Returns true if frame is s, or argument, and next frame is s.
// ignores a font change
auto Stack::is_frame2(const std::string &S) const -> bool {
    std::string s = the_names[S];
    auto        k = size() - 1;
    while (at(k).frame == " ") k--;
    if (at(k).frame == s) return true;
    if (at(k).frame == the_names["fonts"]) {
        if (k < 1) return false;
        k--;
    }
    if (at(k).frame == s) return true;
    if (at(k).frame != the_names["argument"]) return false;
    if (k < 1) return false;
    k--;
    while (at(k).frame == " ") k--;
    return at(k).frame == s;
}

// Returns true inside a float (table or figure)
auto Stack::is_float() -> bool {
    auto k = size();
    for (size_t i = 0; i < k; i++) {
        if (at(i).frame == " ") continue;
        if (at(i).frame == the_names["figure_env"]) return true;
        if (at(i).frame == the_names["table_env"]) return true;
    }
    return false;
}

// Prints something in trace mode when we push a non-empty frame.
void Stack::push_trace() {
    if (tracing_stack()) {
        auto        ptr = size() - 1;
        std::string fr  = at(ptr).frame;
        if (fr != " ") {
            Logger::finish_seq();
            the_log << "{Push " << fr << " " << ptr << "}\n";
        }
    }
}

// Internal code of push
void Stack::ipush(std::string fr, Xml *V) { push_back(StackSlot(V, the_parser.get_cur_line(), std::move(fr), cur_mode, cur_lid)); }

// Pushes code with frame.
void Stack::push(std::string fr, Xml *V) {
    top_stack()->push_back_unless_nullptr(V);
    ipush(std::move(fr), V);
    push_trace();
}

// Pushes a new empty object, for \hbox, etc
auto Stack::push_hbox(std::string name) -> Xml * {
    Xml *code = new Xml(std::move(name), nullptr);
    ipush(the_names["hbox"], code);
    push_trace();
    return code;
}

// Creates a temporary, sets mode to argument mode.
// The number of the element is 2. No other element has 2 as number
// (see Stack::Stack).
auto Stack::temporary() -> Xml * {
    Xml *res = new Xml("temporary", Xid(2));
    ipush(the_names["argument"], res);
    cur_mode = mode_argument;
    push_trace();
    return res;
}

// We assume that document is numbered 1. This simplifies  the mechanism
// for adding attributes to the document.
// Called after all math elements are created
void Stack::init_all(const std::string &a) {
    cur_mode = mode_v;
    cur_lid  = std::string("uid1");
    Xml *V   = new Xml(std::string(a), nullptr);
    V->push_back_unless_nullptr(nullptr); // Make a hole for the color pool
    V->id = 1;
    ipush(the_names["document"], V);
    newline_xml = new Xml("\n");
}

// Tracing of stack when popping
void Stack::trace_pop(bool sw) {
    if (tracing_stack()) {
        Logger::finish_seq();
        the_log << "{Pop ";
        if (sw) the_log << "(module) ";
        the_log << int(size() - 1) << ":";
        trace_stack();
        the_log << "}\n";
    }
}

// This prints the whole stack.
void Stack::dump() {
    auto l = size();
    for (size_t i = 0; i < l; i++) at(i).fulldump(i);
}

// Like Stack::dump, less verbose.
void Stack::trace_stack() {
    auto l = size();
    for (size_t i = 0; i < l; i++) at(i).dump();
}

// This pops an element, top-stack should be a.
void Stack::pop(const std::string &a) {
    trace_pop(false);
    while (back().frame == " ") { // ignore dummy frames
        the_parser.cur_font.not_on_stack();
        pop_back();
    }
    if (a != back().frame) {
        err_buf = fmt::format("Error in pop; stack holds {}; trying to pop ", encode(back().frame), encode(a));
        the_parser.signal_error();
        return;
    }
    cur_mode = back().md;
    cur_lid  = back().uid;
    pop_back();
    if (empty()) {
        err_buf = "Error in pop; stack empty; trying to pop " + encode(a);
        the_parser.signal_error();
        std::string S = the_names["document"];
        ipush(S, new Xml(S, nullptr)); // stack should never be empty
    }
}

// Pushes a <p>. The argument a is (an attribute value for) a vertical space
// if indent =false, the paragraph is not indented.
// if a<0 ? beurks...
auto Stack::push_par(size_t k) -> Xid {
    Xml *res = new Xml(the_names["cst_p"], nullptr);
    Xid  id  = res->id;
    push(the_names["cst_p"], res);
    cur_mode = mode_h; // we are in horizontal mode now
    check_font();
    if (k > 0) id.add_attribute(the_names["rend"], the_names.center(k));
    return id;
}

// Fonts without argument like \it, (still ok ?)
void Stack::fonts0(const std::string &x) {
    Xml *res                                 = fonts1(x);
    res->id.get_att()[the_names["'hi_flag"]] = "";
    push(std::string(" "), res);
}

// Adds font info when required
void Stack::check_font() {
    while (back().frame == " ") pop_back();
    bool        w = the_main.pack_font_elt;
    std::string s;
    if (w) {
        Buffer aux;
        bool   nonempty = false;
        s               = the_parser.cur_font.size_change();
        if (s != "cst_empty") { // \todo empty string or something
            aux += encode(the_names[s]);
            nonempty = true;
        }
        s = the_parser.cur_font.shape_change();
        if (s != "cst_empty") {
            if (nonempty) aux.append(",");
            aux += encode(the_names[s]);
            nonempty = true;
        }
        s = the_parser.cur_font.family_change();
        if (s != "cst_empty") {
            if (nonempty) aux.append(",");
            aux += encode(the_names[s]);
            nonempty = true;
        }
        s = the_parser.cur_font.series_change();
        if (s != "cst_empty") {
            if (nonempty) aux.append(",");
            aux += encode(the_names[s]);
            nonempty = true;
        }
        if (nonempty) {
            auto     a           = std::string(aux);
            Xml *    res         = new Xml(the_names["hi"], nullptr);
            AttList &W           = res->id.get_att();
            W[the_names["rend"]] = a;
            push(std::string(" "), res);
        }
    } else {
        s = the_parser.cur_font.size_change();
        if (s != "cst_empty") fonts0(s);
        s = the_parser.cur_font.shape_change();
        if (s != "cst_empty") fonts0(s);
        s = the_parser.cur_font.family_change();
        if (s != "cst_empty") fonts0(s);
        s = the_parser.cur_font.series_change();
        if (s != "cst_empty") fonts0(s);
    }
    if (auto c = the_parser.cur_font.color; !c.empty()) {
        Xml *    res          = new Xml(the_names["hi"], nullptr);
        AttList &W            = res->id.get_att();
        W[the_names["color"]] = c;
        push(std::string(" "), res);
    }
    the_parser.cur_font.is_on_stack();
}

// Push a new XML element
void Stack::push1(std::string name, std::string x) { push(std::move(name), new Xml(std::move(x), nullptr)); }

// Push a new XML element
void Stack::push1(const std::string &x) { push(x, new Xml(x, nullptr)); }

// Code done when a module ends. pop until stack (nearly) empty
void Stack::end_module() {
    trace_pop(true);
    for (;;) {
        if (empty()) {
            spdlog::critical("Unexpected empty stack");
            abort();
        }
        if (back().frame == the_names["document"]) return;
        if (back().frame == the_names["module"]) {
            pop(the_names["module"]);
            return;
        }
        pop_back();
    }
}

// Conditional pop
void Stack::pop_if_frame(const std::string &x) {
    if (first_frame() == x) pop(x);
}

// Code done when a section ends. Pops all that is required
void Stack::para_aux(int x) {
    if (x <= 6) pop_if_frame(the_names["cst_div6"]);
    if (x <= 5) pop_if_frame(the_names["cst_div5"]);
    if (x <= 4) pop_if_frame(the_names["cst_div4"]);
    if (x <= 3) pop_if_frame(the_names["cst_div3"]);
    if (x <= 2) pop_if_frame(the_names["cst_div2"]);
    if (x <= 1) pop_if_frame(the_names["cst_div1"]);
    if (x <= 0) pop_if_frame(the_names["cst_div0"]);
}

// This allocates a slot for a new table.
auto Stack::new_array_info(Xid id) -> ArrayInfo & {
    AI.emplace_back(id);
    return AI.back();
}

// This finds the table info given an ID
auto Stack::find_cell_props(Xid id) -> ArrayInfo * {
    if (AI.empty()) return nullptr;
    if (!(AI.back().id == id)) return nullptr;
    return &AI.back();
}

// This finds the cell row and table ID currently on the  stack.
void Stack::find_cid_rid_tid(Xid &cid, Xid &rid, Xid &tid) {
    auto k = size() - 1;
    while (at(k).frame == " ") k--;
    if (at(k).frame == the_names["cell"]) {
        cid = at(k).obj->id;
        k--;
    }
    while (at(k).frame == " ") k--;
    if (at(k).frame == the_names["row"]) {
        rid = at(k).obj->id;
        k--;
    }
    while (at(k).frame == " ") k--;
    if (at(k).frame == the_names["tabular"] || at(k).frame == the_names["tabular*"]) {
        tid = at(k).obj->id;
        k--;
    }
}

auto Stack::find_ctrid(subtypes m) -> size_t {
    for (auto k = size(); k-- >= 1;) {
        Xml *obj = at(k).obj;
        if (obj == nullptr) continue;
        std::string frame = at(k).frame;
        if (frame == " ") continue;
        if (m == xmlcurrow_code && frame == the_names["row"]) return obj->id.value;
        if (m == xmlcurcell_code && frame == the_names["cell"]) return obj->id.value;
        if (m == xmlcurarray_code && (frame == the_names["tabular"] || frame == the_names["tabular*"])) return obj->id.value;
    }
    return 0;
}

// Computes the cell-id in cid, and returns the table associated.
auto Stack::get_my_table(Xid &cid) -> ArrayInfo * {
    Xid rid, tid;
    find_cid_rid_tid(cid, rid, tid);
    ArrayInfo *A = find_cell_props(tid);
    if (A == nullptr) {
        spdlog::critical("Fatal: {} find_cell_prop failure", tid.value);
        abort();
    }
    return A;
}

// This deletes a table-information slot from the stack.
void Stack::delete_table_atts() {
    if (AI.empty()) return;
    AI.pop_back();
}

auto Stack::get_u_or_v(bool u_or_v) -> TokenList {
    Xid        unused;
    ArrayInfo *A       = get_my_table(unused);
    auto       cell_no = A->cell_no;
    return A->get_u_or_v(u_or_v, cell_no);
}

// Adds positions attributes to the current cell, given the current
// table info.
void Stack::finish_cell(int w) {
    Xid        cid;
    ArrayInfo *A       = get_my_table(cid);
    auto       cell_no = A->cell_no;
    AttList    atts    = A->get_cell_atts(cell_no);
    int        n       = 0;
    if (auto a = fetch_att(cid, the_names["cols"])) {
        try {
            n = std::stoi(*a);
        } catch (...) { spdlog::warn("Could not parse `{}' as an integer", shbuf); }
    }
    if (n != 0) {
        cell_no += to_unsigned(n);
    } else {
        cell_no++;
        cid.add_attribute(atts, true);
    }
    A->cell_no = (w == -1 ? cell_no : to_unsigned(w));
}

// hack...
void Stack::mark_omit_cell() { back().omit_cell = true; }

// This removes the last element of the top-stack
auto Stack::remove_last() -> Xml * { return top_stack()->remove_last(); }

void Stack::create_new_anchor(Xid xid, const std::string &id, const std::string &idtext) {
    AttList &AL              = get_att_list(xid.value);
    AL[the_names["id"]]      = id;
    AL[the_names["id-text"]] = idtext;
}

// mark current element as target for a label.
auto Stack::add_new_anchor() -> std::string {
    std::string id = next_label_id();
    set_cur_id(id);
    create_new_anchor(get_xid(), id, get_cur_label());
    return id;
}

auto Stack::add_new_anchor_spec() -> std::string {
    static size_t last_top_label_id = 0;
    auto          id                = std::string(fmt::format("cid{}", ++last_top_label_id));
    set_cur_id(id);
    create_new_anchor(get_xid(), id, get_cur_label());
    return id;
}

// Add an anchor if needed.
auto Stack::add_anchor(const std::string &s, bool spec) -> std::string {
    if (!spec && (top_stack()->tail_is_anchor())) return get_cur_id();
    std::string id = next_label_id();
    set_cur_id(id);
    if (!spec) {
        add_newid0("anchor");
        create_new_anchor(get_xid(), id, std::string(s));
    } else {
        create_new_anchor(cur_xid(), id, std::string(s));
    }
    return id;
}

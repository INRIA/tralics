#include "tralics/Xml.h"
#include "tralics/AttList.h"
#include "tralics/LabelInfo.h"
#include "tralics/Logger.h"
#include "tralics/MainClass.h"
#include "tralics/MathDataP.h"
#include "tralics/Parser.h"
#include "tralics/WordList.h"
#include "tralics/XmlAction.h"
#include "tralics/globals.h"
#include "tralics/util.h"
#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

namespace {
    std::ofstream decoy_fp;

    // Figure with subfigure. We construct a table with two rows
    // for a par. ctr holds the value of the counter for the caption.
    auto figline(Xml *from, int &ctr, Xml *junk) -> Xml * {
        Xml *row1  = new Xml(the_names["row"], nullptr);
        Xml *row2  = new Xml(the_names["row"], nullptr);
        int  nrows = 0;
        for (;;) {
            Xml *sf = from->get_first_env("subfigure");
            if (sf == nullptr) break;
            nrows++;
            if (!sf->is_element()) {
                junk->push_back_unless_nullptr(sf);
                continue;
            }
            Xml *leg   = sf->get_first_env("leg");
            Xml *texte = sf->get_first_env("texte");

            sf->add_non_empty_to(junk);
            if (texte != nullptr) {
                texte->change_name("cell");
                row1->push_back_unless_nullptr(texte);
                texte->id.add_attribute(sf->id);
            }
            if (leg != nullptr) {
                leg->change_name("cell");
                Buffer B;
                B << '(' << uchar(ctr) << ')' << ' ';
                leg->add_first(new Xml(B));
                row2->push_back_unless_nullptr(leg);
            }
            ctr++;
        }
        from->add_non_empty_to(junk);
        if (nrows == 0) return nullptr;
        Xml *res = new Xml(the_names["table"], nullptr);
        res->id.add_attribute(the_names["rend"], the_names["inline"]);
        res->push_back_unless_nullptr(row1);
        res->push_back_unless_nullptr(row2);
        return new Xml(the_names["cst_p"], res);
    }

    void table_subfigure(Xml *from, Xml *to, Xml *junk) {
        to->id.add_attribute(the_names["rend"], the_names["array"]);
        int ctr = 'a';
        for (;;) {
            Xml *sf = from->get_first_env("cst_p");
            if (sf == nullptr) break;
            if (sf->is_element()) {
                Xml *res1 = figline(sf, ctr, junk);
                if (res1 != nullptr) to->push_back_unless_nullptr(res1);
            } else
                junk->push_back_unless_nullptr(sf);
        }
    }

    void raw_subfigure(Xml *from, Xml *to, Xml *junk) {
        to->id.add_attribute(the_names["rend"], the_names["subfigure"]);
        int         n          = 0;
        static auto parid_name = std::string("parid");
        for (;;) {
            Xml *P = from->get_first_env("cst_p");
            if (P == nullptr) break;
            if (!P->is_element()) {
                junk->push_back_unless_nullptr(P);
                continue;
            }
            auto par_id = std::string(std::to_string(n));
            ++n;
            for (;;) {
                Xml *sf = P->get_first_env("subfigure");
                if (sf == nullptr) {
                    P->add_non_empty_to(junk);
                    break;
                }
                if (!sf->is_element()) {
                    junk->push_back_unless_nullptr(sf);
                    continue;
                }
                sf->id.add_attribute(parid_name, par_id);
                Xml *leg   = sf->get_first_env("leg");
                Xml *texte = sf->get_first_env("texte");
                sf->add_non_empty_to(junk);
                if (leg != nullptr) {
                    leg->change_name("head");
                    sf->push_back_unless_nullptr(leg);
                }
                if (texte != nullptr) {
                    Xml *xx = texte->get_first_env("figure");
                    if (xx != nullptr) { sf->id.add_attribute_but_rend(xx->id); }
                    texte->add_non_empty_to(junk);
                }
                to->push_back_unless_nullptr(sf);
                to->push_back_unless_nullptr(the_stack.newline_xml);
            }
        }
    }

    // Post processor of figure.
    void postprocess_figure(Xml *to, Xml *from) {
        Xml *     X{nullptr};
        XmlAction X1(the_names["table"], rc_contains);
        XmlAction X2(the_names["subfigure"], rc_contains);
        XmlAction X3(the_names["figure"], rc_how_many);
        XmlAction X4(the_names["pre"], rc_contains);
        from->recurse0(X1);
        from->recurse0(X2);
        from->recurse(X3);
        from->recurse0(X4);
        int w = 4;
        if (X1.is_ok())
            w = 1;
        else if (X2.is_ok())
            w = 2;
        else if (X4.is_ok())
            w = 3;
        else if (X3.int_val == 1)
            w = 0;
        switch (w) {
        case 0: // a single figure
            X = from->get_first_env("figure");
            if (X != nullptr) { // copy all atributes of X but rend in this
                to->id.add_attribute_but_rend(X->id);
            }
            return;
        case 1: // a table in the figure, move all tables
            X = new Xml(the_names["cst_p"], nullptr);
            to->push_back_unless_nullptr(X);
            from->move(the_names["table"], X);
            to->id.add_attribute(the_names["rend"], the_names["array"]);
            return;
        case 3: // verbatim material in the figure; move all lines
            X = new Xml(the_names["cst_empty"], nullptr);
            to->push_back_unless_nullptr(X);
            from->move(the_names["pre"], X);
            to->id.add_attribute(the_names["rend"], the_names["pre"]);
            return;
        case 2: // a subfigure
            //    T->remove_empty_par();
            X = new Xml(the_names["cst_p"], nullptr); // will contain junk
            if (eqtb_int_table[use_subfigure_code].val != 0)
                raw_subfigure(from, to, X);
            else
                table_subfigure(from, to, X);
            from->add_non_empty_to(X);
            from->swap_x(X);
            return;
        default: { // other cases
            from->remove_empty_par();
            Xml *nbsp = new Xml(std::string(" &#xA0;"));
            from->subst_env0(the_names["hfill"], nbsp);
            from->subst_env0(the_names["hfil"], nbsp);
            from->move(the_names["cst_p"], to);
            XmlAction X5(the_names["figure"], rc_return_first);
            from->recurse0(X5);
            if (X5.xml_val != nullptr) // dommage
                from->add_non_empty_to(to);
        }
        }
    }

    // This removes the object S, together with the label n
    void remove_label(String s, const std::string &n) {
        for (auto &i : ref_list) {
            std::string V  = i.second;
            auto *      li = labinfo(V);
            if (li->id != n) continue;
            if (!li->used) continue;
            log_and_tty << "Error signaled by postprocessor\n"
                        << "Removing `" << s << "' made the following label disappear: " << V << "\n";
            nb_errs++;
        }
        for (auto &defined_label : defined_labels) {
            std::string j = defined_label.first;
            LabelInfo * V = defined_label.second;
            if (j == n && V->defined && !V->used) {
                removed_labels.emplace_back(s, n);
                V->defined = false;
            }
        }
    }

    // Post processor table.
    void postprocess_table(Xml *to, Xml *from) {
        XmlAction X1(the_names["table"], rc_how_many);
        from->recurse(X1);
        // Special case: more than one tabular in the table
        // We move in to all tabular
        if (X1.int_val > 1) {
            Xml *X = new Xml(the_names["cst_p"], nullptr);
            to->push_back_unless_nullptr(X);
            from->move(the_names["table"], X);
            to->id.add_attribute(the_names["rend"], the_names["array"]);
            return;
        }
        // Normal case
        from->remove_empty_par();
        from->remove_par_bal_if_ok();
        to->id.add_attribute(the_names["rend"], the_names["display"]);
        Xml *C = from->single_non_empty();
        if (C != nullptr && C->is_element()) {
            if (C->has_name_of("figure")) {
                to->push_back_unless_nullptr(C);
                from->clear();
            } else if (C->has_name_of("formula")) {
                to->push_back_unless_nullptr(C);
                from->clear();
            } else if (C->has_name_of("table")) {
                if (!C->all_empty())
                    to->push_back_list(C);
                else
                    to->push_back_unless_nullptr(C); // This is strange...
                to->id.add_attribute_but_rend(C->id);
                from->clear();
                to->change_name("table");
            }
        }
    }
} // namespace

// Returns a new element named N, initialised with z (if not empty...)
Xml::Xml(std::string N, Xml *z) : name(std::move(N)) {
    id = the_stack.next_xid(this);
    if (z != nullptr) add_tmp(gsl::not_null{z});
}

// Case of cline, placed after a row. If action is false, we check whether the
// row, including the spans of the cells, is adapted to the pattern.
// If action is true, we do something
auto Xml::try_cline(bool action) -> bool {
    auto a    = cline_first - 1;
    bool a_ok = false; // true after skip
    for (auto &k : *this) {
        if (a == 0) {
            if (a_ok) return true;
            a    = (cline_last - cline_first) + 1;
            a_ok = true;
        }
        if (!k->is_element()) {
            std::string N = k->name;
            if (std::string(N) == "\n") continue; // allow newline separator
            return false;
        }
        auto c = k->get_cell_span();
        if (c == -1) return false;
        if (c == 0) continue; // ignore null span cells
        if (a_ok && action) k->id.add_bottom_rule();
        a = a - c;
        if (a < 0) return false;
    }
    return false; // list too small
}

// Puts the total span in res, return false in case of trouble
auto Xml::total_span(long &res) const -> bool {
    long r = 0;
    for (const auto &k : *this) {
        if (!k->is_element()) {
            std::string N = k->name;
            if (std::string(N) == "\n") continue; // allow newline separator
            return false;
        }
        auto c = k->get_cell_span();
        if (c == -1) return false;
        r += c;
    }
    res = r;
    return true;
}

// Special case when \\ started a row; we may have an empty cell followed by
// newline. Returns false if the cell is non-empty, has a span or a top border
// If action is true, we kill the cell/newline
auto Xml::try_cline_again(bool action) -> bool {
    bool seen_cell = false;
    auto len       = size();
    for (size_t k = 0; k < size(); k++) {
        if (action) {
            erase(begin() + to_signed(k));
            --k;
            continue;
        }
        if (!at(k)->is_element() && k == len - 1) {
            std::string N = at(k)->name;
            if (std::string(N) == "\n") continue;
            return false;
        }
        if (at(k)->get_cell_span() != 1) return false;
        if (!at(k)->id.has_attribute(the_names["cell_topborder"]).empty()) return false;
        if (seen_cell) return false;
        if (!at(k)->is_whitespace()) return false;
        seen_cell = true;
    }
    return seen_cell;
}

void Xml::bordermatrix() {
    if (size() <= 1) return;
    auto n = size() - 1;
    auto F = front();
    if (F != nullptr && F->is_element() && F->size() > 1) { F->insert_at(1, new Xml(the_names["mtd"], nullptr)); }
    auto att    = std::string("rowspan");
    auto attval = std::string(std::to_string(n));
    F           = at(1);
    if (F != nullptr && F->is_element() && F->size() > 1) {
        Xml *aux = new Xml(the_names["mtd"], MathDataP::mk_mo("("));
        aux->add_att(att, attval);
        F->insert_at(1, aux);
        aux = new Xml(the_names["mtd"], MathDataP::mk_mo(")"));
        aux->add_att(att, attval);
        F->push_back_unless_nullptr(aux);
    }
}

// returns the element with a new id, if it's a <mo> and has a movablelimits
// attributes; return null otherwise.
auto Xml::spec_copy() const -> Xml * {
    if (name != the_names["mo"]) return nullptr;
    AttList &X = id.get_att();
    if (X.find(the_names["movablelimits"]) == X.end()) return nullptr;
    Xml *res                                               = new Xml(name, nullptr);
    static_cast<std::vector<gsl::not_null<Xml *>> &>(*res) = *this;
    res->id.add_attribute(X, true);
    return res;
}

auto Xml::real_size() const -> long { return is_element() ? to_signed(size()) : -2; }

auto Xml::value_at(long n) -> Xml * {
    if (!is_element() || n < 0 || n >= to_signed(size())) return nullptr;
    return at(to_unsigned(n));
}

auto Xml::put_at(long n, gsl::not_null<Xml *> x) -> bool {
    if (!is_element() || n < 0 || n >= to_signed(size())) return false;
    at(to_unsigned(n)) = x;
    return true;
}

auto Xml::remove_at(long n) -> bool {
    if (!is_element() || n < 0 || n >= to_signed(size())) return false;
    erase(begin() + n);
    return true;
}

auto Xml::is_child(Xml *x) const -> bool {
    if (!is_element()) return false;
    return std::find(begin(), end(), x) != end();
}

auto Xml::deep_copy() -> gsl::not_null<Xml *> {
    if (!is_element()) return gsl::not_null{this};
    gsl::not_null res{new Xml(name, nullptr)};
    res->id.add_attribute(id);
    for (size_t i = 0; i < size(); i++) { res->push_back_unless_nullptr((*this)[i]->deep_copy()); }
    return res;
}

// This finds a sub element named match, and does some action X
// Recursion stops when found.
void Xml::recurse0(XmlAction &X) {
    for (size_t k = 0; k < size(); k++) {
        Xml *y = at(k);
        if (!y->is_element()) continue;
        if (y->has_name(X.match)) switch (X.what) {
            case rc_contains: X.mark_found(); return;
            case rc_delete_first:
                X.int_val = to_signed(y->id.value);
                erase(begin() + to_signed(k));
                return;
            case rc_return_first:
                X.xml_val = y;
                X.mark_found();
                return;
            case rc_return_first_and_kill:
                X.xml_val = y;
                erase(begin() + to_signed(k));
                X.mark_found();
                return;
            default: spdlog::critical("Fatal: illegal value in recurse0"); abort();
            }
        y->recurse0(X);
        if (X.is_ok()) return;
    }
}

// This does some action for every element named X.
void Xml::recurse(XmlAction &X) {
    for (size_t k = 0; k < size(); k++) {
        Xml *T = at(k);
        if (!T->is_element()) continue;
        if (T->has_name(X.match)) {
            switch (X.what) {
            case rc_delete:
                erase(begin() + to_signed(k));
                --k;
                continue;
            case rc_delete_empty:
                if (T->is_whitespace()) {
                    erase(begin() + to_signed(k));
                    --k;
                    continue;
                }
                break;
            case rc_how_many: ++X.int_val; break;
            case rc_subst: at(k) = gsl::not_null{X.xml_val}; continue;
            case rc_move:
                X.xml_val->push_back_unless_nullptr(T);
                erase(begin() + to_signed(k));
                --k;
                continue;
            case rc_composition: // a module in the comp section
            {
                std::string an = T->id.has_attribute(the_names["id"]);
                if (!an.empty()) remove_label("module in composition", an);
                erase(begin() + to_signed(k));
                k--;
                for (size_t j = 0; j < T->size(); j++) {
                    Xml *W = T->at(j);
                    if (W == nullptr) continue;
                    if (W->is_element() && W->has_name_of("head")) {
                        std::replace(the_parser.all_heads.begin(), the_parser.all_heads.end(), W, (Xml *)nullptr);
                        continue;
                    }
                    insert_at(k + 1, W);
                    k++;
                }
                recurse(X); // this->tree has changed
                return;     // nothing more to do.
            }
            case rc_rename: T->name = X.string_val; break;
            default: spdlog::critical("Fatal: illegal value in recurse"); abort();
            }
        }
        T->recurse(X);
    }
}

// Returns a pointer to the last son.
auto Xml::last_addr() const -> Xml * { return !empty() ? back().get() : nullptr; }

// Returns a pointer to the last element
// and removes it if it is a Box
auto Xml::last_box() -> Xml * {
    if (empty()) return nullptr;
    auto res = back();
    if (res->is_element()) {
        pop_back();
        return res;
    }
    return nullptr;
}

// Remove last item if its an empty <hi>
void Xml::remove_last_empty_hi() {
    if (empty()) return;
    auto T = back();
    if (!T->is_element()) return;
    if (!T->only_recur_hi()) return;
    pop_back();
}

// Return true if this is an empty <hi> (recursion allowed)
auto Xml::only_recur_hi() const -> bool {
    if (!id.is_font_change()) return false;
    if (empty()) return true;
    Xml *x = single_son();
    if (x == nullptr) return false;
    if (!x->is_element()) return false;
    return x->only_recur_hi();
}

// Return true if this contains only <hi>
// Exemple \mbox{\tt x} gives {\tt x}
auto Xml::only_hi() const -> bool {
    Xml *x = single_non_empty();
    if (x == nullptr) return false;
    if (!x->is_element()) return false;
    return x->id.is_font_change();
}

// Adds the content of x to this. Attributes are lost
// Assumes that x is not a string.
void Xml::push_back_list(Xml *x) {
    for (size_t i = 0; i < x->size(); i++) push_back_unless_nullptr(x->at(i));
}

// Insert X at the end; but the value if it is a temporary.
void Xml::add_tmp(gsl::not_null<Xml *> x) {
    if (x->is_element() && x->has_name_of("temporary"))
        push_back_list(x);
    else
        push_back(x);
}

// Inserts x at position pos.
void Xml::insert_at(size_t pos, Xml *x) { insert(begin() + to_signed(pos), gsl::not_null{x}); }

// Inserts x at position 0.
void Xml::add_first(Xml *x) { insert(begin(), gsl::not_null{x}); }

// This find an element with a single son, the son should be res.
auto Xml::find_on_tree(Xml *check, Xml *&res) const -> bool {
    for (auto T : *this) {
        if (T->is_element() && T->size() == 1 && T->at(0) == check) {
            res = T;
            return true;
        }
    }
    for (auto T : *this) {
        if (T->is_element() && T->find_on_tree(check, res)) return true;
    }
    return false;
}

void Xml::insert_bib(Xml *bib, Xml *match) {
    Xml *ptr = this;
    if (match != nullptr) find_on_tree(match, ptr);
    ptr->add_tmp(gsl::not_null{bib});
}

void Xml::print_on(std::ostream &o) const {
    if (!is_element()) {
        if (id.value == 0)
            o << encode(name);
        else if (id.value == size_t(-1))
            fmt::print(o, "<!--{}-->", encode(name));
        else if (id.value == size_t(-2)) {
            fmt::print(o, "<!{}", encode(name));
            for (const auto &e : *this) o << e;
            o << ">";
        } else if (id.value == size_t(-3))
            fmt::print(o, "<?{}?>", encode(name));
        return;
    }

    if (empty() && !name.empty()) {
        fmt::print(o, "<{}{}/>", name.c_str(), id.get_att());
        return;
    }

    if (!name.empty()) fmt::print(o, "<{}{}>", name.c_str(), id.get_att());
    for (const auto &e : *this) o << e;
    if (!name.empty()) fmt::print(o, "</{}>", name.c_str());
}

auto operator<<(std::ostream &fp, const Xml *T) -> std::ostream & {
    if (T != nullptr) T->print_on(fp);
    return fp;
}

// Replace <name/> by vl.
void Xml::subst_env0(std::string match, Xml *vl) {
    XmlAction X(std::move(match), rc_subst, vl);
    recurse(X);
}

// Removes and returns first element named N.
auto Xml::get_first_env(const std::string &N) -> Xml * {
    XmlAction X(the_names[N], rc_return_first_and_kill);
    recurse0(X);
    return X.xml_val;
}

// This returns true if it is possible to remove the p
auto Xml::is_empty_p() const -> bool {
    if (!has_name_of("cst_p")) return false;
    if (!empty()) return false;
    AttList &X = id.get_att();
    if (X.empty()) return true;
    if (X.size() != 1) return false;
    return std::any_of(X.begin(), X.end(), [](const auto &i) { return i.first == the_names["noindent"]; });
}

auto Xml::all_empty() const -> bool { return empty() && name.empty(); }

// Returns true if empty (white space only)
auto Xml::is_whitespace() const -> bool {
    return std::all_of(begin(), end(), [](Xml *T) { return !T->is_element() && only_space(T->name); });
}

// If there is one non-empty son returns it.
auto Xml::single_non_empty() const -> Xml * {
    Xml *res{nullptr};
    for (auto y : *this) {
        if (y->is_element()) {
            if (res == nullptr)
                res = y;
            else
                return nullptr;
        } else if (!only_space(y->name))
            return nullptr;
    }
    return res;
}

auto Xml::single_son() const -> Xml * { return size() == 1 ? at(0).get() : nullptr; }

// Removes empty <p> elements
void Xml::remove_empty_par() {
    XmlAction X(the_names["cst_p"], rc_delete_empty);
    recurse(X);
}

// This swaps the trees of this and x
void Xml::swap_x(Xml *x) { std::vector<gsl::not_null<Xml *>>::swap(*x); }

// Moves to res every son named match.
void Xml::move(std::string match, Xml *res) {
    XmlAction X(std::move(match), rc_move, res);
    recurse(X);
}

// This is used to implement \unhbox
void Xml::unbox(Xml *x) {
    if (x == nullptr) return;
    if (x->is_element()) {
        push_back_list(x);
    } else {
        Buffer &b = scbuf; // \todo without scbuf
        b.clear();
        b.append(encode(x->name));
        add_last_string(encode(x->name));
    }
}

// Replaces <foo><p>a b c</p></foo> by <foo> a b c </foo>
void Xml::remove_par_bal_if_ok() {
    Xml *res = single_non_empty();
    if ((res != nullptr) && res->is_element() && res->has_name_of("cst_p")) {
        std::vector<gsl::not_null<Xml *>>::operator=(*res);
        res->clear();
    }
}

// Post processor of figure or table.
void Xml::postprocess_fig_table(bool is_fig) {
    // First copy this into a temporarry
    Xml *T = new Xml(the_names["temporary"], nullptr);
    swap_x(T);
    // move the caption from T to this
    Xml *C = T->get_first_env("scaption");
    if (C != nullptr) {
        push_back(gsl::not_null{C}); // copy caption
        C->change_name("caption");
        push_back(gsl::not_null{the_stack.newline_xml});
    }
    C = T->get_first_env("alt_caption");
    if (C != nullptr) {
        push_back(gsl::not_null{C});
        push_back(gsl::not_null{the_stack.newline_xml});
    }

    // Move all data from T to this
    if (is_fig)
        postprocess_figure(this, T);
    else
        postprocess_table(this, T);
    // test for junk in T
    T->remove_empty_par();
    T->remove_par_bal_if_ok();
    if (T->is_whitespace()) return;
    Logger::finish_seq();
    log_and_tty << "Warning: junk in " << (is_fig ? "figure" : "table") << "\n";
    {
        int         n = the_parser.get_cur_line();
        std::string f = the_parser.get_cur_filename();
        log_and_tty << "detected on line " << n;
        if (!f.empty()) log_and_tty << " of file " << f;
        log_and_tty << ".\n";
    }
    Xml *U = new Xml(std::string("unexpected"), nullptr);
    push_back_unless_nullptr(U);
    T->add_non_empty_to(U);
}

// Adds all non-empty elements to res
void Xml::add_non_empty_to(Xml *res) {
    for (size_t k = 0; k < size(); k++) {
        Xml *T = at(k);
        if (!T->is_element() && only_space(T->name)) continue;
        res->push_back_unless_nullptr(T);
    }
}

// This is used by sT_translate. It converts an XML element
// to a string, using scbuf as temporary. clears the object
auto Xml::convert_to_string() -> std::string {
    scbuf.clear();
    convert_to_string(scbuf);
    clear();
    return scbuf;
}

// This converts the content to a string. May be recursive
void Xml::convert_to_string(Buffer &b) {
    if (!is_element()) {
        b += name;
        return;
    }
    if (name.empty() || name == the_names["temporary"]) {
        for (auto &k : *this) k->convert_to_string(b);
        return;
    }
    err_buf.clear();
    if (id.is_font_change()) {
        std::string w = id.has_attribute(the_names["rend"]);
        if (!w.empty()) {
            err_buf += "unexpected font change " + encode(w);
            the_parser.unexpected_font();
            the_parser.signal_error();
            return;
        }
    }
    err_buf += "unexpected element " + encode(name);
    the_parser.signal_error();
}

// Puts *this in the buffer B. Uses Internal Encoding
// Used to print the title of a section.
void Xml::put_in_buffer(Buffer &b) {
    for (size_t k = 0; k < size(); k++) {
        if (!at(k)->is_element())
            b += encode(at(k)->name);
        else if (at(k)->has_name_of("hi"))
            at(k)->put_in_buffer(b);
        else
            b += "<" + encode(at(k)->name) + "/>";
    }
}

// Removes and returns the last element
auto Xml::remove_last() -> Xml * {
    if (empty()) return nullptr;
    Xml *res = back();
    pop_back();
    return res;
}

// True if this is empty, or contains only a <hi> element which is empty
auto Xml::par_is_empty() -> bool {
    if (empty()) return true;
    if (size() > 1) return false;
    if (!at(0)->is_element()) return false;
    if (!at(0)->id.is_font_change()) return false;
    return at(0)->par_is_empty();
}

// This adds x at the end the element
void Xml::push_back_unless_nullptr(Xml *x) {
    if (x != nullptr) push_back(gsl::not_null{x});
}

// True if last element on the tree is a string.
auto Xml::last_is_string() const -> bool { return !empty() && back()->id.value == 0; }

// Assume that last element is a string. This string is put in the internal
// buffer
void Xml::last_to_SH() const {
    shbuf.clear();
    shbuf.append(std::string(back()->name));
}

// This adds B at the end the element, via concatenation, if possible.
void Xml::add_last_string(const std::string &B) {
    if (B.empty()) return;
    shbuf.clear();
    if (last_is_string()) {
        last_to_SH();
        pop_back();
    }
    shbuf.append(B);
    push_back_unless_nullptr(new Xml(shbuf));
}

// This adds x and a \n at the end of this.
void Xml::add_last_nl(Xml *x) {
    if (x != nullptr) {
        push_back_unless_nullptr(x);
        push_back_unless_nullptr(the_stack.newline_xml);
    }
}

// Removes a trailing space on the tree.
void Xml::remove_last_space() {
    if (!last_is_string()) return;
    last_to_SH();
    auto k = shbuf.size();
    shbuf.remove_space_at_end();
    if (k != shbuf.size()) {
        pop_back();
        if (!shbuf.empty()) push_back_unless_nullptr(new Xml(shbuf));
    }
}

// This adds a NL to the end of the element
void Xml::add_nl() {
    if (!all_empty() && back_or_nullptr() == the_stack.newline_xml) return;
    push_back_unless_nullptr(the_stack.newline_xml);
}

// This returns the span of the current cell; -1 in case of trouble
// the default value is 1
auto Xml::get_cell_span() const -> long { // \todo std::optional<size_t>
    if (!is_element()) return 0;
    if (!has_name(the_names["cell"])) return -1; // not a cell
    auto a = fetch_att(id, the_names["cols"]);
    if (!a) return 1;              // no property, default is 1
    auto o = Buffer(*a).int_val(); // \todo without Buffer
    return o ? to_signed(*o) : -1;
}

auto Xml::tail_is_anchor() const -> bool { return !empty() && back()->is_anchor(); }

#pragma once
#include "Buffer.h"
#include "NameMapper.h"
#include "Xid.h"
#include <gsl/gsl>

struct XmlAction;

class Xml : public std::vector<gsl::not_null<Xml *>> { // \todo value semantics
public:
    Xid         id{0}; ///< id of the objet
    std::string name;  ///< name of the element

    explicit Xml(const std::string &n) : name(std::string(n)) {}
    Xml(const std::string &s, Xid n) : id(n), name(the_names[s]) {}
    Xml(std::string N, Xml *z);

    [[nodiscard]] auto all_empty() const -> bool;
    [[nodiscard]] auto back_or_nullptr() const -> Xml * { return empty() ? nullptr : back().get(); }
    [[nodiscard]] auto get_cell_span() const -> long;
    [[nodiscard]] auto has_name(const std::string &s) const -> bool { return name == s; }
    [[nodiscard]] auto has_name_of(const std::string &s) const -> bool { return name == the_names[s]; }
    [[nodiscard]] auto is_anchor() const -> bool { return is_element() && name == the_names["anchor"]; }
    [[nodiscard]] auto is_element() const -> bool { return to_signed(id.value) > 0; }
    [[nodiscard]] auto is_whitespace() const -> bool;
    [[nodiscard]] auto is_empty_p() const -> bool;
    [[nodiscard]] auto last_addr() const -> Xml *;
    [[nodiscard]] auto last_is_string() const -> bool;
    [[nodiscard]] auto only_hi() const -> bool;
    [[nodiscard]] auto only_recur_hi() const -> bool;
    [[nodiscard]] auto real_size() const -> long;
    [[nodiscard]] auto single_non_empty() const -> Xml *;
    [[nodiscard]] auto single_son() const -> Xml *;
    [[nodiscard]] auto tail_is_anchor() const -> bool;
    [[nodiscard]] auto spec_copy() const -> Xml *;

    auto convert_to_string() -> std::string;
    auto deep_copy() -> gsl::not_null<Xml *>;
    auto find_on_tree(Xml *check, Xml *&res) const -> bool;
    auto get_first_env(const std::string &name) -> Xml *;

    void add_att(const std::string &a, const std::string &b) const { id.add_attribute(a, b); }
    void add_first(Xml *x);
    void add_ref(std::string s);
    void add_tmp(gsl::not_null<Xml *> x);
    void add_last_nl(Xml *x);
    void add_last_string(const std::string &B);
    void add_nl();
    void add_non_empty_to(Xml *res);
    void change_name(const std::string &s) { name = the_names[s]; }
    void convert_to_string(Buffer &B);
    void insert_at(size_t pos, Xml *x);
    void insert_bib(Xml *bib, Xml *match);
    auto is_child(Xml *x) const -> bool;
    void kill_name() { name = std::string(); }
    auto last_box() -> Xml *;
    void last_to_SH() const;
    void move(std::string match, Xml *res);
    void one_fig_tab(bool is_fig);
    auto par_is_empty() -> bool;
    void postprocess_fig_table(bool is_fig);
    auto put_at(long n, gsl::not_null<Xml *> x) -> bool;
    void put_in_buffer(Buffer &b);
    void push_back_unless_nullptr(Xml *x);
    void push_back_list(Xml *x);
    void recurse(XmlAction &X);
    void recurse0(XmlAction &X);
    void remove_empty_par();
    auto remove_at(long n) -> bool;
    auto remove_last() -> Xml *;
    void remove_last_empty_hi();
    void remove_last_space();
    void remove_par_bal_if_ok();
    void subst_env0(std::string match, Xml *vl);
    void swap_x(Xml *x);
    void print_on(std::ostream &o) const;
    auto total_span(long &res) const -> bool;
    auto try_cline(bool action) -> bool;
    auto try_cline_again(bool action) -> bool;
    void unbox(Xml *x);
    auto value_at(long n) -> Xml *;
    void replace_first(Xml *x) {
        if (!empty()) at(0) = gsl::not_null{x};
    }
    void bordermatrix();
};

inline Xml *the_page_xml;

struct XmlAndType {
    Xml *      value;
    math_types type;
};

auto read_xml(const std::string &s) -> Xml *;

auto operator<<(std::ostream &fp, const Xml *T) -> std::ostream &;

using EqtbBox = EQTB<Xml *>;

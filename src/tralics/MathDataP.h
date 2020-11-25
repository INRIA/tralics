#pragma once
#include "Math.h"
#include <array>

// This is a global object for math handling
class MathDataP : public std::vector<Math> {
    static const int                                      m_offset = 10000;
    std::array<Xml *, last_math_loc>                      built_in_table{};   // the static math table
    std::vector<Xml *>                                    xml_math_table;     // the dynamic math table
    size_t                                                xmath_pos{};        // number of slots used in the dynamic table
    size_t                                                lmath_pos{};        // number of slots used in the math table
    std::array<std::optional<std::string>, del_tablesize> xml_lr_ptable;      // table of fence attributes
    std::array<math_types, nb_mathchars>                  math_char_type{};   // the math type for +, = etc
    std::array<Xml *, nb_simplemath>                      simplemath_table{}; // translation of $x$ etc
    std::array<Xml *, 29>                                 mc_table{};
    bool                                                  no_ent_names{};
    Token                                                 nomathsw0; // says next token is for nomathml only
    Token                                                 nomathsw1; // says next token is for normal mode only
public:
    std::array<Xml *, last_math_loc> built_in_table_alt{}; // the static math table
private:
    void boot_table();
    void boot2();
    void boot_chars();
    void boot_xml_lr_tables();
    auto mk_gen(String name, String ent, String ent2, math_loc pos, math_loc pos2, const std::string &bl, symcodes t, bool hack) -> Token;
    void mk_ic(String name, String ent, String ent2, math_loc pos);
    void mk_icb(String name, String ent, String ent2, math_loc pos);
    void mk_oc(String name, String ent, String ent2, math_loc pos);
    void mk_oco(String name, String ent, String ent2, math_loc pos);
    void mk_ocol(String name, String ent, String ent2, math_loc pos);
    void mk_ocb(String name, String ent, String ent2, math_loc pos);
    void mk_ocr(String name, String ent, String ent2, math_loc pos);
    void mk_oc(String name, String ent, String ent2, math_loc pos, symcodes t, bool hack);
    void mk_moo(String name, String ent, math_loc pos);
    void fill_lr(size_t a, String b, String c);
    void fill_lr(size_t a, String b);

public:
    void boot();
    auto init_builtin(const std::string &name, math_loc pos, Xml *x, symcodes t) -> Token;
    void realloc_list0();
    void realloc_list();
    auto find_math_location(math_list_type c, subtypes n, std::string s) -> subtypes;
    auto find_xml_location() -> subtypes;
    auto find_xml_location(Xml *y) -> subtypes;
    auto make_mfenced(size_t open, size_t close, gsl::not_null<Xml *> val) -> gsl::not_null<Xml *>;
    void TM_mk(String a, String b, math_types c);
    void finish_math_mem();
    auto get_mc_table(size_t i) { return gsl::not_null{mc_table[i]}; }
    auto get_builtin(size_t p) { return gsl::not_null{built_in_table[p]}; }
    auto get_builtin_alt(size_t p) -> Xml * { return built_in_table_alt[p]; }
    void init_builtin(size_t i, Xml *X) { built_in_table[i] = X; }
    void init_builtin(size_t i, size_t j) { built_in_table[i] = built_in_table[j]; }
    void init_builtin(size_t i, Buffer &B) { built_in_table[i] = new Xml(B); }
    auto get_xml_val(size_t i) -> Xml * {
        if (i < m_offset) return built_in_table[i];
        return xml_math_table[i - m_offset];
    }
    auto get_list(size_t k) -> Math & { return (*this)[k]; }
    void push_back(size_t k, CmdChr X, subtypes c);
    auto get_simplemath_val(size_t i) -> Xml * { return simplemath_table[i]; }
    auto get_fence(size_t k) -> std::optional<std::string> { return xml_lr_ptable[k]; }
    auto get_math_char_type(size_t i) -> math_types { return math_char_type[i]; }
    void set_type(size_t k, math_list_type c);

    static auto mk_mo(String a) -> gsl::not_null<Xml *>;
    auto        mk_gen(String name, String ent, String ent2, math_loc pos, const std::string &bl, symcodes t, bool hack) -> Token;
};

inline MathDataP math_data; // \todo unique instance, should we use static stuff?

#pragma once

#include "../txio.h"
#include "../txstring.h"

class MainClass {
    std::string   infile;        // file argument given to the program
    std::string   raweb_dir;     // the main tralics dir
    std::string   raweb_dir_src; // is raweb_dir + "/src/"
    std::string   no_year;       // is miaou
    std::string   raclass;       // is ra2003
    std::string   year_string;   // is 2003
    std::string   dclass;        // documentclass of the file
    std::string   type_option;   // the type option
    std::string   dtd, dtdfile;  // the dtd, and its external location
    std::string   dtype;         // the doc type found in the configuration file
    std::string   start_date;    // string with date of start of run.
    std::string   short_date;    // date of start of run.
    std::string   tcf_file;
    std::string   machine;
    std::string   out_name;        // Name of output file
    std::string   in_dir;          // Input directory
    std::string   default_class;   //
    int           year{9876};      // current year
    int           env_number{0};   // number of environments seen
    int           current_line{0}; // current line number
    int           bibtex_size{0};
    int           bibtex_extension_size{0};
    int           tpa_mode{3};
    int           dft{3};        // default dtd for standard classes
    int           cur_fp_len;    // number of bytes sent to XML file
    LinePtr       input_content; // content of the tex source
    LinePtr       tex_source;    // the data to be translated
    LinePtr       config_file;   // content of configuratrion file
    LinePtr       from_config;   // lines extracted from the configuration
    line_iterator doc_class_pos;
    system_type   cur_os;
    // vector <EnvList> the_env; // the list of environments for checks
    std::vector<Istring>     bibtex_fields_s;
    std::vector<Istring>     bibtex_fields;
    std::vector<Istring>     bibtex_extensions;
    std::vector<Istring>     bibtex_extensions_s;
    std::vector<std::string> all_config_types;
    Buffer                   line_buffer; // buffer for current line

    Buffer               b_after, b_current; // aux buffers.
    String               external_prog{"rahandler.pl"};
    std::string          version_string{"2.15.4"};
    int                  input_encoding{1};
    output_encoding_type output_encoding{en_boot};
    output_encoding_type log_encoding{en_boot};

    bool no_zerowidthspace{false};
    bool no_zerowidthelt{false};
    bool footnote_hack{true};
    bool prime_hack{false};
    bool use_all_sizes_sw{false};
    bool noent_names{false};
    bool interactive_math{false};
    bool shell_escape_allowed{false};
    bool find_words{false};
    bool handling_ra{true};
    bool no_undef_mac{false};
    bool use_font_elt_sw{false};
    bool pack_font_elt_sw{false};
    bool distinguish_refer{true};
    bool noconfig{false};
    bool nomathml{false};
    bool dualmath{false};
    bool old_phi{false};
    bool verbose{false};          // are we verbose ?
    bool dverbose{false};         // are we verbose at begin document ?
    bool silent{false};           // are we silent ?
    bool double_quote_att{false}; // double quote as attribute value delimitor
    bool use_tcf{false};
    bool etex_enabled{true};
    bool use_math_variant{false};
    bool simplified_ra{false};
    bool todo_xml{true};

public:
    Stack * the_stack; // pointer to the stack
    StrHash SH;        // the XML hash table

public:
    MainClass();
    void               add_to_from_config(int n, Buffer &b) { from_config.add(n, b, true); }
    void               bad(std::string, std::string);
    void               bad1(std::string, std::string);
    void               bad_char_before_brace(int k, String s, String info);
    void               bad_end_env(std::string, int);
    void               bad_ignore_char(int k, String s);
    auto               check_for_tcf(const std::string &) -> bool;
    void               check_section_use();
    auto               check_theme(const std::string &) -> std::string;
    void               check_ur(const Buffer &);
    auto               get_bibtex_fields() -> std::vector<Istring> & { return bibtex_fields; }
    auto               get_bibtex_fields_s() -> std::vector<Istring> & { return bibtex_fields_s; }
    auto               get_bibtex_extensions() -> std::vector<Istring> & { return bibtex_extensions; }
    auto               get_bibtex_extensions_s() -> std::vector<Istring> & { return bibtex_extensions_s; }
    auto               get_fp_len() -> int { return cur_fp_len; }
    auto               get_footnote_hack() -> bool { return footnote_hack; }
    auto               get_no_undef_mac() -> bool { return no_undef_mac; }
    auto               get_no_year() -> std::string { return no_year; }
    auto               get_year_string() -> std::string { return year_string; }
    [[nodiscard]] auto get_input_encoding() const -> int { return input_encoding; }
    [[nodiscard]] auto get_output_encoding() const -> output_encoding_type { return output_encoding; }
    [[nodiscard]] auto get_log_encoding() const -> output_encoding_type { return log_encoding; }
    [[nodiscard]] auto get_tpa_mode() const -> int { return tpa_mode; }
    [[nodiscard]] auto get_prime_hack() const -> bool { return prime_hack; }
    [[nodiscard]] auto get_math_variant() const -> bool { return use_math_variant; }
    [[nodiscard]] auto get_zws_mode() const -> bool { return no_zerowidthspace; }
    [[nodiscard]] auto get_zws_elt() const -> bool { return no_zerowidthelt; }
    [[nodiscard]] auto get_version() const -> std::string { return version_string; }
    void               handle_one_bib_file(std::string);
    void               incr_cur_fp_len(int a) { cur_fp_len += a; }
    [[nodiscard]] auto in_ra() const -> bool { return handling_ra; }
    [[nodiscard]] auto in_simple_ra() const -> bool { return simplified_ra; }
    void               inhibit_xml() { todo_xml = false; }
    [[nodiscard]] auto is_verbose() const -> bool { return verbose; }
    [[nodiscard]] auto is_interactive_math() const -> bool { return interactive_math; }
    [[nodiscard]] auto is_shell_escape_allowed() const -> bool { return shell_escape_allowed; }
    [[nodiscard]] auto non_interactive() const -> bool { return !interactive_math; }
    void               parse_args(int argc, char **argv);
    void               parse_option(int &, int, char **argv);
    void               banner();
    [[nodiscard]] auto print_os() const -> String;
    void               read_config_and_other();
    void               RRbib(String);
    void               run();
    void               run(int n, char **argv);
    void               set_distinguish(bool b) { distinguish_refer = b; }
    void               set_doc_class_pos(line_iterator x) { doc_class_pos = x; }
    void               set_ent_names(String);
    void               set_foot_hack(bool b) { footnote_hack = b; }
    void               set_fp_len(int a) { cur_fp_len = a; }
    void               set_input_encoding(int wc);
    void               set_tcf_file(std::string s) {
        tcf_file = std::move(s);
        use_tcf  = true;
    }
    void               set_use_font(bool b) { use_font_elt_sw = b; }
    void               set_pack_font(bool b) { pack_font_elt_sw = b; }
    void               set_use_sizes(bool b) { use_all_sizes_sw = b; }
    void               set_start_date(std::string s) { start_date = std::move(s); }
    void               set_short_date(std::string s) { short_date = std::move(s); }
    void               set_default_class(std::string s) { default_class = std::move(s); }
    [[nodiscard]] auto get_short_date() const -> std::string { return short_date; }
    [[nodiscard]] auto get_default_class() const -> std::string { return default_class; }
    void               set_tpa_status(String);
    [[nodiscard]] auto use_all_sizes() const -> bool { return use_all_sizes_sw; }
    [[nodiscard]] auto use_noent_names() const -> bool { return noent_names; }
    [[nodiscard]] auto use_font_elt() const -> bool { return use_font_elt_sw; }
    auto               use_old_phi() -> bool { return old_phi; }
    auto               use_double_quote_att() -> bool { return double_quote_att; }
    [[nodiscard]] auto pack_font_elt() const -> bool { return pack_font_elt_sw; }
    void               unexpected_eof(std::string, int);
    [[nodiscard]] auto d_verbose() const -> bool { return dverbose; }
    void               bad_year();

private:
    void after_main_text();
    auto append_checked_line() -> int;
    void append_non_eof_line(String, int);
    auto append_nonempty_line() -> int;
    void bad_mod(int a, std::string b, Buffer &c);
    void boot_bibtex(bool);
    void call_dvips(std::string);
    void check_all();
    void check_before_begin(int k);
    auto check_for_alias_type(bool vb) -> bool;
    auto check_for_arg(int &p, int argc, char **argv) -> String;
    void check_for_input();
    void check_kw(int, Buffer &);
    void check_line(Buffer &);
    auto check_line_aux(Buffer &) -> bool;
    void check_mod();
    void check_options();
    void check_presentation();
    void check_project(Buffer &a);
    void check_ra_dir();
    auto check_section() -> int;
    void check_year_string(int, bool);
    void dubious_command(int k, bool where);
    void end_document();
    void end_env(std::string);
    void end_mod();
    void end_with_help(int);
    auto find_config_file() -> bool;
    auto find_document_type() -> bool;
    void find_dtd();
    void find_field(String a);
    auto find_opt_field(String info) -> bool;
    void finish_init();
    void finish_xml();
    auto get_a_new_line() -> bool;
    void get_doc_type();
    void get_machine_name();
    void get_os();
    void get_type_from_config();
    void handle_latex2init(String file_name);
    void ignore_text();
    void make_perl_script();
    void merge_bib();
    void mkcfg();
    void mk_empty();
    void more_boot();
    auto need_script() -> bool { return in_ra() && !in_simple_ra(); }
    void one_bib_file(bib_from pre, std::string bib);
    void open_main_file();
    void open_config_file();
    void out_gathered_math();
    void out_sep();
    void out_xml();
    void print_job();
    void print_mods();
    void print_mods_end(std::fstream *);
    void print_mods_end_xml();
    void print_mods_start();
    void print_mods_start_xml();
    void print_nb_error(int n);
    void read_one_file();
    void run_ra();
    void run_simple_ra();
    void run_not_ra();
    void see_aux_info(int k);
    void see_name(String s);
    void see_name1();
    void see_name2();
    void see_name3();
    void see_project();
    void see_topic();
    void show_input_size();
    auto split_one_arg(String, int &) -> String;
    void start_document(Buffer &a);
    void start_env(std::string);
    void start_error();
    void open_log();
    void trans0();
    void usage_and_quit(int);
};
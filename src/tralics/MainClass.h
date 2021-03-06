#pragma once
#include "LineList.h"
#include <filesystem>
#include <optional>

class Stack;

class MainClass {
    std::filesystem::path                infile;   ///< file argument given to the program
    std::optional<std::filesystem::path> tcf_file; ///< File name of the `tcf` to use, if found

    std::string no_year;     // is miaou
    std::string raclass;     // is ra2003
    std::string dclass;      // documentclass of the file
    std::string type_option; // the type option
    std::string dtd;         // the DTD
    std::string dtype;       // the doc type found in the configuration file
    std::string out_name;    // Name of output file
    std::string in_dir;      // Input directory
    std::string ult_name;    // absolute name of input.ult

    int year{9876};      // current year
    int env_number{0};   // number of environments seen
    int current_line{0}; // current line number
    int bibtex_size{0};
    int bibtex_extension_size{0};
    int dft{3}; // default dtd for standard classes
    int trivial_math{1};

    LineList input_content; // content of the tex source
    LineList tex_source;    // the data to be translated
    LineList config_file;   // content of configuratrion file
    LineList from_config;   // lines extracted from the configuration

    system_type cur_os{};

    std::vector<std::string> all_config_types;
    std::vector<std::string> after_conf;

    bool noconfig{false};
    bool nomathml{false};
    bool dualmath{false};
    bool silent{false}; // are we silent ?
    bool etex_enabled{true};
    bool multi_math_label{false};
    bool load_l3{false};
    bool verbose{false}; ///< Are we verbose ?

public:
    std::string default_class; ///< The default class
    std::string short_date;    ///< Date of start of run (short format)

    std::vector<std::string> bibtex_fields_s;
    std::vector<std::string> bibtex_fields;
    std::vector<std::string> bibtex_extensions;
    std::vector<std::string> bibtex_extensions_s;

    size_t               input_encoding{1};        ///< Encoding of the input file \todo one type to rule all encodings
    output_encoding_type log_encoding{en_boot};    ///< Encoding of the log file \todo this should always be UTF-8
    output_encoding_type output_encoding{en_boot}; ///< Encoding of the XML output \todo this should always be UTF-8

    line_iterator doc_class_pos;

    int tpa_mode{3};

    bool double_quote_att{false}; ///< double quote as attribute value delimitor
    bool dverbose{false};         ///< Are we verbose at begin document ?
    bool footnote_hack{true};     ///< Not sure what this activates
    bool math_variant{false};
    bool no_entnames{false};
    bool no_undef_mac{false};
    bool no_xml{false}; ///< Are we in syntax-only mode (no XML output)?
    bool no_zerowidthelt{false};
    bool no_zerowidthspace{false};
    bool pack_font_elt{false};
    bool prime_hack{false};
    bool shell_escape_allowed{false};
    bool use_all_sizes{false};
    bool use_font_elt{false};

    auto check_for_tcf(const std::string &s) -> bool; ///< Look for a `.tcf` file, and if found set `tcf_file` and `use_tcf`

    void add_to_from_config(int n, const std::string &b); ///< Add contents to `from_config`
    void run(int argc, char **argv);                      ///< Do everything
    void set_ent_names(const std::string &s);             ///< Set no_entnames from a string saying yes or no
    void set_input_encoding(size_t wc);                   ///< Set default input file encoding and log the action \todo remove?

private:
    void parse_args(int argc, char **argv);           ///< Parse the command-line arguments
    void parse_option(int &p, int argc, char **argv); ///< Interprets one command-line option, advances p
    void read_config_and_other();                     ///< Read the config file and extract all relevant information
    void set_tpa_status(const std::string &s);        ///< Handles argument of -tpa_status switch

    auto append_nonempty_line() -> int;
    auto check_for_alias_type(bool vb) -> bool;
    auto check_line_aux(Buffer &) -> bool;
    auto find_config_file() -> std::optional<std::filesystem::path>; // \todo static in MainClass.cpp
    auto find_document_type() -> bool;                               ///< Massage the output of get_doc_type
    auto find_opt_field(String info) -> bool;
    auto get_a_new_line() -> bool;
    void after_main_text();
    void append_non_eof_line(String, int);
    void bad_mod(int a, std::string b, Buffer &c);
    void boot_bibtex();
    void call_dvips(std::string);
    void check_all();
    void check_before_begin(int k);
    void check_for_input(); ///< Reads the input file named in `infile`
    void check_kw(int, Buffer &);
    void check_line(Buffer &);
    void check_mod();
    void check_options();
    void check_presentation();
    void check_project(Buffer &a);
    void check_ra_dir();
    void dubious_command(int k, bool where);
    void end_document();
    void end_env(std::string);
    void end_mod();
    void find_dtd(); ///< Finds the DTD, create default if nothing given
    void find_field(String a);
    void finish_xml();
    void get_doc_type();         ///< Determine document type from various sources
    void get_os();               ///< Sets cur_os to the current OS as a symbolic string
    void get_type_from_config(); ///< Extracts a type from the configuration file
    void handle_latex2init(String file_name);
    void ignore_text();
    void make_perl_script();
    void merge_bib();
    void mkcfg();
    void more_boot() const; ///< Finish bootstrapping
    void open_config_file(std::filesystem::path f);
    void open_log(); ///< Opens the log file, prints some information
    void open_main_file();
    void out_gathered_math();
    void out_sep();
    void out_xml(); ///< Ouput the XML and compute the word list
    void see_aux_info(int k);
    void see_name(std::filesystem::path s); ///< Extract versions of a filename with and without ext
    void show_input_size();
    void start_document(Buffer &a);
    void start_env(std::string);
    void start_error();
    void trans0(); ///< Start the latex to XML translation
};

inline MainClass the_main;

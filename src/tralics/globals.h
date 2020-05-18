#pragma once
#include "Istring.h"
#include "codepoint.h"
#include <filesystem>
#include <vector>

// \todo move from extern to inline

class Buffer;
struct LinePtr;

extern std::vector<std::filesystem::path> conf_path;
inline std::vector<std::filesystem::path> input_path;

extern std::string file_name;       // Job name, without directory
extern std::string all_themes;      //
extern std::string everyjob_string; //
inline std::string tralics_version{"2.15.4"};

extern std::vector<LinePtr> file_pool; // pool managed by filecontents

extern size_t leftquote_val, rightquote_val;

inline int init_file_pos = 0; // position in init file

extern bool bad_minus;
extern bool compatibility;
extern bool nofloat_hack;
extern bool only_input_data;
extern bool raw_bib;
extern bool seen_enddocument;
extern bool ra_ok;
inline bool global_in_load = false;
inline bool global_in_url  = false;

extern std::array<std::array<codepoint, lmaxchar>, max_encoding - 2> custom_table;

auto next_label_id() -> Istring;
auto null_cs_name() -> std::string;

namespace main_ns {
    void register_file(LinePtr &&x);                                                      ///< Push a file onto the pool
    auto search_in_confdir(const std::string &s) -> std::optional<std::filesystem::path>; ///< Searches for a file in conf_path
} // namespace main_ns

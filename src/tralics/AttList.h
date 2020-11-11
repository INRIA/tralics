#pragma once
#include <cassert>
#include <map>
#include <optional>
#include <string>
#include <vector>

struct AttList : public std::map<std::string, std::string> {          // map and not unordered_map for reproducible XML output
    [[nodiscard]] auto lookup(const std::string &x) -> std::string *; // \todo use map API
};

auto operator<<(std::ostream &o, const AttList &l) -> std::ostream &;

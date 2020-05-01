#include "tralics/Buffer.h"
#include "tralics/LabelInfo.h"
#include <memory>
#include <unordered_map>

namespace {
    std::vector<std::string> SH{"", "", " "};

    auto find_or_insert(const std::string &s) -> size_t {
        static std::unordered_map<std::string, size_t> s_to_i{{"", 1}, {" ", 2}};
        if (auto tmp = s_to_i.find(s); tmp != s_to_i.end()) return tmp->second;
        s_to_i.emplace(s, SH.size());
        SH.push_back(s);
        return SH.size() - 1;
    }
} // namespace

Istring::Istring(size_t N) : id(N), name(SH[N]), value(Buffer(name).convert_to_out_encoding()) {}

Istring::Istring(const std::string &s) : id(find_or_insert(s)), name(s), value(Buffer(name).convert_to_out_encoding()) {}

auto Istring::labinfo() const -> LabelInfo * {
    static std::unordered_map<size_t, std::unique_ptr<LabelInfo>> LI;
    if (LI.find(id) == LI.end()) LI.emplace(id, std::make_unique<LabelInfo>());
    return LI[id].get();
}

#include "tralics/AllIndex.h"
#include "tralics/MainClass.h"
#include "tralics/Stack.h"

// By default, this is a glossary and a main index
AllIndex::AllIndex() {
    emplace_back("glossary", "Glossary", 6);
    emplace_back("default", "Index", 5);
}

// Returns the index location associated to the name S
// If S is not found, the main index is used
auto AllIndex::find_index(const std::string &s) -> OneIndex & {
    auto out = std::find_if(begin(), end(), [&](const auto &i) { return i.name == s; });
    return out != end() ? *out : (*this)[1];
}

void AllIndex::new_index(const std::string &s, const std::string &title) {
    if (std::any_of(begin(), end(), [&](const auto &i) { return i.name == s; })) return;
    auto id = the_stack.next_xid(nullptr).value;
    emplace_back(s, title, id);
}

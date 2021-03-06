#include "tralics/TexRule.h"
#include "tralics/AttList.h"
#include "tralics/NameMapper.h"

// This scans a hrule or a vrule.
// May be used in \cleaders\hrule; the result is the_stack.top_stack()
// Depending on the type, some fields have a default value of 0.4pt, or 0 pt.
// Others are -\infty (in reality, -2^14pt = - 2^30sp)

namespace {
    constexpr int default_rule_dimen = -1073741824;
} // namespace

void TexRule::reset() {
    rule_h = ScaledInt{default_rule_dimen};
    rule_w = ScaledInt{default_rule_dimen};
    rule_d = ScaledInt{default_rule_dimen};
}

void TexRule::convert(AttList &res) const {
    if (rule_h.value != default_rule_dimen) res[the_names["height"]] = std::string(rule_h);
    if (rule_d.value != default_rule_dimen) res[the_names["depth"]] = std::string(rule_d);
    if (rule_w.value != default_rule_dimen) res[the_names["width"]] = std::string(rule_w);
}

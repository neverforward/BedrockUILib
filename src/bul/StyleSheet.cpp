#include "bul/StyleSheet.h"

#include <algorithm>
#include <cstddef>
#include <utility>

namespace bul {

int Selector::specificity() const noexcept {
    int value = 0;
    if (!id.empty()) {
        value += 100;
    }
    if (!className.empty()) {
        value += 10;
    }
    if (!type.empty()) {
        value += 1;
    }
    return value;
}

bool Selector::matches(std::string_view type, std::string_view className, std::string_view id) const noexcept {
    return (this->type.empty() || this->type == type) && (this->className.empty() || this->className == className)
        && (this->id.empty() || this->id == id);
}

StyleSheet& StyleSheet::rule(Selector selector, Style style) {
    mRules.push_back(Rule{std::move(selector), std::move(style)});
    return *this;
}

Style StyleSheet::styleFor(std::string_view type, std::string_view className, std::string_view id) const {
    struct Match {
        int         specificity;
        std::size_t order;
        Rule const* rule;
    };

    std::vector<Match> matches;
    matches.reserve(mRules.size());
    for (std::size_t i = 0; i < mRules.size(); ++i) {
        Rule const& rule = mRules[i];
        if (rule.selector.matches(type, className, id)) {
            matches.push_back(Match{rule.selector.specificity(), i, &rule});
        }
    }
    std::stable_sort(matches.begin(), matches.end(), [](Match const& left, Match const& right) {
        return left.specificity < right.specificity;
    });

    Style resolved;
    for (Match const& match : matches) {
        resolved.overlaidBy(match.rule->style);
    }
    return resolved;
}

namespace {

StyleSheet gSheet;

} // namespace

void setStyleSheet(StyleSheet sheet) { gSheet = std::move(sheet); }

StyleSheet const& styleSheet() { return gSheet; }

} // namespace bul

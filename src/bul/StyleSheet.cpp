#include "bul/StyleSheet.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "bul/widget/Widget.h"

namespace bul {

// -------------------------------------------------------------------- builder
//
// The builder keeps a partially filled selector and the combinator for the next
// compound; every accessor mutates that selector and returns the builder so the
// calls chain into `set`.

StyleSheet::Builder::Builder(StyleSheet& sheet, Selector selector)
: mSheet(sheet),
  mSelector(std::move(selector)) {}

StyleSheet::Builder& StyleSheet::Builder::next(Combinator combinator) {
    mNext = combinator;
    return *this;
}

/// True when the builder needs a new compound before naming a type.
bool StyleSheet::Builder::needsNewCompound() const {
    if (mSelector.steps.empty()) {
        return true;
    }
    Compound const& compound = mSelector.steps.back().compound;
    return !compound.type.empty() || compound.universal;
}

/// The compound the following property calls modify, created on demand.
Compound& StyleSheet::Builder::currentCompound() {
    if (mSelector.steps.empty()) {
        mSelector.steps.emplace_back();
    }
    return mSelector.steps.back().compound;
}

StyleSheet::Builder& StyleSheet::Builder::type(std::string_view name) {
    if (needsNewCompound()) {
        Selector::Step step;
        step.combinator = mSelector.steps.empty() ? Combinator::Descendant : mNext;
        mSelector.steps.push_back(std::move(step));
    }
    mSelector.steps.back().compound.type = std::string{name};
    return *this;
}

StyleSheet::Builder& StyleSheet::Builder::any() {
    if (needsNewCompound()) {
        Selector::Step step;
        step.combinator = mSelector.steps.empty() ? Combinator::Descendant : mNext;
        mSelector.steps.push_back(std::move(step));
    }
    mSelector.steps.back().compound.universal = true;
    return *this;
}

StyleSheet::Builder& StyleSheet::Builder::className(std::string value) {
    currentCompound().className = std::move(value);
    return *this;
}

StyleSheet::Builder& StyleSheet::Builder::id(std::string value) {
    currentCompound().id = std::move(value);
    return *this;
}

StyleSheet::Builder& StyleSheet::Builder::attr(std::string name) {
    return attr(std::move(name), {}, Attribute::Match::Exists);
}

StyleSheet::Builder& StyleSheet::Builder::attr(std::string name, std::string value, Attribute::Match match) {
    currentCompound().attributes.push_back(Attribute{std::move(name), std::move(value), match});
    return *this;
}

StyleSheet::Builder& StyleSheet::Builder::attr(std::initializer_list<std::pair<std::string, std::string>> list) {
    for (auto const& entry : list) {
        attr(entry.first, entry.second);
    }
    return *this;
}

StyleSheet::Builder& StyleSheet::Builder::pseudo(std::string_view name, std::string_view argument) {
    currentCompound().pseudos.push_back(Pseudo{std::string{name}, std::string{argument}});
    return *this;
}

StyleSheet::Builder& StyleSheet::Builder::descendant() { return next(Combinator::Descendant); }
StyleSheet::Builder& StyleSheet::Builder::child() { return next(Combinator::Child); }
StyleSheet::Builder& StyleSheet::Builder::adjacent() { return next(Combinator::Adjacent); }
StyleSheet::Builder& StyleSheet::Builder::sibling() { return next(Combinator::Sibling); }

StyleSheet& StyleSheet::Builder::set(Style style) {
    mSheet.rule(mSelector, std::move(style));
    return mSheet;
}

// ---------------------------------------------------------------------- sheet

StyleSheet::StyleSheet(std::initializer_list<Entry> entries) {
    for (auto const& entry : entries) {
        rule(entry.selector, entry.style);
    }
}

StyleSheet::Builder StyleSheet::selector() { return Builder{*this, {}}; }

StyleSheet::Builder StyleSheet::type(std::string_view name) { return selector().type(name); }

StyleSheet::Builder StyleSheet::className(std::string value) { return selector().className(std::move(value)); }

StyleSheet::Builder StyleSheet::id(std::string value) { return selector().id(std::move(value)); }

StyleSheet::Builder StyleSheet::attr(std::string name) { return selector().attr(std::move(name)); }

StyleSheet::Builder StyleSheet::attr(std::string name, std::string value, Attribute::Match match) {
    return selector().attr(std::move(name), std::move(value), match);
}

StyleSheet& StyleSheet::rule(Selector selector, Style style) {
    mRules.push_back(Rule{std::move(selector), std::move(style)});
    return *this;
}

StyleSheet& StyleSheet::rule(std::string_view selectorText, Style style) {
    for (auto& selector : Selector::parseList(selectorText)) {
        mRules.push_back(Rule{std::move(selector), style});
    }
    return *this;
}

namespace {

/// How specific a rule is, plus where it was declared: the cascade order.
struct Ranked {
    int         specificity;
    std::size_t order;
    Rule const* rule;
};

/// Every rule whose selector matches, weakest first, so overlaying them in order
/// applies the cascade.
template <class Matches>
Style resolveRules(std::vector<Rule> const& rules, Matches const& matches) {
    std::vector<Ranked> ranked;
    ranked.reserve(rules.size());
    for (std::size_t i = 0; i < rules.size(); ++i) {
        Rule const& rule = rules[i];
        if (matches(rule.selector)) {
            ranked.push_back(Ranked{rule.selector.specificity(), i, &rule});
        }
    }
    std::stable_sort(ranked.begin(), ranked.end(), [](Ranked const& left, Ranked const& right) {
        return left.specificity < right.specificity;
    });

    Style resolved;
    for (Ranked const& entry : ranked) {
        resolved.overlaidBy(entry.rule->style);
    }
    return resolved;
}

} // namespace

Style StyleSheet::styleFor(Widget const& widget) const {
    return resolveRules(mRules, [&](Selector const& selector) { return selector.matches(widget); });
}

Style StyleSheet::styleFor(std::string_view type, std::string_view className, std::string_view id) const {
    return resolveRules(mRules, [&](Selector const& selector) {
        return selector.matches(type, className, id);
    });
}

// ------------------------------------------------------------------ the sheet

namespace {

/// The sheet every widget in the process resolves against.
StyleSheet gSheet;

} // namespace

void setStyleSheet(StyleSheet sheet) { gSheet = std::move(sheet); }

StyleSheet const& styleSheet() { return gSheet; }

} // namespace bul

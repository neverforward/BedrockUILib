#include "bul/Selector.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "bul/widget/Widget.h"

namespace bul {

namespace {

/// Trims both ends; used for the `:nth-*` expression.
std::string_view trim(std::string_view text) noexcept {
    auto const isSpace = [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
    while (!text.empty() && isSpace(text.front())) {
        text.remove_prefix(1);
    }
    while (!text.empty() && isSpace(text.back())) {
        text.remove_suffix(1);
    }
    return text;
}

// ---------------------------------------------------------------- attributes
//
// One small predicate per operator keeps the matching itself a plain loop, and
// makes an unpublished operator a compile-time gap instead of a silent miss.

using AttributePredicate = bool (*)(std::string_view, std::string_view);

bool attributeEquals(std::string_view text, std::string_view value) { return text == value; }

/// One word of a space-separated list, the `~=` operator.
bool attributeWord(std::string_view text, std::string_view value) {
    std::size_t start = 0;
    while (start <= text.size()) {
        std::size_t const space = text.find(' ', start);
        std::size_t const end   = space == std::string_view::npos ? text.size() : space;
        if (text.substr(start, end - start) == value) {
            return true;
        }
        if (space == std::string_view::npos) {
            return false;
        }
        start = space + 1;
    }
    return false;
}

/// `en` or `en-...`, the `|=` operator.
bool attributePrefixDash(std::string_view text, std::string_view value) {
    return text == value || (text.size() > value.size() && text.starts_with(value) && text[value.size()] == '-');
}

bool attributeStartsWith(std::string_view text, std::string_view value) { return text.starts_with(value); }
bool attributeEndsWith(std::string_view text, std::string_view value) { return text.ends_with(value); }
bool attributeContains(std::string_view text, std::string_view value) {
    return text.find(value) != std::string_view::npos;
}

/// The comparison for one operator; `nullptr` means "the attribute exists".
AttributePredicate predicateFor(Attribute::Match match) noexcept {
    switch (match) {
    case Attribute::Match::Exists:
        return nullptr;
    case Attribute::Match::Equals:
        return attributeEquals;
    case Attribute::Match::Word:
        return attributeWord;
    case Attribute::Match::PrefixDash:
        return attributePrefixDash;
    case Attribute::Match::StartsWith:
        return attributeStartsWith;
    case Attribute::Match::EndsWith:
        return attributeEndsWith;
    case Attribute::Match::Contains:
        return attributeContains;
    }
    return nullptr;
}

// ---------------------------------------------------------------- structure
//
// Structural pseudo classes read the widget's position among its siblings, so
// they ask the parent for the child list. The helpers below return -1 when the
// widget has no parent.

/// 0-based position among the parent's children, or -1.
long siblingIndex(Widget const& widget) {
    Widget const* parent = widget.parent();
    if (parent == nullptr) {
        return -1;
    }
    std::vector<Widget*> const siblings = parent->children();
    for (std::size_t i = 0; i < siblings.size(); ++i) {
        if (siblings[i] == &widget) {
            return static_cast<long>(i);
        }
    }
    return -1;
}

/// How many siblings there are, or -1 without a parent.
long siblingCount(Widget const& widget) {
    Widget const* parent = widget.parent();
    return parent == nullptr ? -1 : static_cast<long>(parent->children().size());
}

/// 0-based position among the siblings of the same type name, or -1.
long typeIndex(Widget const& widget) {
    Widget const* parent = widget.parent();
    if (parent == nullptr) {
        return -1;
    }
    long index = -1;
    for (Widget* sibling : parent->children()) {
        if (sibling->typeName() == widget.typeName()) {
            ++index;
            if (sibling == &widget) {
                return index;
            }
        }
    }
    return -1;
}

/// How many siblings share this type, or -1 without a parent.
long typeCount(Widget const& widget) {
    Widget const* parent = widget.parent();
    if (parent == nullptr) {
        return -1;
    }
    long count = 0;
    for (Widget* sibling : parent->children()) {
        if (sibling->typeName() == widget.typeName()) {
            ++count;
        }
    }
    return count;
}

/// The previous sibling, or nullptr at the start of the list.
Widget* previousSibling(Widget const& widget) {
    Widget const* parent = widget.parent();
    if (parent == nullptr) {
        return nullptr;
    }
    std::vector<Widget*> const siblings = parent->children();
    for (std::size_t i = 0; i < siblings.size(); ++i) {
        if (siblings[i] == &widget) {
            return i == 0 ? nullptr : siblings[i - 1];
        }
    }
    return nullptr;
}

/// `odd`, `even`, `3` or `an+b`, against a 0-based index.
bool matchesNth(long index, std::string_view expression) {
    expression = trim(expression);
    if (expression == "odd") {
        return index >= 0 && index % 2 == 0;
    }
    if (expression == "even") {
        return index >= 0 && index % 2 == 1;
    }
    if (index < 0) {
        return false;
    }

    std::size_t const n = expression.find('n');
    if (n == std::string_view::npos) {
        return index + 1 == std::atol(std::string{expression}.c_str());
    }

    std::string_view const aText = trim(expression.substr(0, n));
    long const             a =
        aText.empty() || aText == "+" ? 1 : aText == "-" ? -1 : std::atol(std::string{aText}.c_str());
    std::string_view const bText = trim(expression.substr(n + 1));
    long const             b     = bText.empty() ? 0 : std::atol(std::string{bText}.c_str());
    if (a == 0) {
        return index + 1 == b;
    }
    long const diff = index + 1 - b;
    return diff % a == 0 && diff / a >= 0;
}

// ----------------------------------------------------------------- pseudos

bool isOneOf(std::string_view name, std::initializer_list<std::string_view> names) {
    return std::find(names.begin(), names.end(), name) != names.end();
}

/// The pseudo classes that only ask the widget's state.
bool matchStatePseudo(std::string_view name, Widget const& widget) {
    if (isOneOf(name, {"hover", "active"})) {
        return widget.state("hover");
    }
    if (name == "focus") {
        return widget.state("focus");
    }
    if (isOneOf(name, {"disabled", "checked", "invalid", "required"})) {
        return widget.state(name);
    }
    if (name == "enabled") {
        return !widget.state("disabled");
    }
    if (name == "valid") {
        return !widget.state("invalid");
    }
    if (name == "optional") {
        return !widget.state("required");
    }
    if (name == "read-only") {
        return widget.state("read-only");
    }
    if (name == "read-write") {
        return !widget.state("read-only");
    }
    return false;
}

/// The pseudo classes that ask where the widget sits in the tree.
bool matchStructuralPseudo(std::string_view name, std::string_view argument, Widget const& widget) {
    if (name == "root") {
        return widget.parent() == nullptr;
    }
    if (name == "empty") {
        return widget.children().empty();
    }
    if (name == "first-child") {
        return siblingIndex(widget) == 0;
    }
    if (name == "last-child") {
        long const count = siblingCount(widget);
        return count > 0 && siblingIndex(widget) == count - 1;
    }
    if (name == "only-child") {
        return siblingCount(widget) == 1;
    }
    if (name == "nth-child") {
        return matchesNth(siblingIndex(widget), argument);
    }
    if (name == "nth-last-child") {
        long const count = siblingCount(widget);
        return count > 0 && matchesNth(count - 1 - siblingIndex(widget), argument);
    }
    if (name == "first-of-type") {
        return typeIndex(widget) == 0;
    }
    if (name == "last-of-type") {
        long const count = typeCount(widget);
        return count > 0 && typeIndex(widget) == count - 1;
    }
    if (name == "only-of-type") {
        return typeCount(widget) == 1;
    }
    if (name == "nth-of-type") {
        return matchesNth(typeIndex(widget), argument);
    }
    if (name == "nth-last-of-type") {
        long const count = typeCount(widget);
        return count > 0 && matchesNth(count - 1 - typeIndex(widget), argument);
    }
    return false;
}

/// `:has(...)`: any descendant matches.
bool hasDescendant(Widget const& widget, std::string_view argument) {
    std::vector<Widget*> pending = widget.children();
    while (!pending.empty()) {
        Widget* const node = pending.back();
        pending.pop_back();
        for (auto const& inner : Selector::parseList(argument)) {
            if (inner.matches(*node)) {
                return true;
            }
        }
        for (Widget* child : node->children()) {
            pending.push_back(child);
        }
    }
    return false;
}

/// The pseudo classes that take another selector.
bool matchLogicalPseudo(std::string_view name, std::string_view argument, Widget const& widget) {
    if (name == "not") {
        auto const inner = Selector::parse(argument);
        return inner && !inner->matches(widget);
    }
    if (isOneOf(name, {"is", "where"})) {
        for (auto const& inner : Selector::parseList(argument)) {
            if (inner.matches(widget)) {
                return true;
            }
        }
        return false;
    }
    if (name == "has") {
        return hasDescendant(widget, argument);
    }
    return false;
}

/// One pseudo class against the widget; unknown names never match.
bool matchPseudo(Pseudo const& pseudo, Widget const& widget) {
    if (matchStatePseudo(pseudo.name, widget)) {
        return true;
    }
    if (matchStructuralPseudo(pseudo.name, pseudo.argument, widget)) {
        return true;
    }
    return matchLogicalPseudo(pseudo.name, pseudo.argument, widget);
}

/// One compound against one widget: type, class, id, attributes, pseudos.
bool matchCompound(Compound const& compound, Widget const& widget) {
    if (!compound.type.empty() && widget.typeName() != compound.type) {
        return false;
    }
    if (!compound.className.empty() && widget.className() != compound.className) {
        return false;
    }
    if (!compound.id.empty() && widget.id() != compound.id) {
        return false;
    }
    for (auto const& attribute : compound.attributes) {
        auto const value = widget.attribute(attribute.name);
        if (!value) {
            return false;
        }
        auto const predicate = predicateFor(attribute.match);
        if (predicate != nullptr && !predicate(*value, attribute.value)) {
            return false;
        }
    }
    for (auto const& pseudo : compound.pseudos) {
        if (!matchPseudo(pseudo, widget)) {
            return false;
        }
    }
    return true;
}

/// The selector from step `index` backwards: the compound, then its link to the
/// compound before it.
bool matchSelector(Selector const& selector, std::size_t index, Widget const& widget) {
    if (!matchCompound(selector.steps[index].compound, widget)) {
        return false;
    }
    if (index == 0) {
        return true;
    }

    switch (selector.steps[index].combinator) {
    case Combinator::Descendant:
        for (Widget* ancestor = widget.parent(); ancestor != nullptr; ancestor = ancestor->parent()) {
            if (matchSelector(selector, index - 1, *ancestor)) {
                return true;
            }
        }
        return false;
    case Combinator::Child:
        return widget.parent() != nullptr && matchSelector(selector, index - 1, *widget.parent());
    case Combinator::Adjacent: {
        Widget* const previous = previousSibling(widget);
        return previous != nullptr && matchSelector(selector, index - 1, *previous);
    }
    case Combinator::Sibling:
        for (Widget* previous = previousSibling(widget); previous != nullptr; previous = previousSibling(*previous)) {
            if (matchSelector(selector, index - 1, *previous)) {
                return true;
            }
        }
        return false;
    }
    return false;
}

} // namespace

bool Selector::matches(Widget const& widget) const {
    if (steps.empty()) {
        return true;
    }
    return matchSelector(*this, steps.size() - 1, widget);
}

bool Selector::matches(std::string_view type, std::string_view className, std::string_view id) const noexcept {
    if (steps.empty()) {
        return true;
    }
    if (steps.size() != 1) {
        return false;
    }
    Compound const& compound = steps[0].compound;
    if (!compound.type.empty() && compound.type != type) {
        return false;
    }
    if (!compound.className.empty() && compound.className != className) {
        return false;
    }
    return compound.id.empty() || compound.id == id;
}

} // namespace bul

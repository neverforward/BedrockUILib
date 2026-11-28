#pragma once

#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "bul/Export.h"
#include "bul/Selector.h"
#include "bul/Style.h"

namespace bul {

class Widget;

/// A document-wide list of component rules, the kit's spelling of a stylesheet.
/// The theme is the base palette, the rules colour whole components, and a
/// widget's own `Style` is the inline style that always wins.
///
/// A rule is built with the fluent selector, then `set`:
///
/// ```cpp
/// using namespace bul::literals;
///
/// bul::StyleSheet sheet;
/// sheet.className("danger").set({.backgroundColor = bul::Rgba::fromRgb(0xE5484D)});
/// sheet.type("Label").set({.fontSize = 14_px});
/// sheet.type<bul::Label>().set({.fontSize = 14_px});
/// sheet.type("Stack").child().type("Label").set({.color = ...});   // Stack > Label
/// sheet.type("Input").pseudo("disabled").set({.color = ...});      // Input:disabled
/// sheet.type("Input").attr("kind", "password").set({.color = ...});
/// ```
///
/// The selector itself lives in `bul/Selector.h`; this class only stores rules,
/// resolves them in cascade order and exposes the builder that fills them in.
class BUL_API StyleSheet {
public:
    /// The half-built rule the `type` / `className` / `id` / `attr` / `pseudo` /
    /// combinator calls return; `set` finishes it and hands the sheet back.
    class BUL_API Builder {
    public:
        Builder(StyleSheet& sheet, Selector selector);

        /// Starts a new compound: `type("Stack").child().type("Label")`.
        Builder& type(std::string_view name);
        /// The same with a widget class: `.type<bul::Label>()`.
        template <class T>
        Builder& type() {
            return type(T::kTypeName);
        }
        /// The universal selector, `*`.
        Builder& any();
        Builder& className(std::string value);
        Builder& id(std::string value);

        /// `[name]`, `[name=value]`, and the other attribute operators.
        Builder& attr(std::string name);
        Builder& attr(std::string name, std::string value, Attribute::Match match = Attribute::Match::Equals);
        Builder& attr(std::initializer_list<std::pair<std::string, std::string>> list);

        /// One pseudo class: `pseudo("hover")`, `pseudo("nth-child", "2")`,
        /// `pseudo("not", ".danger")`, `pseudo("has", "Label")`. Every pseudo
        /// class goes through this one function.
        Builder& pseudo(std::string_view name, std::string_view argument = {});

        /// The combinator that links the next compound to the current one.
        Builder& descendant();
        Builder& child();
        Builder& adjacent();
        Builder& sibling();

        /// Finishes the rule.
        StyleSheet& set(Style style);

    private:
        Builder& next(Combinator combinator);
        /// True when a type call has to open a new compound.
        [[nodiscard]] bool needsNewCompound() const;
        /// The compound the property calls modify, created on demand.
        Compound& currentCompound();

        StyleSheet& mSheet;
        Selector    mSelector;
        Combinator  mNext{Combinator::Descendant};
    };

    /// One rule as it is written down: a selector and the style it applies.
    struct Entry {
        Selector selector{};
        Style    style{};
    };

    StyleSheet() = default;
    StyleSheet(std::initializer_list<Entry> entries);

    Builder selector();
    Builder type(std::string_view name);
    template <class T>
    Builder type() {
        return Builder{*this, {}}.type<T>();
    }
    Builder className(std::string value);
    Builder id(std::string value);
    Builder attr(std::string name);
    Builder attr(std::string name, std::string value, Attribute::Match match = Attribute::Match::Equals);

    StyleSheet& rule(Selector selector, Style style);
    /// Parses selector text (`"Button.danger"`, `"Stack > Label"`); text that is
    /// not a selector is ignored.
    StyleSheet& rule(std::string_view selectorText, Style style);

    [[nodiscard]] bool                     empty() const noexcept { return mRules.empty(); }
    [[nodiscard]] std::vector<Rule> const& rules() const noexcept { return mRules; }

    /// The style this sheet resolves for one widget: every matching rule, in
    /// cascade order (specificity, then declaration order), so stronger and
    /// later rules win.
    [[nodiscard]] Style styleFor(Widget const& widget) const;

    /// The same for a widget described by its three attributes, for tests and
    /// for callers that have no widget at hand.
    [[nodiscard]] Style styleFor(std::string_view type, std::string_view className, std::string_view id) const;

private:
    std::vector<Rule> mRules;
};

/// The active sheet, the document of the UI: every widget resolves its style
/// against it while painting.
BUL_API void                            setStyleSheet(StyleSheet sheet);
[[nodiscard]] BUL_API StyleSheet const& styleSheet();

} // namespace bul

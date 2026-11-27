#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "bul/Export.h"
#include "bul/Style.h"

namespace bul {

class Widget;

/// A CSS-like selector: every part that is set must match. An empty part matches
/// anything, so `{ .type = "Button" }` is the `Button` element selector and
/// `{ .className = "danger" }` is the `.danger` class selector.
struct BUL_API Selector {
    std::string type{};      // element selector, e.g. "Button"
    std::string className{}; // class selector, e.g. "danger"
    std::string id{};        // id selector, e.g. "submit"

    /// CSS specificity: id 100, class 10, element 1.
    [[nodiscard]] int specificity() const noexcept;

    [[nodiscard]] bool matches(std::string_view type, std::string_view className, std::string_view id) const noexcept;
};

/// One selector with the style it applies.
struct BUL_API Rule {
    Selector selector{};
    Style    style{};
};

/// A document-wide list of component rules, the kit's spelling of a stylesheet:
/// the theme is the base palette, the rules colour whole components, and a
/// widget's own `Style` is the inline style that always wins.
///
/// ```cpp
/// bul::StyleSheet sheet;
/// sheet.rule({.type = "Button"}, {.primary = bul::Rgba::fromRgb(0x4C8BF5)});
/// sheet.rule({.className = "danger"}, {.primary = bul::Rgba::fromRgb(0xE5484D)});
/// bul::setStyleSheet(std::move(sheet));
/// ```
class BUL_API StyleSheet {
public:
    StyleSheet& rule(Selector selector, Style style);

    [[nodiscard]] bool                     empty() const noexcept { return mRules.empty(); }
    [[nodiscard]] std::vector<Rule> const& rules() const noexcept { return mRules; }

    /// The style this sheet resolves for one widget: every matching rule, in
    /// cascade order (specificity, then declaration order), so stronger and
    /// later rules win.
    [[nodiscard]] Style styleFor(std::string_view type, std::string_view className, std::string_view id) const;

private:
    std::vector<Rule> mRules;
};

/// The active sheet, the document of the UI: every widget resolves its style
/// against it while painting.
BUL_API void                            setStyleSheet(StyleSheet sheet);
[[nodiscard]] BUL_API StyleSheet const& styleSheet();

} // namespace bul

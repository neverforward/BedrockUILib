#pragma once

#include <optional>

#include "bul/Canvas.h"
#include "bul/Color.h"
#include "bul/Export.h"
#include "bul/Geometry.h"
#include "bul/Literals.h"

namespace bul {

struct Theme;

/// A four-sided spacing, each side a style length: `Padding::all(4_px)` or
/// `Padding::symmetric(1_em, 4_px)`. A plain `Insets` converts into it, so code
/// written before units existed keeps working.
struct BUL_API Padding {
    Length left{};
    Length top{};
    Length right{};
    Length bottom{};

    constexpr Padding() noexcept = default;
    constexpr Padding(Length left, Length top, Length right, Length bottom) noexcept
    : left(left),
      top(top),
      right(right),
      bottom(bottom) {}
    constexpr Padding(Insets const& insets) noexcept
    : left(insets.left),
      top(insets.top),
      right(insets.right),
      bottom(insets.bottom) {}

    [[nodiscard]] static constexpr Padding all(Length value) noexcept { return {value, value, value, value}; }

    [[nodiscard]] static constexpr Padding symmetric(Length horizontal, Length vertical) noexcept {
        return {horizontal, vertical, horizontal, vertical};
    }

    /// The spacing in GUI units, given the font height relative units resolve
    /// against.
    [[nodiscard]] constexpr Insets resolve(float baseFont) const noexcept {
        return {
            left.resolve(baseFont),
            top.resolve(baseFont),
            right.resolve(baseFont),
            bottom.resolve(baseFont),
        };
    }

    friend constexpr bool operator==(Padding const&, Padding const&) = default;
};

/// Whether the widget draws at all. A hidden widget keeps its space, like the web's
/// `visibility: hidden`; `display: none` (no space at all) is not modelled.
enum class Visibility { Visible, Hidden };

/// `text-transform`: how the printed text is cased.
enum class TextTransform { None, Uppercase, Lowercase, Capitalize };

/// `white-space`, the parts that change line breaking: `normal` wraps, `nowrap`
/// and `pre` do not.
enum class WhiteSpace { Normal, NoWrap, Pre };

/// `vertical-align` for a text block: where it sits inside its box.
enum class TextVertical { Top, Middle, Bottom };

/// `display`: a `none` widget takes no space and draws nothing.
enum class Display { Block, None };

/// `font-weight`, to the extent the engine font can show it.
enum class FontWeight { Normal, Bold };

/// `font-style`; the kit's font has no oblique face, so italic currently
/// draws upright.
enum class FontStyle { Normal, Italic };

/// Per-widget visual overrides, the kit's spelling of an HTML `style=""`
/// attribute. Every property follows the stylesheet vocabulary: it means what its namesake means, and
/// it replaces the component's own default for that slot.
///
/// Every property is optional, so a component resolves "style, then theme". A
/// container passes its resolved theme down to its children, which is how a
/// style on a `Stack` colours the whole subtree until a child overrides a
/// property itself.
struct BUL_API Style {
    /// Text colour, like the web's `color`.
    std::optional<Rgba> color{};
    /// The widget's own surface, like the web's `background-color`.
    std::optional<Rgba> backgroundColor{};
    /// Border colour, like the web's `border-color`.
    std::optional<Rgba> borderColor{};
    /// Border stroke width, like the web's `border-width`.
    std::optional<Length> borderWidth{};
    /// Inner spacing, like the web's `padding` (1 to 4 values).
    std::optional<Padding> padding{};
    /// Space between a container's children, like the web's `gap`.
    std::optional<Length> gap{};
    /// Text height, like the web's `font-size`; resolved against the theme's base font,
    /// so `14_px` and `2_em` mean what the stylesheet means.
    std::optional<Length> fontSize{};
    /// The colour a control is filled with when it is on, like the web's
    /// `accent-color` on a checkbox or a range input.
    std::optional<Rgba> accentColor{};

    // ---------------------------------------------------------- box and text

    /// Outer spacing, like the web's `margin` (one length per side).
    std::optional<Padding> margin{};
    /// Fixed and bounded sizes, like the web's `width` / `height` / `min-*` /
    /// `max-*`.
    std::optional<Length> width{};
    std::optional<Length> height{};
    std::optional<Length> minWidth{};
    std::optional<Length> minHeight{};
    std::optional<Length> maxWidth{};
    std::optional<Length> maxHeight{};
    /// Horizontal alignment of text, like the web's `text-align`.
    std::optional<Align> textAlign{};
    /// Line advance, like the web's `line-height`.
    std::optional<Length> lineHeight{};
    /// Like the web's `visibility`.
    std::optional<Visibility> visibility{};
    /// Like the web's `text-transform`.
    std::optional<TextTransform> textTransform{};
    /// Like the web's `white-space`, for line breaking.
    std::optional<WhiteSpace> whiteSpace{};
    /// Like the web's `vertical-align`, for text blocks.
    std::optional<TextVertical> verticalAlign{};

    /// Like the web's `display`; `None` drops the widget from the layout.
    std::optional<Display> display{};
    /// Like the web's `border-radius`, a length.
    std::optional<Length> borderRadius{};
    /// 0 to 1, applied to everything the widget and its children paint.
    std::optional<float> opacity{};
    /// Like the web's `letter-spacing`, a length added between glyphs.
    std::optional<Length> letterSpacing{};
    /// Like the web's `font-weight`.
    std::optional<FontWeight> fontWeight{};
    /// Like the web's `font-style`.
    std::optional<FontStyle> fontStyle{};
    /// Obfuscated text, the `§k` run of the game's own text format. Not a web style;
    /// a kit extension because the game has the code and the web does not.
    std::optional<bool> obfuscated{};

    /// Overlays `over`: every property it sets replaces this one's.
    Style& overlaidBy(Style const& over) noexcept;

    [[nodiscard]] bool empty() const noexcept;
};

/// The theme a widget paints with: the inherited theme with this style applied.
[[nodiscard]] BUL_API Theme applyStyle(Theme const& base, Style const& style);

} // namespace bul

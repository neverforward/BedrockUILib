#pragma once

#include <optional>

#include "bul/Canvas.h"
#include "bul/Color.h"
#include "bul/Export.h"
#include "bul/Geometry.h"
#include "bul/Style.h"

namespace bul {

/// Visual tokens. Every value here is a placeholder default: the library has no
/// look of its own, consumers override what they care about. Colours are
/// straight RGBA in [0, 1].
///
/// The `optional` fields at the bottom are the style layer: a `Style` or a
/// stylesheet rule sets them, and a component falls back to its own token when
/// they are empty. That is what lets `Button { background-color: #4C8BF5; }`
/// mean "paint the button fill", while `primary` stays the button's default.
struct BUL_API Theme {
    Rgba text{};
    Rgba textMuted{};
    Rgba panel{};
    Rgba panelBorder{};
    Rgba accent{};

    /// Primary button (the default one): the action the author wants first.
    Rgba primary{};
    Rgba primaryHover{};
    Rgba primaryText{};
    /// Neutral surface used by outline hovers.
    Rgba button{};
    Rgba buttonHover{};
    /// Secondary button: a quieter surface next to the default one.
    Rgba secondary{};
    Rgba secondaryHover{};
    /// Destructive button: an action that deletes or discards.
    Rgba destructive{};
    Rgba destructiveHover{};
    Rgba destructiveText{};

    /// Text field surface; slightly inset against `panel`.
    Rgba input{};
    /// Text field surface under the pointer.
    Rgba inputHover{};
    /// Text field border, also used by the disabled and invalid states with an
    /// alpha change instead of a separate token.
    Rgba inputBorder{};
    /// Marker of an invalid field: its border and the required asterisk. Not the
    /// destructive button surface, which is far too dark to read as a message.
    Rgba invalid{};

    float fontScale{1.f};
    /// Body text height in GUI units: the base an `em`, `ex` or `%` length is
    /// resolved against.
    float fontSize{9.f};
    /// Inner padding of a container; none by default, so existing layouts keep
    /// their size until a style or a stylesheet asks for one.
    std::optional<Insets> padding{};
    /// Space between stacked children.
    float gap{8.f};
    /// Opacity of the full-screen backdrop behind a modal window.
    float scrim{0.22f};
    /// Padding of the modal window frame around the root widget.
    float windowPadding{10.f};

    // --------------------------------------------------------------- style layer
    //
    // Set by a `Style` or by a stylesheet rule; a component uses them in place of
    // its own slot and falls back to the token above when they are empty.

    /// Any text the component draws, like the web's `color`.
    std::optional<Rgba> color{};
    /// The component's own surface, like the web's `background-color`.
    std::optional<Rgba> background{};
    /// Text alignment for the components that place text themselves.
    std::optional<Align> textAlign{};
    /// Line advance for the components that lay text out line by line.
    std::optional<float> lineHeight{};
    /// `visibility: hidden` sets this to false.
    bool visible{true};
    /// `text-transform`.
    std::optional<TextTransform> textTransform{};
    /// `white-space`, for line breaking.
    std::optional<WhiteSpace> whiteSpace{};
    /// `vertical-align`, for text blocks.
    std::optional<TextVertical> verticalAlign{};
    /// `display: none` clears this: the widget takes no space and draws nothing.
    bool displayed{true};
    /// Rounded surface and border radius; 0 draws square corners.
    float borderRadius{0.f};
    /// Whole-subtree alpha; 1 is opaque.
    float opacity{1.f};
    /// Extra advance between glyphs.
    float letterSpacing{0.f};
    /// `font-weight: bold`, drawn as a `§l` run.
    bool bold{false};
    /// `font-style: italic`, drawn as a `§o` run.
    bool italic{false};
    /// Obfuscated text, drawn as a `§k` run.
    bool obfuscated{false};
    /// Border colour, like the web's `border-color`.
    std::optional<Rgba> border{};
    /// Border stroke width, like the web's `border-width`.
    std::optional<float> borderWidth{};

    [[nodiscard]] static Theme neutral();
};

} // namespace bul

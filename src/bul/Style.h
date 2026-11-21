#pragma once

#include <optional>

#include "bul/Color.h"
#include "bul/Export.h"

namespace bul {

struct Theme;

/// Per-widget colour overrides, the kit's spelling of an HTML `style=""`
/// attribute: the widget's own colours, merged on top of the `Theme` it
/// inherited from its container.
///
/// `Style` only carries the values the author set (each token is optional), so a
/// component resolves its colours as "style, then theme". A container passes its
/// resolved theme down to its children, which is how a style on a `Stack` colours
/// the whole subtree until a child overrides a token itself.
struct BUL_API Style {
    std::optional<Rgba> text{};
    std::optional<Rgba> textMuted{};
    std::optional<Rgba> panel{};
    std::optional<Rgba> panelBorder{};
    std::optional<Rgba> accent{};

    std::optional<Rgba> primary{};
    std::optional<Rgba> primaryHover{};
    std::optional<Rgba> primaryText{};
    std::optional<Rgba> button{};
    std::optional<Rgba> buttonHover{};
    std::optional<Rgba> secondary{};
    std::optional<Rgba> secondaryHover{};
    std::optional<Rgba> destructive{};
    std::optional<Rgba> destructiveHover{};
    std::optional<Rgba> destructiveText{};

    std::optional<Rgba> input{};
    std::optional<Rgba> inputHover{};
    std::optional<Rgba> inputBorder{};
    std::optional<Rgba> invalid{};

    /// Overlays `over`: every token it sets replaces this one's.
    Style& overlaidBy(Style const& over) noexcept;

    [[nodiscard]] bool empty() const noexcept;
};

/// The theme a widget paints with: the inherited theme with this style applied.
[[nodiscard]] BUL_API Theme applyStyle(Theme const& base, Style const& style);

} // namespace bul

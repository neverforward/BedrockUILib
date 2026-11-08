#pragma once

#include "bul/Color.h"
#include "bul/Export.h"

namespace bul {

/// Visual tokens. Every value here is a placeholder default: the library has no
/// look of its own, consumers override what they care about. Colours are
/// straight RGBA in [0, 1].
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
    /// Inner padding of a panel.
    float padding{10.f};
    /// Space between stacked children.
    float gap{8.f};
    /// Opacity of the full-screen backdrop behind a modal window.
    float scrim{0.22f};

    [[nodiscard]] static Theme neutral();
};

} // namespace bul

#pragma once

#include "bul/Color.h"

namespace bul {

/// Visual tokens. Every value here is a placeholder default: the library has no
/// look of its own, consumers override what they care about. Colours are
/// straight RGBA in [0, 1].
struct Theme {
    Rgba  text{};
    Rgba  textMuted{};
    Rgba  panel{};
    Rgba  panelBorder{};
    Rgba  button{};
    Rgba  buttonHover{};
    Rgba  accent{};
    float fontScale{1.f};
    /// Inner padding of a widget (button, panel).
    float padding{10.f};
    /// Space between stacked children.
    float gap{8.f};
    /// Opacity of the full-screen backdrop behind a modal window.
    float scrim{0.22f};

    [[nodiscard]] static Theme neutral();
};

} // namespace bul

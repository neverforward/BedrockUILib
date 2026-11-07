#include "bul/Theme.h"

namespace bul {

Theme Theme::neutral() {
    Theme theme;
    theme.text        = Rgba::fromRgb(0xF2F4F8);
    theme.textMuted   = Rgba::fromRgb(0x9AA0AA);
    theme.panel       = Rgba::fromRgb(0x1A1D24);
    theme.panelBorder = Rgba::fromRgb(0x737884);
    theme.button      = Rgba::fromRgb(0x2A2F3A);
    theme.buttonHover = Rgba::fromRgb(0x3A4150);
    theme.accent      = Rgba::fromRgb(0x59A9FF);
    return theme;
}

} // namespace bul

#include "bul/Theme.h"

namespace bul {

Theme Theme::neutral() {
    Theme theme;
    theme.text        = Rgba::fromRgb(0xF2F4F8);
    theme.textMuted   = Rgba::fromRgb(0x9AA0AA);
    theme.panel       = Rgba::fromRgb(0x1A1D24);
    theme.panelBorder = Rgba::fromRgb(0x737884);
    theme.accent      = Rgba::fromRgb(0x59A9FF);
    // Neutral palette: the prominent button is a lighter grey than the quiet one,
    // so the two stay distinguishable without the library picking a brand colour.
    theme.primary      = Rgba::fromRgb(0x4A5160);
    theme.primaryHover = Rgba::fromRgb(0x5A6273);
    theme.primaryText  = Rgba::fromRgb(0xF7F9FC);
    theme.button       = Rgba::fromRgb(0x2A2F3A);
    theme.buttonHover  = Rgba::fromRgb(0x3A4150);
    // Secondary is deliberately quiet: a dark neutral next to the primary fill.
    theme.secondary        = Rgba::fromRgb(0x272B33);
    theme.secondaryHover   = Rgba::fromRgb(0x323843);
    theme.destructive      = Rgba::fromRgb(0x7A2E2E);
    theme.destructiveHover = Rgba::fromRgb(0x933636);
    theme.destructiveText  = Rgba::fromRgb(0xFFE6E6);
    // Fields sit one step darker than the panel so they read as an inset
    // surface, the way the vanilla text edit box does.
    theme.input       = Rgba::fromRgb(0x11141A);
    theme.inputHover  = Rgba::fromRgb(0x161A22);
    theme.inputBorder = Rgba::fromRgb(0x5A6070);
    // The invalid marker has to stay readable on both the panel and the field
    // surface, so it is a lightened red rather than the button's dark one.
    theme.invalid = Rgba::fromRgb(0xE05A5A);
    return theme;
}

} // namespace bul

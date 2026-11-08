#include "bul/widget/Button.h"

#include <utility>

#include "bul/Theme.h"

namespace bul {

Button::Button(std::string text, std::function<void()> onClick)
: mText(std::move(text)),
  mOnClick(std::move(onClick)) {}

Button::Metrics Button::metrics(Theme const& theme) const noexcept {
    // Sizes only change scale and padding; colours come from the type. Default
    // keeps the previous height (line + 2 * 5) so existing layouts do not jump.
    switch (mSize) {
    case Size::XSmall:
        return {0.8f * theme.fontScale, 6.f, 1.f};
    case Size::Small:
        return {0.9f * theme.fontScale, 8.f, 3.f};
    case Size::Large:
        return {1.15f * theme.fontScale, 14.f, 8.f};
    case Size::Default:
    default:
        return {1.f * theme.fontScale, 10.f, 5.f};
    }
}

bul::Size Button::measure(float, Theme const& theme, Canvas const& canvas) const {
    Metrics const profile = metrics(theme);
    return {
        canvas.textWidth(mText, profile.textScale) + 2.f * profile.padX + mMargin.horizontal(),
        canvas.lineHeight(profile.textScale) + 2.f * profile.padY + mMargin.vertical()
    };
}

void Button::paint(Canvas& canvas, Theme const& theme) const {
    Metrics const profile = metrics(theme);
    bool const    hovered = mHovered && !disabled();
    float const   fade    = disabled() ? 0.5f : 1.f;
    Rgba          surface{};
    Rgba          border{};
    Rgba          textColor = theme.text;
    bool          filled    = true;
    bool          outlined  = true;

    switch (mType) {
    case Type::Default:
        surface   = hovered ? theme.primaryHover : theme.primary;
        border    = hovered ? theme.primaryHover : theme.primary;
        textColor = theme.primaryText;
        break;
    case Type::Secondary:
        surface = hovered ? theme.secondaryHover : theme.secondary;
        border  = theme.panelBorder;
        break;
    case Type::Destructive:
        surface   = hovered ? theme.destructiveHover : theme.destructive;
        border    = theme.destructive;
        textColor = theme.destructiveText;
        break;
    case Type::Outline:
        filled  = hovered; // transparent surface until hovered
        surface = theme.buttonHover;
        border  = hovered ? theme.accent : theme.panelBorder;
        break;
    case Type::Link:
        filled    = false;
        outlined  = false;
        textColor = hovered ? theme.accent : theme.text;
        break;
    }

    if (filled) {
        canvas.fill(mBounds, surface, fade);
    }
    if (outlined) {
        canvas.outline(mBounds, border.withAlpha(fade), 1.f);
    }
    canvas.text(
        mText,
        {mBounds.x, mBounds.y + (mBounds.height - canvas.lineHeight(profile.textScale)) / 2.f},
        mBounds.width,
        textColor.withAlpha(fade),
        profile.textScale,
        Align::Center
    );
}

void Button::activate() {
    if (disabled()) {
        return;
    }
    if (mOnClick) {
        mOnClick();
    }
}

} // namespace bul

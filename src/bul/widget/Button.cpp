#include "bul/Spec.h"

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
    Metrics profile{};
    switch (mSize) {
    case Size::XSmall:
        profile = {0.8f * theme.fontScale, 6.f, 1.f};
        break;
    case Size::Small:
        profile = {0.9f * theme.fontScale, 8.f, 3.f};
        break;
    case Size::Large:
        profile = {1.15f * theme.fontScale, 14.f, 8.f};
        break;
    case Size::Default:
    default:
        profile = {1.f * theme.fontScale, 10.f, 5.f};
        break;
    }
    // A padding in the style or the stylesheet replaces the size profile's.
    if (theme.padding) {
        profile.padX = theme.padding->horizontal() / 2.f;
        profile.padY = theme.padding->vertical() / 2.f;
    }
    return profile;
}

bul::Size Button::measureContent(float, Theme const& theme, Canvas const& canvas) const {
    Metrics const profile = metrics(theme);
    return {
        canvas.textWidth(mText, profile.textScale) + 2.f * profile.padX + mMargin.horizontal(),
        canvas.lineHeight(profile.textScale) + 2.f * profile.padY + mMargin.vertical()
    };
}

void Button::paintContent(Canvas& canvas, Theme const& theme) const {
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

    // The style layer wins over the type's own colours: `background-color` paints
    // the fill, `color` the label, `border-*` the outline.
    if (theme.background) {
        surface = *theme.background;
        filled  = true;
    }
    if (theme.color) {
        textColor = *theme.color;
    }
    if (theme.border) {
        border   = *theme.border;
        outlined = true;
    }
    float const borderWidth = theme.borderWidth.value_or(1.f);

    float const radius = theme.borderRadius;
    if (filled) {
        Widget::paintBox(canvas, mBounds, radius, surface, fade, outlined ? borderWidth : 0.f);
    }
    if (outlined) {
        Widget::paintBorder(canvas, mBounds, radius, border, borderWidth, fade);
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


Button& Button::apply(spec::Button const& node) {
    type(node.type);
    size(node.size);
    disabled(node.disabled);
    return *this;
}
} // namespace bul

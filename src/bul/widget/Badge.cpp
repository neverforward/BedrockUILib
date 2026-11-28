#include "bul/widget/Badge.h"

#include <utility>

#include "bul/Theme.h"

namespace bul {

namespace {

constexpr float kPadX  = 5.f;
constexpr float kPadY  = 1.5f;
constexpr float kScale = 0.75f;

} // namespace

Badge::Badge(std::string text, Variant variant) : mText(std::move(text)), mVariant(variant) {}

Size Badge::measureContent(float, Theme const& theme, Canvas const& canvas) const {
    float const scale = theme.fontScale * kScale;
    float const padX  = theme.padding ? theme.padding->horizontal() / 2.f : kPadX;
    float const padY  = theme.padding ? theme.padding->vertical() / 2.f : kPadY;
    return {
        canvas.textWidth(mText, scale) + 2.f * padX + mMargin.horizontal(),
        canvas.lineHeight(scale) + 2.f * padY + mMargin.vertical()
    };
}

void Badge::paintContent(Canvas& canvas, Theme const& theme) const {
    float const scale  = theme.fontScale * kScale;
    bool        filled = true;
    Rgba        surface{};
    Rgba        border{};
    Rgba        textColor = theme.text;

    switch (mVariant) {
    case Variant::Default:
        surface   = theme.primary;
        border    = theme.primary;
        textColor = theme.primaryText;
        break;
    case Variant::Secondary:
        surface   = theme.secondary;
        border    = theme.secondary;
        textColor = theme.text;
        break;
    case Variant::Outline:
        filled = false;
        border = theme.panelBorder;
        break;
    case Variant::Destructive:
        surface   = theme.destructive;
        border    = theme.destructive;
        textColor = theme.destructiveText;
        break;
    }

    if (theme.background) {
        surface = *theme.background;
        filled  = true;
    }
    if (theme.color) {
        textColor = *theme.color;
    }
    if (theme.border) {
        border = *theme.border;
    }
    float const borderWidth = theme.borderWidth.value_or(1.f);

    float const radius = theme.borderRadius;
    if (filled) {
        Widget::paintBox(canvas, mBounds, radius, surface, 1.f, borderWidth);
    }
    Widget::paintBorder(canvas, mBounds, radius, border, borderWidth, 1.f);
    canvas.text(
        mText,
        {mBounds.x, mBounds.y + (mBounds.height - canvas.lineHeight(scale)) / 2.f},
        mBounds.width,
        textColor,
        scale,
        Align::Center
    );
}

} // namespace bul

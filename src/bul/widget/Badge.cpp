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

Size Badge::measure(float, Theme const& theme, Canvas const& canvas) const {
    float const scale = theme.fontScale * kScale;
    return {
        canvas.textWidth(mText, scale) + 2.f * kPadX + mMargin.horizontal(),
        canvas.lineHeight(scale) + 2.f * kPadY + mMargin.vertical()
    };
}

void Badge::paint(Canvas& canvas, Theme const& theme) const {
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

    if (filled) {
        canvas.fill(mBounds, surface, 1.f);
    }
    canvas.outline(mBounds, border, 1.f);
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

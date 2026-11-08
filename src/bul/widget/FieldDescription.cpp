#include "bul/widget/FieldDescription.h"

#include <utility>

#include "bul/Theme.h"

namespace bul {

FieldDescription::FieldDescription(std::string text, float scale) : mText(std::move(text)), mScale(scale) {}

Size FieldDescription::measure(float, Theme const& theme, Canvas const& canvas) const {
    float const scale = theme.fontScale * mScale;
    return {canvas.textWidth(mText, scale) + mMargin.horizontal(), canvas.lineHeight(scale) + mMargin.vertical()};
}

void FieldDescription::paint(Canvas& canvas, Theme const& theme) const {
    float const scale = theme.fontScale * mScale;
    float const alpha = mState.disabled ? 0.5f : 1.f;
    canvas.text(
        mText,
        {mBounds.x, mBounds.y + (mBounds.height - canvas.lineHeight(scale)) / 2.f},
        mBounds.width,
        theme.textMuted.withAlpha(alpha),
        scale,
        Align::Left
    );
}

} // namespace bul

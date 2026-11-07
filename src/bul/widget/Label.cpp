#include "bul/widget/Label.h"

#include <utility>

#include "bul/Theme.h"

namespace bul {

Label::Label(std::string text, float scale, Align align) : mText(std::move(text)), mScale(scale), mAlign(align) {}

Size Label::measure(float, Theme const& theme, Canvas const& canvas) const {
    float const size = theme.fontScale * mScale;
    return {canvas.textWidth(mText, size) + mMargin.horizontal(), canvas.lineHeight(size) + mMargin.vertical()};
}

void Label::paint(Canvas& canvas, Theme const& theme) const {
    float const size = theme.fontScale * mScale;
    canvas.text(mText, {mBounds.x, mBounds.y}, mBounds.width, mMuted ? theme.textMuted : theme.text, size, mAlign);
}

} // namespace bul

#include "bul/widget/Button.h"

#include <utility>

#include "bul/Theme.h"

namespace bul {

Button::Button(std::string text, std::function<void()> onClick)
: mText(std::move(text)),
  mOnClick(std::move(onClick)) {}

Size Button::measure(float, Theme const& theme, Canvas const& canvas) const {
    float const size  = theme.fontScale;
    float const textW = canvas.textWidth(mText, size);
    return {
        textW + 2.f * theme.padding + mMargin.horizontal(),
        canvas.lineHeight(size) + theme.padding + mMargin.vertical()
    };
}

void Button::paint(Canvas& canvas, Theme const& theme) const {
    canvas.fill(mBounds, mHovered ? theme.buttonHover : theme.button, 1.f);
    canvas.outline(mBounds, theme.panelBorder, 1.f);
    canvas.text(
        mText,
        {mBounds.x, mBounds.y + (mBounds.height - canvas.lineHeight(theme.fontScale)) / 2.f},
        mBounds.width,
        theme.text,
        theme.fontScale,
        Align::Center
    );
}

void Button::activate() {
    if (mOnClick) {
        mOnClick();
    }
}

} // namespace bul

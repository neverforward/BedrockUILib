#include "bul/widget/FieldLabel.h"

#include <algorithm>
#include <utility>

#include "bul/Theme.h"

namespace bul {

namespace {

constexpr float kBadgeGap         = 4.f;
constexpr char  kRequiredMarker[] = " *";

} // namespace

FieldLabel::FieldLabel(std::string text, float scale) : mText(std::move(text)), mScale(scale) {}

FieldLabel& FieldLabel::badge(std::unique_ptr<Widget> widget) {
    mBadge = std::move(widget);
    return *this;
}

float FieldLabel::textWidth(Theme const& theme, Canvas const& canvas) const noexcept {
    float const scale = theme.fontScale * mScale;
    float       width = canvas.textWidth(mText, scale);
    if (required()) {
        width += canvas.textWidth(kRequiredMarker, scale);
    }
    return width;
}

Size FieldLabel::measure(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    float const scale  = theme.fontScale * mScale;
    float       width  = textWidth(theme, canvas);
    float       height = canvas.lineHeight(scale);
    if (mBadge) {
        Size const badgeSize  = mBadge->measure(availableWidth, theme, canvas);
        width                += kBadgeGap + badgeSize.width;
        height                = std::max(height, badgeSize.height);
    }
    return {width + mMargin.horizontal(), height + mMargin.vertical()};
}

void FieldLabel::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    Widget::layout(outer, theme, canvas);
    if (!mBadge) {
        return;
    }
    Size const  badgeSize = mBadge->measure(mBounds.width, theme, canvas);
    float const x         = mBounds.x + textWidth(theme, canvas) + kBadgeGap;
    float const y         = mBounds.y + (mBounds.height - badgeSize.height) / 2.f;
    mBadge->layout({x, y, badgeSize.width, badgeSize.height}, theme, canvas);
}

void FieldLabel::paint(Canvas& canvas, Theme const& theme) const {
    float const scale = theme.fontScale * mScale;
    float const y     = mBounds.y + (mBounds.height - canvas.lineHeight(scale)) / 2.f;
    float const alpha = disabled() ? 0.5f : 1.f;

    Rgba color = (mMuted ? theme.textMuted : theme.text).withAlpha(alpha);
    canvas.text(mText, {mBounds.x, y}, 0.f, color, scale, Align::Left);

    if (required()) {
        float const markerX = mBounds.x + canvas.textWidth(mText, scale);
        canvas.text(kRequiredMarker, {markerX, y}, 0.f, theme.invalid.withAlpha(alpha), scale, Align::Left);
    }
    if (mBadge) {
        mBadge->paint(canvas, theme);
    }
}

} // namespace bul

#include "bul/Spec.h"

#include "bul/widget/Switch.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "bul/Spec.h"
#include "bul/Theme.h"

#include "bul/widget/TextFlow.h"

namespace bul {

using namespace detail;

namespace {

constexpr float kWidth       = 22.f;
constexpr float kHeight      = 12.f;
constexpr float kThumb       = 10.f;
constexpr float kInset       = 1.f;
constexpr float kGap         = 6.f;
constexpr float kDisabled    = 0.5f;
constexpr float kInvalidCast = 0.45f;

/// A filled stadium (a rectangle with fully round ends) drawn as one crisp pixel
/// per unit: every row spans between the two end circles.
void fillStadium(Canvas& canvas, Point topLeft, float width, float height, Rgba color, float alpha) {
    float const radius  = height / 2.f;
    float const centerY = topLeft.y + radius;
    int const   rows    = static_cast<int>(std::round(height));
    for (int row = 0; row < rows; ++row) {
        float const dy    = (topLeft.y + static_cast<float>(row) + 0.5f) - centerY;
        float const dx    = std::sqrt(std::max(radius * radius - dy * dy, 0.f));
        float const left  = topLeft.x + radius - dx;
        float const right = topLeft.x + width - radius + dx;
        if (right > left) {
            canvas.fill({left, topLeft.y + static_cast<float>(row), right - left, 1.f}, color, alpha);
        }
    }
}

/// A crisp pixel circle, the same rule as the radio disc: a unit pixel is filled
/// when its centre is inside the radius.
void fillCircle(Canvas& canvas, Point topLeft, float diameter, Rgba color, float alpha) {
    int const   size   = static_cast<int>(std::round(diameter));
    float const radius = diameter / 2.f;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float const dx = static_cast<float>(x) + 0.5f - radius;
            float const dy = static_cast<float>(y) + 0.5f - radius;
            if (dx * dx + dy * dy > radius * radius) {
                continue;
            }
            canvas.fill({topLeft.x + static_cast<float>(x), topLeft.y + static_cast<float>(y), 1.f, 1.f}, color, alpha);
        }
    }
}

} // namespace

Switch::Switch() = default;

Switch& Switch::text(std::string text) {
    mText = std::move(text);
    return *this;
}

Switch& Switch::checked(bool value) {
    mChecked = value;
    return *this;
}

Switch& Switch::onCheckedChange(std::function<void(bool)> callback) {
    mOnCheckedChange = std::move(callback);
    return *this;
}

Switch& Switch::disabled(bool value) {
    mLocalDisabled = value;
    return *this;
}

Switch& Switch::readOnly(bool value) {
    mLocalReadOnly = value;
    return *this;
}

Switch& Switch::invalid(bool value) {
    mLocalInvalid = value;
    return *this;
}

Switch& Switch::required(bool value) noexcept {
    mLocalRequired = value;
    return *this;
}

void Switch::toggle() {
    if (disabled() || readOnly()) {
        return;
    }
    mChecked = !mChecked;
    if (mOnCheckedChange) {
        mOnCheckedChange(mChecked);
    }
}

Size Switch::measureContent(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    float const scale      = theme.fontScale;
    float const lineHeight = canvas.lineHeight(scale);
    float const height     = std::max(kHeight, lineHeight);
    float const textWidth  = mText.empty() ? 0.f : frameWidth(buildFrame(canvas, scale, mText));
    float       width      = kWidth + (mText.empty() ? 0.f : kGap + textWidth);
    if (availableWidth > 0.f) {
        width = std::min(width, availableWidth);
    }
    return {width, height};
}

void Switch::paintContent(Canvas& canvas, Theme const& theme) const {
    float const scale      = theme.fontScale;
    float const lineHeight = canvas.lineHeight(scale);
    float const fade       = disabled() ? kDisabled : 1.f;
    bool const  invalid    = mLocalInvalid || mState.invalid;

    Rect const track{mBounds.x, mBounds.y + (mBounds.height - kHeight) / 2.f, kWidth, kHeight};

    Rgba trackColor =
        mChecked ? theme.accent : (mFocused || (mHovered && !disabled()) ? theme.buttonHover : theme.button);
    if (invalid && mChecked) {
        trackColor = theme.invalid;
    }
    fillStadium(canvas, {track.x, track.y}, kWidth, kHeight, trackColor, fade);
    if (invalid && !mChecked) {
        fillStadium(canvas, {track.x, track.y}, kWidth, kHeight, theme.invalid, fade * kInvalidCast);
    }

    // The thumb: at the left when off, at the right when on.
    float const thumbX = track.x + (mChecked ? kWidth - kThumb - kInset : kInset);
    float const thumbY = track.y + (kHeight - kThumb) / 2.f;
    fillCircle(canvas, {thumbX, thumbY}, kThumb, theme.text, fade);

    if (!mText.empty()) {
        float const textX = track.right() + kGap;
        float const room  = std::max(mBounds.right() - textX, 0.f);
        drawFittedText(
            canvas,
            mText,
            {textX, mBounds.y + (mBounds.height - lineHeight) / 2.f},
            room,
            scale,
            theme.text.withAlpha(fade)
        );
    }
}

void Switch::activate() { toggle(); }

bool Switch::key(KeyStroke const& stroke) {
    if (stroke.key == Key::Space && !disabled()) {
        toggle();
        return true;
    }
    return false;
}


Switch& Switch::apply(spec::Switch const& node) {
    if (!node.text.empty()) {
        text(node.text);
    }
    checked(node.checked);
    readOnly(node.readOnly);
    disabled(node.disabled);
    invalid(node.invalid);
    if (node.required) {
        required(true);
    }
    if (node.onCheckedChange) {
        onCheckedChange(node.onCheckedChange);
    }
    return *this;
}
} // namespace bul

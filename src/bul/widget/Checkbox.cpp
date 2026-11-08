#include "bul/widget/Checkbox.h"

#include <algorithm>
#include <utility>

#include "bul/Theme.h"
#include "bul/widget/TextFlow.h"

namespace bul {

using namespace detail;

namespace {

constexpr float kBox           = 11.f;
constexpr float kGap           = 6.f;
constexpr float kBorder        = 1.f;
constexpr float kDisabledAlpha = 0.5f;

/// One pixel of the tick, relative to the inner top-left of the box: a short
/// stroke down, then the long stroke up to the right.
constexpr Point kTick[] = {
    {0.f, 2.f},
    {1.f, 3.f},
    {2.f, 4.f},
    {3.f, 3.f},
    {4.f, 2.f},
    {5.f, 1.f},
    {6.f, 0.f},
};

} // namespace

Checkbox::Checkbox() = default;

Checkbox& Checkbox::text(std::string text) {
    mText = std::move(text);
    return *this;
}

Checkbox& Checkbox::checked(bool value) {
    mChecked = value;
    return *this;
}

Checkbox& Checkbox::indeterminate(bool value) {
    mIndeterminate = value;
    return *this;
}

Checkbox& Checkbox::onCheckedChange(std::function<void(bool)> callback) {
    mOnCheckedChange = std::move(callback);
    return *this;
}

Checkbox& Checkbox::disabled(bool value) {
    mLocalDisabled = value;
    return *this;
}

Checkbox& Checkbox::readOnly(bool value) {
    mLocalReadOnly = value;
    return *this;
}

Checkbox& Checkbox::invalid(bool value) {
    mLocalInvalid = value;
    return *this;
}

Checkbox& Checkbox::required(bool value) noexcept {
    mLocalRequired = value;
    return *this;
}

void Checkbox::toggle() {
    if (disabled() || readOnly()) {
        return;
    }
    mChecked       = mIndeterminate || !mChecked;
    mIndeterminate = false;
    if (mOnCheckedChange) {
        mOnCheckedChange(mChecked);
    }
}

Size Checkbox::measure(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    float const scale      = theme.fontScale;
    float const lineHeight = canvas.lineHeight(scale);
    float const height     = std::max(kBox, lineHeight);
    float const textWidth  = mText.empty() ? 0.f : frameWidth(buildFrame(canvas, scale, mText));
    float       width      = kBox + (mText.empty() ? 0.f : kGap + textWidth);

    if (availableWidth > 0.f) {
        width = std::min(width, availableWidth);
    }
    return {width + mMargin.horizontal(), height + mMargin.vertical()};
}

void Checkbox::paint(Canvas& canvas, Theme const& theme) const {
    float const scale      = theme.fontScale;
    float const lineHeight = canvas.lineHeight(scale);
    float const fade       = disabled() ? kDisabledAlpha : 1.f;
    bool const  marked     = mChecked || mIndeterminate;

    Rect const box{mBounds.x, mBounds.y + (mBounds.height - kBox) / 2.f, kBox, kBox};

    // The box is a plain filled square: no border and no focus ring. States are
    // told apart by colour alone -- the raised surface while unticked (lighter
    // under the pointer or keyboard focus), the accent once ticked or mixed, and
    // the invalid colour when the field flagged an error.
    bool const invalid = mLocalInvalid || mState.invalid;
    Rgba const idle    = mFocused || (mHovered && !disabled()) ? theme.buttonHover : theme.button;
    Rgba const fill    = marked ? (invalid ? theme.invalid : theme.accent) : idle;
    canvas.fill(box, fill, fade);
    if (invalid && !marked) {
        // Unticked stays a surface square with a red cast; a ticked box is fully
        // red instead, because a red tint over the accent turns muddy.
        canvas.fill(box, theme.invalid, fade * 0.45f);
    }

    // Checkbox.Indicator: the tick, or the mixed dash. Drawn in the panel colour
    // so it reads on the accent fill. The tick sits two pixels down and one to
    // the right of the inner top-left so it is centred in the box.
    if (marked && fade > 0.f) {
        float const innerX = box.x + kBorder + 1.f;
        float const innerY = box.y + kBorder + 2.f;
        Rgba const  ink    = theme.panel.withAlpha(fade);
        if (mIndeterminate) {
            canvas.fill({innerX + 2.f, innerY + 2.f, 5.f, 1.f}, ink, 1.f);
        } else {
            for (Point const& pixel : kTick) {
                canvas.fill({innerX + pixel.x, innerY + pixel.y, 1.f, 1.f}, ink, 1.f);
            }
        }
    }

    if (!mText.empty()) {
        float const textX = box.right() + kGap;
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

void Checkbox::activate() { toggle(); }

bool Checkbox::key(KeyStroke const& stroke) {
    if (stroke.key == Key::Space && !disabled()) {
        toggle();
        return true;
    }
    return false;
}

} // namespace bul

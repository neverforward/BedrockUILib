#include "bul/widget/RadioGroup.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "bul/Theme.h"
#include "bul/widget/TextFlow.h"

namespace bul {

using namespace detail;

namespace {

constexpr float kDisc        = 11.f;
constexpr float kGap         = 6.f;
constexpr float kDisabled    = 0.5f;
constexpr float kInvalidCast = 0.45f;

/// A filled circle drawn as one crisp pixel per unit.
///
/// A pixel is either in or out, by whether its centre is inside the radius: no
/// alpha blending, so the rim stays sharp at any GUI scale. The disc is 13 units
/// wide on purpose -- an 11-unit circle has too few steps per side and reads as a
/// square with cut corners, while coverage anti-aliasing at that size only blurs.
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

class RadioGroup::Radio final : public Widget {
public:
    Radio(RadioGroup& group, std::string value, std::string text, bool disabled)
    : mGroup(group),
      mValue(std::move(value)),
      mText(std::move(text)),
      mLocalDisabled(disabled) {}

    [[nodiscard]] std::string const& value() const noexcept { return mValue; }
    [[nodiscard]] bool               checked() const noexcept { return mGroup.value() == mValue; }
    [[nodiscard]] bool               disabled() const noexcept { return mLocalDisabled || mGroup.disabled(); }

    [[nodiscard]] Size measure(float availableWidth, Theme const& theme, Canvas const& canvas) const override {
        float const scale      = theme.fontScale;
        float const lineHeight = canvas.lineHeight(scale);
        float const height     = std::max(kDisc, lineHeight);
        float const textWidth  = mText.empty() ? 0.f : frameWidth(buildFrame(canvas, scale, mText));
        float       width      = kDisc + (mText.empty() ? 0.f : kGap + textWidth);
        if (availableWidth > 0.f) {
            width = std::min(width, availableWidth);
        }
        return {width, height};
    }

    void paint(Canvas& canvas, Theme const& theme) const override {
        float const scale      = theme.fontScale;
        float const lineHeight = canvas.lineHeight(scale);
        float const fade       = disabled() ? kDisabled : 1.f;
        bool const  chosen     = checked();
        // An invalid group with no choice flags every disc; once something is
        // chosen only the chosen disc carries the error, so the others stay quiet.
        bool const invalid = mGroup.invalid() && (chosen || mGroup.value().empty());

        Point const disc{mBounds.x, mBounds.y + (mBounds.height - kDisc) / 2.f};
        Rgba const  fill = chosen ? (invalid ? theme.invalid : theme.accent)
                                  : (mFocused || (mHovered && !disabled()) ? theme.buttonHover : theme.button);
        fillCircle(canvas, disc, kDisc, fill, fade);
        if (!chosen && invalid) {
            fillCircle(canvas, disc, kDisc, theme.invalid, fade * kInvalidCast);
        }

        // Radio.Indicator: a dot in the middle of the disc, in the panel colour so
        // it reads on both the accent and the invalid fill.
        if (chosen && fade > 0.f) {
            fillCircle(canvas, {disc.x + 3.f, disc.y + 3.f}, 5.f, theme.panel, fade);
        }

        if (!mText.empty()) {
            float const textX = mBounds.x + kDisc + kGap;
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

    void activate() override {
        if (disabled()) {
            return;
        }
        mGroup.pick(mValue);
    }

    bool key(KeyStroke const& stroke) override {
        if (disabled()) {
            return false;
        }
        switch (stroke.key) {
        case Key::Space:
            mGroup.pick(mValue);
            return true;
        case Key::Up:
        case Key::Left:
            mGroup.move(-1);
            return true;
        case Key::Down:
        case Key::Right:
            mGroup.move(1);
            return true;
        default:
            return false;
        }
    }

    [[nodiscard]] bool interactive() const noexcept override { return !disabled(); }
    [[nodiscard]] bool focusable() const noexcept override { return !disabled(); }
    void               setHovered(bool hovered) override { mHovered = hovered; }
    void               setFocused(bool focused) override { mFocused = focused; }

private:
    RadioGroup& mGroup;
    std::string mValue;
    std::string mText;
    bool        mLocalDisabled{};
    bool        mHovered{};
    bool        mFocused{};
};

RadioGroup::RadioGroup() = default;

RadioGroup::~RadioGroup() = default;

RadioGroup& RadioGroup::direction(Stack::Direction value) {
    mDirection = value;
    return *this;
}

RadioGroup& RadioGroup::gap(float value) {
    mGap = value;
    return *this;
}

RadioGroup& RadioGroup::add(std::string value, std::string text) {
    return add(std::move(value), std::move(text), false);
}

RadioGroup& RadioGroup::add(std::string value, std::string text, bool disabled) {
    mRadios.push_back(std::make_unique<Radio>(*this, std::move(value), std::move(text), disabled));
    return *this;
}

RadioGroup& RadioGroup::value(std::string value) {
    mValue = std::move(value);
    return *this;
}

RadioGroup& RadioGroup::onValueChange(std::function<void(std::string const&)> callback) {
    mOnValueChange = std::move(callback);
    return *this;
}

RadioGroup& RadioGroup::disabled(bool value) {
    mLocalDisabled = value;
    return *this;
}

RadioGroup& RadioGroup::readOnly(bool value) {
    mLocalReadOnly = value;
    return *this;
}

RadioGroup& RadioGroup::invalid(bool value) {
    mLocalInvalid = value;
    return *this;
}

RadioGroup& RadioGroup::required(bool value) noexcept {
    mLocalRequired = value;
    return *this;
}

void RadioGroup::pick(std::string const& value) {
    if (disabled() || readOnly() || mValue == value) {
        return;
    }
    mValue = value;
    if (mOnValueChange) {
        mOnValueChange(mValue);
    }
}

void RadioGroup::move(int step) {
    if (disabled() || readOnly() || mRadios.empty()) {
        return;
    }
    auto const count = static_cast<int>(mRadios.size());
    int        index = -1;
    for (int i = 0; i < count; ++i) {
        if (mRadios[static_cast<std::size_t>(i)]->value() == mValue) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        index = step >= 0 ? -1 : count;
    }
    for (int attempt = 0; attempt < count; ++attempt) {
        index = (index + step + count) % count;
        if (!mRadios[static_cast<std::size_t>(index)]->disabled()) {
            pick(mRadios[static_cast<std::size_t>(index)]->value());
            return;
        }
    }
}

Size RadioGroup::measure(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    bool const  vertical = mDirection == Stack::Direction::Vertical;
    float const gap      = mGap >= 0.f ? mGap : theme.gap;
    float       main     = 0.f;
    float       cross    = 0.f;
    for (std::size_t i = 0; i < mRadios.size(); ++i) {
        Size const size = mRadios[i]->measure(availableWidth, theme, canvas);
        if (vertical) {
            main  += size.height;
            cross  = std::max(cross, size.width);
        } else {
            main  += size.width;
            cross  = std::max(cross, size.height);
        }
        if (i + 1 < mRadios.size()) {
            main += gap;
        }
    }
    return vertical ? Size{cross, main} : Size{main, cross};
}

void RadioGroup::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    Widget::layout(outer, theme, canvas);
    if (mRadios.empty()) {
        return;
    }
    bool const  vertical = mDirection == Stack::Direction::Vertical;
    float const gap      = mGap >= 0.f ? mGap : theme.gap;
    float       cursor   = vertical ? mBounds.y : mBounds.x;
    for (auto& radio : mRadios) {
        Size const size = radio->measure(mBounds.width, theme, canvas);
        if (vertical) {
            radio->layout({mBounds.x, cursor, mBounds.width, size.height}, theme, canvas);
            cursor += size.height + gap;
        } else {
            radio->layout({cursor, mBounds.y, size.width, size.height}, theme, canvas);
            cursor += size.width + gap;
        }
    }
}

void RadioGroup::paint(Canvas& canvas, Theme const& theme) const {
    for (auto const& radio : mRadios) {
        radio->paint(canvas, theme);
    }
}

Widget* RadioGroup::hitTest(Point point) {
    for (auto it = mRadios.rbegin(); it != mRadios.rend(); ++it) {
        if (Widget* hit = (*it)->hitTest(point)) {
            return hit;
        }
    }
    return mBounds.contains(point) ? this : nullptr;
}

void RadioGroup::setControlState(ControlState const& state) { mState = state; }

void RadioGroup::collectFocusables(std::vector<Widget*>& out) {
    for (auto& radio : mRadios) {
        radio->collectFocusables(out);
    }
}

} // namespace bul

#include "bul/widget/Select.h"

#include <algorithm>
#include <utility>

#include "bul/Theme.h"
#include "bul/widget/TextFlow.h"

namespace bul {

using namespace detail;

namespace {

constexpr float kBorder        = 1.f;
constexpr float kPadX          = 6.f;
constexpr float kPadY          = 4.f;
constexpr float kMinWidth      = 120.f;
constexpr float kIconWidth     = 12.f;
constexpr float kIconSize      = 6.f;
constexpr float kPopupGap      = 2.f;
constexpr float kPopupPad      = 2.f;
constexpr float kItemPadY      = 2.f;
constexpr float kRowPadX       = 4.f;
constexpr float kMarkerSize    = 3.f;
constexpr float kLabelGap      = 4.f;
constexpr float kDisabledAlpha = 0.5f;

} // namespace

Select::Select() = default;

Select& Select::placeholder(std::string text) {
    mPlaceholder = std::move(text);
    return *this;
}

Select& Select::item(std::string value, std::string label) {
    mItems.push_back(Item{std::move(value), std::move(label), false});
    return *this;
}

Select& Select::item(std::string value, std::string label, bool disabled) {
    mItems.push_back(Item{std::move(value), std::move(label), disabled});
    return *this;
}

Select& Select::value(std::string value) {
    mValue = std::move(value);
    return *this;
}

Select& Select::onValueChange(std::function<void(std::string const&)> callback) {
    mOnValueChange = std::move(callback);
    return *this;
}

Select& Select::onOpenChange(std::function<void(bool)> callback) {
    mOnOpenChange = std::move(callback);
    return *this;
}

Select& Select::open(bool value) {
    setOpen(value);
    return *this;
}

Select& Select::disabled(bool value) {
    mLocalDisabled = value;
    return *this;
}

Select& Select::required(bool value) noexcept {
    mLocalRequired = value;
    return *this;
}

std::string Select::displayText() const {
    int const index = selectedIndex();
    if (index >= 0) {
        Item const& item = mItems[static_cast<std::size_t>(index)];
        return item.label.empty() ? item.value : item.label;
    }
    if (!mValue.empty()) {
        return mValue;
    }
    return mPlaceholder;
}

int Select::selectedIndex() const noexcept {
    if (mValue.empty()) {
        return -1;
    }
    for (std::size_t index = 0; index < mItems.size(); ++index) {
        if (mItems[index].value == mValue) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

int Select::itemAt(Point point) const noexcept {
    if (!mPopup.contains(point) || mItemHeight <= 0.f) {
        return -1;
    }
    float const local = point.y - (mPopup.y + kPopupPad);
    if (local < 0.f) {
        return -1;
    }
    auto const index = static_cast<std::size_t>(local / mItemHeight);
    if (index >= mItems.size()) {
        return -1;
    }
    return static_cast<int>(index);
}

int Select::hoveredIndex() const noexcept {
    int const index = itemAt(mPointer);
    if (index < 0 || mItems[static_cast<std::size_t>(index)].disabled) {
        return -1;
    }
    return index;
}

void Select::setOpen(bool value) {
    if (mOpen == value) {
        return;
    }
    mOpen = value;
    if (mOpen) {
        mHighlight = selectedIndex();
        if (mHighlight >= 0 && mItems[static_cast<std::size_t>(mHighlight)].disabled) {
            mHighlight = -1;
        }
        if (mHighlight < 0) {
            moveHighlight(1);
        }
    } else {
        mHighlight = -1;
    }
    if (mOnOpenChange) {
        mOnOpenChange(mOpen);
    }
}

void Select::moveHighlight(int step) noexcept {
    int const count = static_cast<int>(mItems.size());
    if (count == 0) {
        mHighlight = -1;
        return;
    }
    int index = mHighlight;
    for (int attempt = 0; attempt < count; ++attempt) {
        if (index < 0) {
            index = step >= 0 ? 0 : count - 1;
        } else {
            index = (index + step + count) % count;
        }
        if (!mItems[static_cast<std::size_t>(index)].disabled) {
            mHighlight = index;
            return;
        }
    }
    mHighlight = -1;
}

void Select::select(int index) {
    Item const& item    = mItems[static_cast<std::size_t>(index)];
    bool const  changed = item.value != mValue;
    mValue              = item.value;
    setOpen(false);
    if (changed && mOnValueChange) {
        mOnValueChange(mValue);
    }
}

Size Select::measure(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    float const scale  = theme.fontScale;
    float const height = canvas.lineHeight(scale) + 2.f * kPadY + 2.f * kBorder;

    // Like an input, the width follows the placeholder, never the chosen value:
    // picking a long option would otherwise resize the form under the pointer.
    float const text = frameWidth(buildFrame(canvas, scale, mPlaceholder));

    float width = std::max(text + 2.f * kPadX + kIconWidth + 2.f * kBorder, kMinWidth);
    if (availableWidth > 0.f) {
        width = std::min(width, availableWidth);
    }
    return {width + mMargin.horizontal(), height + mMargin.vertical()};
}

void Select::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    Widget::layout(outer, theme, canvas);
    mItemHeight = canvas.lineHeight(theme.fontScale) + 2.f * kItemPadY;

    // The popup is at least as wide as the trigger, but no label is cut: a wider
    // list hangs to the right instead of showing "..." for a short option.
    float widest = 0.f;
    for (auto const& item : mItems) {
        std::string const& label = item.label.empty() ? item.value : item.label;
        widest                   = std::max(widest, frameWidth(buildFrame(canvas, theme.fontScale, label)));
    }
    float const content = widest + 2.f * kRowPadX + kMarkerSize + kLabelGap + 2.f * kBorder;
    mPopup              = {
        mBounds.x,
        mBounds.bottom() + kPopupGap,
        std::max(mBounds.width, content),
        kPopupPad * 2.f + mItemHeight * static_cast<float>(mItems.size())
    };
}

void Select::paint(Canvas& canvas, Theme const& theme) const {
    float const scale      = theme.fontScale;
    float const lineHeight = canvas.lineHeight(scale);
    float const fade       = disabled() ? 0.5f : 1.f;

    Rgba border = theme.inputBorder;
    if (mState.invalid) {
        border = theme.invalid;
    } else if (mFocused || mOpen) {
        border = theme.accent;
    }

    // While the popup is open the pointer is claimed by the overlay, so the
    // trigger must not read "hovered" and light up for the whole screen.
    bool const lit = mHovered && !mOpen && !disabled();
    canvas.fill(mBounds, lit ? theme.inputHover : theme.input, fade);

    float const textX = mBounds.x + kBorder + kPadX;
    float const room  = std::max(mBounds.width - 2.f * kBorder - 2.f * kPadX - kIconWidth, 0.f);
    bool const  empty = mValue.empty();
    drawFittedText(
        canvas,
        displayText(),
        {textX, mBounds.y + (mBounds.height - lineHeight) / 2.f},
        room,
        scale,
        (empty ? theme.textMuted : theme.text).withAlpha(fade)
    );

    // Select.Icon: a small downward triangle at the right edge.
    float const iconX     = mBounds.right() - kBorder - kPadX - kIconSize;
    float const iconY     = mBounds.y + (mBounds.height - 3.f) / 2.f;
    Rgba const  iconColor = theme.textMuted.withAlpha(fade);
    canvas.fill({iconX, iconY, kIconSize, 1.f}, iconColor, 1.f);
    canvas.fill({iconX + 1.f, iconY + 1.f, kIconSize - 2.f, 1.f}, iconColor, 1.f);
    canvas.fill({iconX + 2.f, iconY + 2.f, kIconSize - 4.f, 1.f}, iconColor, 1.f);

    canvas.outline(mBounds, border.withAlpha(fade), kBorder);
}

void Select::paintOverlay(Canvas& canvas, Theme const& theme) const {
    if (!mOpen || mItems.empty()) {
        return;
    }
    float const scale      = theme.fontScale;
    float const lineHeight = canvas.lineHeight(scale);
    Rect const  popup      = mPopup;

    canvas.fill(popup, theme.panel, 1.f);
    canvas.outline(popup, theme.panelBorder, 1.f);

    int const hovered = hoveredIndex();
    int const active  = hovered >= 0 ? hovered : mHighlight;

    for (std::size_t index = 0; index < mItems.size(); ++index) {
        bool const disabledItem = mItems[index].disabled;
        bool const selected     = mItems[index].value == mValue;
        Rect const row{
            popup.x + kBorder,
            popup.y + kPopupPad + static_cast<float>(index) * mItemHeight,
            std::max(popup.width - 2.f * kBorder, 0.f),
            mItemHeight
        };

        if (static_cast<int>(index) == active && !disabledItem) {
            canvas.fill(row, theme.buttonHover, 1.f);
        } else if (selected) {
            canvas.fill(row, theme.button, 1.f);
        }

        float const markerX = row.x + kRowPadX;
        float const markerY = row.y + (row.height - kMarkerSize) / 2.f;
        if (selected) {
            canvas.fill({markerX, markerY, kMarkerSize, kMarkerSize}, theme.accent, 1.f);
        }

        float const        labelX = markerX + kMarkerSize + kLabelGap;
        float const        room   = std::max(row.right() - labelX - kRowPadX, 0.f);
        Rgba const         color  = disabledItem ? theme.textMuted.withAlpha(kDisabledAlpha) : theme.text;
        std::string const& label  = mItems[index].label.empty() ? mItems[index].value : mItems[index].label;
        drawFittedText(canvas, label, {labelX, row.y + (row.height - lineHeight) / 2.f}, room, scale, color);
    }
}

void Select::activate() {
    if (disabled()) {
        return;
    }
    setOpen(!mOpen);
}

void Select::activateAt(Point point) {
    if (disabled()) {
        return;
    }
    if (!mOpen) {
        setOpen(true);
        return;
    }
    int const index = itemAt(point);
    if (index >= 0) {
        if (!mItems[static_cast<std::size_t>(index)].disabled) {
            select(index);
        }
        return;
    }
    setOpen(false);
}

bool Select::key(KeyStroke const& stroke) {
    if (disabled()) {
        return false;
    }
    switch (stroke.key) {
    case Key::Up:
    case Key::Down:
        if (!mOpen) {
            setOpen(true);
        } else {
            moveHighlight(stroke.key == Key::Down ? 1 : -1);
        }
        return true;
    case Key::Enter:
    case Key::Space:
        if (!mOpen) {
            setOpen(true);
            return true;
        }
        if (mHighlight >= 0) {
            select(mHighlight);
        }
        return true;
    case Key::Escape:
        if (mOpen) {
            setOpen(false);
            return true;
        }
        return false;
    default:
        return false;
    }
}

void Select::collectOverlays(std::vector<Widget*>& out) {
    if (mOpen) {
        out.push_back(this);
    }
}

Widget* Select::overlayHitTest(Point) { return mOpen ? this : nullptr; }

void Select::dismissOverlay() { setOpen(false); }

} // namespace bul

#include "bul/widget/Widget.h"

#include "bul/StyleSheet.h"
#include "bul/Theme.h"

#include <algorithm>
#include <utility>

namespace bul {

namespace {

/// One axis of the size constraint: a fixed value wins, then the maximum, then
/// the minimum (0 means "unset").
[[nodiscard]] float clampAxis(float value, float minimum, float maximum, float fixed) noexcept {
    if (fixed > 0.f) {
        return fixed;
    }
    if (maximum > 0.f && value > maximum) {
        value = maximum;
    }
    if (minimum > 0.f && value < minimum) {
        value = minimum;
    }
    return value;
}

} // namespace

void Widget::paint(Canvas& canvas, Theme const& theme) const { paintContent(canvas, resolvedTheme(theme)); }

void Widget::paintOverlay(Canvas& canvas, Theme const& theme) const {
    paintOverlayContent(canvas, resolvedTheme(theme));
}

Widget& Widget::style(Style value) {
    mStyle = std::move(value);
    return *this;
}

Theme Widget::resolvedTheme(Theme const& theme) const {
    Theme       resolved  = theme;
    Style const fromSheet = styleSheet().styleFor(typeName(), mClassName, mId);
    if (!fromSheet.empty()) {
        resolved = applyStyle(resolved, fromSheet);
    }
    if (!mStyle.empty()) {
        resolved = applyStyle(resolved, mStyle);
    }
    return resolved;
}

Widget& Widget::fixedSize(float width, float height) noexcept {
    mFixedWidth  = width;
    mFixedHeight = height;
    return *this;
}

Widget& Widget::fixedWidth(float value) noexcept {
    mFixedWidth = value;
    return *this;
}

Widget& Widget::fixedHeight(float value) noexcept {
    mFixedHeight = value;
    return *this;
}

Widget& Widget::minSize(float width, float height) noexcept {
    mMinWidth  = width;
    mMinHeight = height;
    return *this;
}

Widget& Widget::maxSize(float width, float height) noexcept {
    mMaxWidth  = width;
    mMaxHeight = height;
    return *this;
}

Size Widget::measure(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    Size const content = measureContent(availableWidth, theme, canvas);
    float      width   = content.width - mMargin.horizontal();
    float      height  = content.height - mMargin.vertical();
    width              = clampAxis(width, mMinWidth, mMaxWidth, mFixedWidth);
    height             = clampAxis(height, mMinHeight, mMaxHeight, mFixedHeight);
    return {width + mMargin.horizontal(), height + mMargin.vertical()};
}

void Widget::layout(Rect const& outer, Theme const&, Canvas const&) {
    mBounds = contentRect(outer);
    if (mFixedWidth > 0.f) {
        mBounds.width = mFixedWidth;
    }
    if (mFixedHeight > 0.f) {
        mBounds.height = mFixedHeight;
    }
}

Widget* Widget::hitTest(Point point) { return mBounds.contains(point) ? this : nullptr; }

Widget& Widget::onClick(std::function<void()> callback) {
    mOnClick = std::move(callback);
    return *this;
}

Widget& Widget::onFocus(std::function<void()> callback) {
    mOnFocus = std::move(callback);
    return *this;
}

Widget& Widget::onBlur(std::function<void()> callback) {
    mOnBlur = std::move(callback);
    return *this;
}

Widget& Widget::onHover(std::function<void(bool)> callback) {
    mOnHover = std::move(callback);
    return *this;
}

Widget& Widget::className(std::string value) {
    mClassName = std::move(value);
    return *this;
}

Widget& Widget::id(std::string value) {
    mId = std::move(value);
    return *this;
}

void Widget::notifyClick(Point point) {
    activateAt(point);
    if (mOnClick) {
        mOnClick();
    }
}

void Widget::notifyFocus(bool focused) {
    setFocused(focused);
    if (focused) {
        if (mOnFocus) {
            mOnFocus();
        }
    } else if (mOnBlur) {
        mOnBlur();
    }
}

void Widget::notifyHover(bool hovered) {
    setHovered(hovered);
    if (mOnHover) {
        mOnHover(hovered);
    }
}

} // namespace bul

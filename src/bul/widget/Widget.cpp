#include "bul/widget/Widget.h"

#include "bul/AlphaCanvas.h"
#include "bul/FormatCanvas.h"
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

void Widget::paint(Canvas& canvas, Theme const& theme) const {
    Theme const resolved = resolvedTheme(theme);
    if (!resolved.visible || !resolved.displayed) {
        return;
    }
    AlphaCanvas  alpha{canvas, resolved.opacity};
    Canvas&      faded = resolved.opacity < 1.f ? static_cast<Canvas&>(alpha) : canvas;
    FormatCanvas formatted{faded, resolved.bold, resolved.italic, resolved.obfuscated};
    Canvas&      target = format::active(resolved.bold, resolved.italic, resolved.obfuscated)
                             ? static_cast<Canvas&>(formatted)
                             : faded;
    paintContent(target, resolved);
}

void Widget::paintOverlay(Canvas& canvas, Theme const& theme) const {
    Theme const resolved = resolvedTheme(theme);
    if (!resolved.visible || !resolved.displayed) {
        return;
    }
    AlphaCanvas  alpha{canvas, resolved.opacity};
    Canvas&      faded = resolved.opacity < 1.f ? static_cast<Canvas&>(alpha) : canvas;
    FormatCanvas formatted{faded, resolved.bold, resolved.italic, resolved.obfuscated};
    Canvas&      target = format::active(resolved.bold, resolved.italic, resolved.obfuscated)
                             ? static_cast<Canvas&>(formatted)
                             : faded;
    paintOverlayContent(target, resolved);
}

bool Widget::displayed(Theme const&) const {
    Style const style = resolvedStyle();
    return !style.display || *style.display != Display::None;
}

void Widget::paintBox(Canvas& canvas, Rect const& rect, float radius, Rgba color, float alpha, float border) {
    if (border <= 0.f) {
        if (radius > 0.f) {
            canvas.fillRounded(rect, radius, color, alpha);
        } else {
            canvas.fill(rect, color, alpha);
        }
        return;
    }
    // The stroke is drawn inside the box, so the fill has to stop where the
    // stroke starts: an inset rectangle with a radius shrunk by the same amount
    // leaves no sliver of colour around the border.
    Rect const  inner{rect.x + border, rect.y + border, rect.width - 2.f * border, rect.height - 2.f * border};
    float const innerRadius = std::max(radius - border, 0.f);
    if (innerRadius > 0.f) {
        canvas.fillRounded(inner, innerRadius, color, alpha);
    } else {
        canvas.fill(inner, color, alpha);
    }
}

void Widget::paintBorder(
    Canvas&     canvas,
    Rect const& rect,
    float       radius,
    Rgba        color,
    float       thickness,
    float       alpha
) {
    if (radius > 0.f) {
        canvas.outlineRounded(rect, radius, color.withAlpha(color.a * alpha), thickness);
    } else {
        canvas.outline(rect, color.withAlpha(color.a * alpha), thickness);
    }
}

Widget& Widget::style(Style value) {
    mStyle = std::move(value);
    return *this;
}

Theme Widget::resolvedTheme(Theme const& theme) const {
    Theme       resolved  = theme;
    Style const fromSheet = styleSheet().styleFor(*this);
    if (!fromSheet.empty()) {
        resolved = applyStyle(resolved, fromSheet);
    }
    if (!mStyle.empty()) {
        resolved = applyStyle(resolved, mStyle);
    }
    return resolved;
}

Style Widget::resolvedStyle() const {
    Style merged = styleSheet().styleFor(*this);
    if (!mStyle.empty()) {
        merged.overlaidBy(mStyle);
    }
    return merged;
}

Widget& Widget::attr(std::string name, std::string value) {
    mAttributes.emplace_back(std::move(name), std::move(value));
    return *this;
}

std::optional<std::string> Widget::attribute(std::string_view name) const {
    for (auto const& [key, value] : mAttributes) {
        if (key == name) {
            return value;
        }
    }
    if (name == "type") {
        return std::string{typeName()};
    }
    if (name == "class") {
        return mClassName;
    }
    if (name == "id") {
        return mId;
    }
    return std::nullopt;
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
    // Resolve here so a style that changes a size (padding, font-size) also
    // changes the widget's measured size, not just its painting.
    Theme const resolved = resolvedTheme(theme);
    Style const style    = resolvedStyle();
    float const baseFont = resolved.fontSize * resolved.fontScale;
    mEffectiveMargin     = style.margin ? style.margin->resolve(baseFont) : mMargin;

    Size const content = measureContent(availableWidth, resolved, canvas);
    float      width   = content.width - mMargin.horizontal();
    float      height  = content.height - mMargin.vertical();

    // Style sizes win over the widget's own constraints.
    float fixedW = mFixedWidth;
    float minW   = mMinWidth;
    float maxW   = mMaxWidth;
    float fixedH = mFixedHeight;
    float minH   = mMinHeight;
    float maxH   = mMaxHeight;
    if (style.width) {
        fixedW = style.width->resolve(baseFont);
    }
    if (style.height) {
        fixedH = style.height->resolve(baseFont);
    }
    if (style.minWidth) {
        minW = style.minWidth->resolve(baseFont);
    }
    if (style.minHeight) {
        minH = style.minHeight->resolve(baseFont);
    }
    if (style.maxWidth) {
        maxW = style.maxWidth->resolve(baseFont);
    }
    if (style.maxHeight) {
        maxH = style.maxHeight->resolve(baseFont);
    }

    width  = clampAxis(width, minW, maxW, fixedW);
    height = clampAxis(height, minH, maxH, fixedH);
    return {width + mEffectiveMargin.horizontal(), height + mEffectiveMargin.vertical()};
}

void Widget::layout(Rect const& outer, Theme const& theme, Canvas const&) {
    mBounds         = contentRect(outer);
    Style const style = resolvedStyle();
    float const baseFont = theme.fontSize * theme.fontScale;
    if (style.width) {
        mBounds.width = style.width->resolve(baseFont);
    } else if (mFixedWidth > 0.f) {
        mBounds.width = mFixedWidth;
    }
    if (style.height) {
        mBounds.height = style.height->resolve(baseFont);
    } else if (mFixedHeight > 0.f) {
        mBounds.height = mFixedHeight;
    }
}

Widget* Widget::hitTest(Point point) { return mBounds.contains(point) ? this : nullptr; }

bool Widget::state(std::string_view name) const {
    if (name == "hover") {
        return mHoveredState;
    }
    if (name == "focus") {
        return mFocusedState;
    }
    return false;
}

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
    mFocusedState = focused;
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
    mHoveredState = hovered;
    setHovered(hovered);
    if (mOnHover) {
        mOnHover(hovered);
    }
}

} // namespace bul
#pragma once

#include "bul/Canvas.h"
#include "bul/Geometry.h"

namespace bul {

struct Theme;

/// A node of the widget tree.
///
/// The tree is retained mode and layout is top-down, like Qt Quick: a parent
/// asks each child for its desired size, then arranges the children inside its
/// own bounds. Widgets only paint through `Canvas`, so the same tree can be
/// driven by the game, a test recorder, or an offline previewer.
class Widget {
public:
    Widget()                         = default;
    Widget(Widget const&)            = delete;
    Widget& operator=(Widget const&) = delete;
    virtual ~Widget()                = default;

    Widget& margin(float all) noexcept {
        mMargin = {all, all, all, all};
        return *this;
    }
    Widget& margin(float horizontal, float vertical) noexcept {
        mMargin = {horizontal, vertical, horizontal, vertical};
        return *this;
    }

    [[nodiscard]] Rect const&   bounds() const noexcept { return mBounds; }
    [[nodiscard]] Insets const& margin() const noexcept { return mMargin; }

    /// Desired size including this widget's margin.
    [[nodiscard]] virtual Size measure(float availableWidth, Theme const&, Canvas const&) const = 0;

    /// Assigns bounds to this widget and its children.
    virtual void layout(Rect const& outer, Theme const&, Canvas const&);

    virtual void paint(Canvas&, Theme const&) const = 0;

    /// Deepest widget under `point`, or nullptr.
    [[nodiscard]] virtual Widget* hitTest(Point point);

    /// Hover feedback; the host clears it every time the pointer moves.
    virtual void setHovered(bool) {}

    /// Pointer click on an interactive widget.
    virtual void activate() {}

    [[nodiscard]] virtual bool interactive() const noexcept { return false; }

protected:
    [[nodiscard]] Rect contentRect(Rect const& outer) const noexcept {
        return {
            outer.x + mMargin.left,
            outer.y + mMargin.top,
            outer.width - mMargin.horizontal(),
            outer.height - mMargin.vertical()
        };
    }

    Rect   mBounds{};
    Insets mMargin{};
};

} // namespace bul

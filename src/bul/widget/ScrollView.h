#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "bul/Layout.h"
#include "bul/widget/Widget.h"

namespace bul::spec {
struct ScrollView;
}

namespace bul {

/// A viewport over a taller content widget: the content is laid out at its
/// measured height, shifted by `offset`, clipped to the viewport, and a scroll
/// bar shows where the view is when the content does not fit.
///
/// Scrolling is by wheel or by dragging the bar; `scrollTo` / `scrollBy` are the
/// programmatic entry points.
class BUL_API ScrollView final : public Widget {
public:
    [[nodiscard]] std::string_view typeName() const noexcept override { return "ScrollView"; }

    /// Applies a spec node; the spec layer calls this so the mapping lives here.
    ScrollView& apply(spec::ScrollView const& node);

    /// The single content, usually a `Stack`.
    ScrollView& content(std::unique_ptr<Widget> child);
    /// Height of the viewport; 0 (the default) means "as tall as the content" --
    /// a scroll view without a limit simply does not scroll.
    ScrollView& viewport(float value) noexcept {
        mViewport = value;
        return *this;
    }
    /// Draws the bar on the right edge; on by default.
    ScrollView& scrollbar(bool value) noexcept {
        mScrollbar = value;
        return *this;
    }
    [[nodiscard]] bool  scrollbar() const noexcept { return mScrollbar; }
    [[nodiscard]] float viewport() const noexcept { return mViewport; }

    /// Distance the wheel scrolls per notch (one notch of the wheel).
    ScrollView& wheelStep(float value) noexcept {
        mWheelStep = value;
        return *this;
    }

    ScrollView& scrollTo(float value);
    ScrollView& scrollBy(float delta);

    [[nodiscard]] float offset() const noexcept { return mOffset; }
    [[nodiscard]] float contentHeight() const noexcept { return mContentHeight; }
    [[nodiscard]] bool  scrollable() const noexcept { return mContentHeight > mBounds.height; }

    [[nodiscard]] Size    measureContent(float availableWidth, Theme const&, Canvas const&) const override;
    void                  layout(Rect const& outer, Theme const&, Canvas const&) override;
    void                  paintContent(Canvas&, Theme const&) const override;
    [[nodiscard]] Widget* hitTest(Point point) override;
    [[nodiscard]] bool    wheel(Point point, float delta) override;
    [[nodiscard]] bool    press(Point point) override;
    void                  drag(Point point) override;
    void                  release() override;
    void                  setControlState(ControlState const& state) override;
    void                  collectFocusables(std::vector<Widget*>& out) override;
    void                  collectOverlays(std::vector<Widget*>& out) override;

private:
    /// The bar lane on the right edge.
    [[nodiscard]] Rect barRect() const noexcept;
    /// The thumb inside that lane.
    [[nodiscard]] layout::Thumb barThumb() const noexcept;
    /// Maps the pointer to an offset while the thumb is dragged.
    void applyDrag(Point point);

    std::unique_ptr<Widget> mContent;
    float                   mViewport{};
    float                   mOffset{};
    mutable float           mContentHeight{};
    bool                    mScrollbar{true};
    float                   mWheelStep{24.f};
    bool                    mDragging{};
    float                   mDragGrab{};
};

} // namespace bul

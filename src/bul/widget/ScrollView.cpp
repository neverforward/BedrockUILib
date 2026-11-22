#include "bul/widget/ScrollView.h"

#include <algorithm>
#include <utility>

#include "bul/Theme.h"

namespace bul {

namespace {

constexpr float kBarWidth = 4.f;
/// Gap between the content and the bar lane.
constexpr float kBarPadding = 4.f;
constexpr float kThumbMin   = 12.f;

} // namespace

ScrollView& ScrollView::content(std::unique_ptr<Widget> child) {
    mContent = std::move(child);
    return *this;
}

ScrollView& ScrollView::scrollTo(float value) {
    // Before the first layout there is no content height to clamp against, so the
    // request is kept and clamped when the content is laid out.
    mOffset =
        mContentHeight > 0.f ? layout::clampScroll(value, {mContentHeight, mBounds.height}) : std::max(value, 0.f);
    return *this;
}

ScrollView& ScrollView::scrollBy(float delta) { return scrollTo(mOffset + delta); }

Size ScrollView::measureContent(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    if (!mContent) {
        return {mMargin.horizontal(), mMargin.vertical()};
    }
    Size const size   = mContent->measure(availableWidth, theme, canvas);
    float      height = size.height;
    if (mViewport > 0.f) {
        height = std::min(height, mViewport);
    }
    return {size.width + mMargin.horizontal(), height + mMargin.vertical()};
}

void ScrollView::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    mBounds = contentRect(outer);
    if (!mContent) {
        return;
    }

    Size const contentSize = mContent->measure(mBounds.width, theme, canvas);
    mContentHeight         = contentSize.height;
    mOffset                = layout::clampScroll(mOffset, {mContentHeight, mBounds.height});

    // The content stops short of the bar lane, so text and controls never sit on
    // the scrollbar.
    bool const  showBar      = mScrollbar && mContentHeight > mBounds.height;
    float const lane         = showBar ? kBarWidth + kBarPadding : 0.f;
    float const contentWidth = std::max(mBounds.width - lane, 0.f);
    mContent->layout({mBounds.x, mBounds.y - mOffset, contentWidth, mContentHeight}, theme, canvas);
}

Rect ScrollView::barRect() const noexcept {
    return {mBounds.right() - kBarWidth, mBounds.y, kBarWidth, mBounds.height};
}

layout::Thumb ScrollView::barThumb() const noexcept {
    return layout::scrollThumb({mContentHeight, mBounds.height}, mOffset, mBounds.height, kThumbMin);
}

void ScrollView::paintContent(Canvas& canvas, Theme const& theme) const {
    if (!mContent) {
        return;
    }

    // Only the part inside the viewport is drawn: the content itself is laid out
    // taller than the viewport and shifted by the offset.
    // Flush what earlier siblings recorded before narrowing the clip: the engine's
    // text batch is drawn at flush time with the *current* clip, so pending text
    // from above the viewport would otherwise be cut away by it.
    canvas.flush();
    canvas.pushClip(mBounds);
    mContent->paint(canvas, theme);
    // The engine draws text in a deferred batch, so it has to be submitted while
    // the clip is still active; otherwise the edge rows would paint over the
    // viewport after popClip.
    canvas.flush();
    canvas.popClip();

    if (!mScrollbar || !scrollable()) {
        return;
    }
    Rect const          track = barRect();
    layout::Thumb const thumb = barThumb();
    canvas.fill(track, theme.button.withAlpha(0.55f), 1.f);
    canvas.fill({track.x, track.y + thumb.start, kBarWidth, thumb.length}, theme.textMuted, 1.f);
}

Widget* ScrollView::hitTest(Point point) {
    if (!mBounds.contains(point)) {
        return nullptr;
    }
    // The bar lane claims the pointer before the content, so a press on the thumb
    // starts a drag instead of reaching a control underneath.
    if (mScrollbar && scrollable() && point.x >= mBounds.right() - kBarWidth) {
        return this;
    }
    if (mContent) {
        if (Widget* hit = mContent->hitTest(point)) {
            return hit;
        }
    }
    return this;
}

bool ScrollView::wheel(Point point, float delta) {
    if (!mBounds.contains(point)) {
        return false;
    }
    if (mContent && mContent->wheel(point, delta)) {
        return true;
    }
    if (!scrollable()) {
        return false;
    }
    scrollBy(delta * mWheelStep);
    return true;
}

bool ScrollView::press(Point point) {
    if (!mScrollbar || !scrollable() || !mBounds.contains(point) || point.x < mBounds.right() - kBarWidth) {
        return false;
    }
    Rect const          track = barRect();
    layout::Thumb const thumb = barThumb();
    Rect const          thumbRect{track.x, track.y + thumb.start, kBarWidth, thumb.length};

    // A press on the thumb keeps the grab point; a press on the lane jumps to it.
    mDragGrab = thumbRect.contains(point) ? point.y - thumbRect.y : thumb.length * 0.5f;
    mDragging = true;
    applyDrag(point);
    return true;
}

void ScrollView::drag(Point point) {
    if (mDragging) {
        applyDrag(point);
    }
}

void ScrollView::release() { mDragging = false; }

void ScrollView::applyDrag(Point point) {
    float const travel = mBounds.height - barThumb().length;
    if (travel <= 0.f) {
        return;
    }
    float const start = std::clamp(point.y - mBounds.y - mDragGrab, 0.f, travel);
    float const max   = std::max(mContentHeight - mBounds.height, 0.f);
    mOffset           = max * (start / travel);
}

void ScrollView::setControlState(ControlState const& state) {
    if (mContent) {
        mContent->setControlState(state);
    }
}

void ScrollView::collectFocusables(std::vector<Widget*>& out) {
    if (mContent) {
        mContent->collectFocusables(out);
    }
}

void ScrollView::collectOverlays(std::vector<Widget*>& out) {
    if (mContent) {
        mContent->collectOverlays(out);
    }
}

} // namespace bul

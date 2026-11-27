#include "bul/Spec.h"

#include "bul/widget/Stack.h"

#include <algorithm>
#include <utility>

#include "bul/Layout.h"
#include "bul/Theme.h"

namespace bul {

Stack::Stack(Direction direction) : mDirection(direction) {}

Stack& Stack::add(std::unique_ptr<Widget> child) { return add(std::move(child), 0.f); }

Stack& Stack::add(std::unique_ptr<Widget> child, float stretch) {
    mEntries.push_back(Entry{std::move(child), stretch});
    return *this;
}

Size Stack::measureContent(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    float const gap   = mGap >= 0.f ? mGap : theme.gap;
    float       main  = 0.f;
    float       cross = 0.f;
    for (std::size_t i = 0; i < mEntries.size(); ++i) {
        Size const size = mEntries[i].child->measure(availableWidth, theme, canvas);
        if (mDirection == Direction::Vertical) {
            main  += size.height;
            cross  = std::max(cross, size.width);
        } else {
            main  += size.width;
            cross  = std::max(cross, size.height);
        }
        if (i + 1 < mEntries.size()) {
            main += gap;
        }
    }
    if (mDirection == Direction::Vertical) {
        return {cross + mMargin.horizontal(), main + mMargin.vertical()};
    }
    return {main + mMargin.horizontal(), cross + mMargin.vertical()};
}

void Stack::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    mBounds         = contentRect(outer);
    float const gap = mGap >= 0.f ? mGap : theme.gap;
    if (mEntries.empty()) {
        return;
    }

    bool const vertical = mDirection == Direction::Vertical;

    std::vector<layout::Item> items;
    items.reserve(mEntries.size());
    for (auto const& entry : mEntries) {
        items.push_back(layout::Item{entry.child->measure(mBounds.width, theme, canvas), entry.stretch});
    }

    // A column stretches its children across the width (the usual full-width
    // row), but each child keeps its own height; a row must not stretch the
    // cross axis, or buttons of different sizes would all grow to the tallest.
    layout::BoxSpec const spec{
        .axis  = vertical ? layout::Axis::Vertical : layout::Axis::Horizontal,
        .gap   = gap,
        .align = vertical ? layout::Align::Stretch : layout::Align::Start,
    };
    std::vector<Rect> const rects = layout::box(mBounds, items, spec);
    for (std::size_t i = 0; i < mEntries.size(); ++i) {
        mEntries[i].child->layout(rects[i], theme, canvas);
    }
}
bool Stack::wheel(Point point, float delta) {
    if (!mBounds.contains(point)) {
        return false;
    }
    for (auto it = mEntries.rbegin(); it != mEntries.rend(); ++it) {
        if (it->child->bounds().contains(point) && it->child->wheel(point, delta)) {
            return true;
        }
    }
    return false;
}
void Stack::paintContent(Canvas& canvas, Theme const& theme) const {
    for (auto const& entry : mEntries) {
        entry.child->paint(canvas, theme);
    }
}

Widget* Stack::hitTest(Point point) {
    for (auto it = mEntries.rbegin(); it != mEntries.rend(); ++it) {
        if (Widget* hit = it->child->hitTest(point)) {
            return hit;
        }
    }
    return mBounds.contains(point) ? this : nullptr;
}

void Stack::setControlState(ControlState const& state) {
    for (auto& entry : mEntries) {
        entry.child->setControlState(state);
    }
}

void Stack::collectFocusables(std::vector<Widget*>& out) {
    for (auto& entry : mEntries) {
        entry.child->collectFocusables(out);
    }
}

void Stack::collectOverlays(std::vector<Widget*>& out) {
    for (auto& entry : mEntries) {
        entry.child->collectOverlays(out);
    }
}


Stack& Stack::apply(spec::Stack const& node) {
    if (node.gap >= 0.f) {
        gap(node.gap);
    }
    for (auto const& child : node.children) {
        if (child.node) {
            add(child.node->build(), child.stretch);
        }
    }
    return *this;
}
} // namespace bul

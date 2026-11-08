#include "bul/widget/Stack.h"

#include <algorithm>
#include <utility>

#include "bul/Theme.h"

namespace bul {

Stack::Stack(Direction direction) : mDirection(direction) {}

Stack& Stack::add(std::unique_ptr<Widget> child) { return add(std::move(child), 0.f); }

Stack& Stack::add(std::unique_ptr<Widget> child, float stretch) {
    mEntries.push_back(Entry{std::move(child), stretch});
    return *this;
}

Size Stack::measure(float availableWidth, Theme const& theme, Canvas const& canvas) const {
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

    bool const        vertical = mDirection == Direction::Vertical;
    std::vector<Size> sizes;
    sizes.reserve(mEntries.size());

    float fixedMain = 0.f;
    float stretch   = 0.f;
    for (std::size_t i = 0; i < mEntries.size(); ++i) {
        Size const size = mEntries[i].child->measure(mBounds.width, theme, canvas);
        sizes.push_back(size);
        if (mEntries[i].stretch > 0.f) {
            stretch += mEntries[i].stretch;
        } else {
            fixedMain += vertical ? size.height : size.width;
        }
    }

    // A stretched child gets a share of what the fixed children leave -- not its
    // measured size plus a remainder. That is what keeps a row of weighted
    // columns inside the row even when one child's measurement already claimed
    // the full available width (a text field with a long value does exactly
    // that), which used to push the last column out of the window.
    float const available = vertical ? mBounds.height : mBounds.width;
    float const gaps      = gap * static_cast<float>(mEntries.size() - 1);
    float const leftover  = std::max(available - gaps - fixedMain, 0.f);

    float cursor = vertical ? mBounds.y : mBounds.x;
    for (std::size_t i = 0; i < mEntries.size(); ++i) {
        float width  = sizes[i].width;
        float height = sizes[i].height;
        if (stretch > 0.f && mEntries[i].stretch > 0.f) {
            float const share = leftover * (mEntries[i].stretch / stretch);
            if (vertical) {
                height = share;
            } else {
                width = share;
            }
        }
        if (vertical) {
            // A column stretches its children across the width (the usual
            // full-width row), but each child keeps its own height.
            mEntries[i].child->layout({mBounds.x, cursor, mBounds.width, height}, theme, canvas);
            cursor += height + gap;
        } else {
            // A row must NOT stretch the cross axis, otherwise children with
            // different heights (button sizes) would all be drawn at the height
            // of the tallest one.
            mEntries[i].child->layout({cursor, mBounds.y, width, height}, theme, canvas);
            cursor += width + gap;
        }
    }
}

void Stack::paint(Canvas& canvas, Theme const& theme) const {
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

} // namespace bul

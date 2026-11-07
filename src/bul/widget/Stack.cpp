#include "bul/widget/Stack.h"

#include <algorithm>
#include <utility>

#include "bul/Theme.h"

namespace bul {

Stack::Stack(Direction direction) : mDirection(direction) {}

Stack& Stack::add(std::unique_ptr<Widget> child) {
    mChildren.push_back(std::move(child));
    return *this;
}

Size Stack::measure(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    float const gap = mGap >= 0.f ? mGap : theme.gap;
    float       main{};
    float       cross{};
    for (std::size_t i = 0; i < mChildren.size(); ++i) {
        Size const size = mChildren[i]->measure(availableWidth, theme, canvas);
        if (mDirection == Direction::Vertical) {
            main  += size.height;
            cross  = std::max(cross, size.width);
        } else {
            main  += size.width;
            cross  = std::max(cross, size.height);
        }
        if (i + 1 < mChildren.size()) {
            main += gap;
        }
    }
    if (mDirection == Direction::Vertical) {
        return {cross + mMargin.horizontal(), main + mMargin.vertical()};
    }
    return {main + mMargin.horizontal(), cross + mMargin.vertical()};
}

void Stack::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    mBounds            = contentRect(outer);
    float const gap    = mGap >= 0.f ? mGap : theme.gap;
    float       cursor = mDirection == Direction::Vertical ? mBounds.y : mBounds.x;
    for (auto& child : mChildren) {
        Size const size = child->measure(mBounds.width, theme, canvas);
        if (mDirection == Direction::Vertical) {
            child->layout({mBounds.x, cursor, mBounds.width, size.height}, theme, canvas);
            cursor += size.height + gap;
        } else {
            child->layout({cursor, mBounds.y, size.width, mBounds.height}, theme, canvas);
            cursor += size.width + gap;
        }
    }
}

void Stack::paint(Canvas& canvas, Theme const& theme) const {
    for (auto const& child : mChildren) {
        child->paint(canvas, theme);
    }
}

Widget* Stack::hitTest(Point point) {
    for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
        if (Widget* hit = (*it)->hitTest(point)) {
            return hit;
        }
    }
    return mBounds.contains(point) ? this : nullptr;
}

} // namespace bul

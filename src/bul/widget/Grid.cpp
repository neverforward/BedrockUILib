#include "bul/widget/Grid.h"

#include <algorithm>
#include <utility>

#include "bul/Layout.h"
#include "bul/Theme.h"

namespace bul {

Grid::Grid(std::size_t columns) : mColumns(columns == 0 ? 1 : columns) {}

Grid& Grid::add(std::unique_ptr<Widget> child) {
    mChildren.push_back(std::move(child));
    return *this;
}

Size Grid::measureContent(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    if (mChildren.empty()) {
        return {mMargin.horizontal(), mMargin.vertical()};
    }
    float const gap   = mGap >= 0.f ? mGap : theme.gap;
    float const slot  = availableWidth > 0.f ? availableWidth / static_cast<float>(mColumns) : availableWidth;
    float       cellW = 0.f;
    float       cellH = 0.f;
    for (auto const& child : mChildren) {
        Size const size = child->measure(slot, theme, canvas);
        cellW           = std::max(cellW, size.width);
        cellH           = std::max(cellH, size.height);
    }
    if (availableWidth > 0.f) {
        cellW = std::min(cellW, std::max(slot, 1.f));
    }
    std::size_t const rows = (mChildren.size() + mColumns - 1) / mColumns;
    float const       width =
        cellW * static_cast<float>(mColumns) + gap * static_cast<float>(mColumns > 1 ? mColumns - 1 : 0);
    float const height = cellH * static_cast<float>(rows) + gap * static_cast<float>(rows - 1);
    return {width + mMargin.horizontal(), height + mMargin.vertical()};
}

void Grid::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    mBounds         = contentRect(outer);
    float const gap = mGap >= 0.f ? mGap : theme.gap;
    if (mChildren.empty()) {
        return;
    }

    std::vector<Size> sizes;
    sizes.reserve(mChildren.size());
    for (auto const& child : mChildren) {
        sizes.push_back(child->measure(mBounds.width / static_cast<float>(mColumns), theme, canvas));
    }

    layout::GridSpec const  spec{.columns = mColumns, .gap = gap};
    std::vector<Rect> const cells = layout::grid(mBounds, sizes, spec);
    for (std::size_t i = 0; i < mChildren.size(); ++i) {
        mChildren[i]->layout(cells[i], theme, canvas);
    }
}

bool Grid::wheel(Point point, float delta) {
    if (!mBounds.contains(point)) {
        return false;
    }
    for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
        if ((*it)->bounds().contains(point) && (*it)->wheel(point, delta)) {
            return true;
        }
    }
    return false;
}
void Grid::paintContent(Canvas& canvas, Theme const& theme) const {
    for (auto const& child : mChildren) {
        child->paint(canvas, theme);
    }
}

Widget* Grid::hitTest(Point point) {
    for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
        if (Widget* hit = (*it)->hitTest(point)) {
            return hit;
        }
    }
    return mBounds.contains(point) ? this : nullptr;
}

void Grid::setControlState(ControlState const& state) {
    for (auto& child : mChildren) {
        child->setControlState(state);
    }
}

void Grid::collectFocusables(std::vector<Widget*>& out) {
    for (auto& child : mChildren) {
        child->collectFocusables(out);
    }
}

void Grid::collectOverlays(std::vector<Widget*>& out) {
    for (auto& child : mChildren) {
        child->collectOverlays(out);
    }
}

} // namespace bul

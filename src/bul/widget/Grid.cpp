#include "bul/Spec.h"

#include "bul/widget/Grid.h"

#include <algorithm>
#include <utility>

#include "bul/Layout.h"
#include "bul/Theme.h"

namespace bul {

Grid::Grid(std::size_t columns) : mColumns(columns == 0 ? 1 : columns) {}

Grid& Grid::add(std::unique_ptr<Widget> child) {
    child->setParent(this);
    mChildren.push_back(std::move(child));
    return *this;
}

Size Grid::measureContent(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    Insets const padding = theme.padding.value_or(Insets{});
    std::vector<Widget*> shown;
    for (auto const& child : mChildren) {
        if (child->displayed(theme)) {
            shown.push_back(child.get());
        }
    }
    if (shown.empty()) {
        return {padding.horizontal() + mMargin.horizontal(), padding.vertical() + mMargin.vertical()};
    }
    float const gap   = mGap >= 0.f ? mGap : theme.gap;
    float const inner = std::max(availableWidth - padding.horizontal(), 0.f);
    float const slot  = inner > 0.f ? inner / static_cast<float>(mColumns) : inner;
    float       cellW = 0.f;
    float       cellH = 0.f;
    for (Widget* child : shown) {
        Size const size = child->measure(slot, theme, canvas);
        cellW           = std::max(cellW, size.width);
        cellH           = std::max(cellH, size.height);
    }
    if (inner > 0.f) {
        cellW = std::min(cellW, std::max(slot, 1.f));
    }
    std::size_t const rows = (shown.size() + mColumns - 1) / mColumns;
    float const       width =
        cellW * static_cast<float>(mColumns) + gap * static_cast<float>(mColumns > 1 ? mColumns - 1 : 0);
    float const height = cellH * static_cast<float>(rows) + gap * static_cast<float>(rows - 1);
    return {width + padding.horizontal() + mMargin.horizontal(), height + padding.vertical() + mMargin.vertical()};
}

void Grid::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    mBounds              = contentRect(outer);
    Insets const padding = theme.padding.value_or(Insets{});
    float const  gap     = mGap >= 0.f ? mGap : theme.gap;
    std::vector<Widget*> shown;
    for (auto const& child : mChildren) {
        if (child->displayed(theme)) {
            shown.push_back(child.get());
        }
    }
    if (shown.empty()) {
        return;
    }

    Rect const inner{
        mBounds.x + padding.left,
        mBounds.y + padding.top,
        std::max(mBounds.width - padding.horizontal(), 0.f),
        std::max(mBounds.height - padding.vertical(), 0.f),
    };

    std::vector<Size> sizes;
    sizes.reserve(mChildren.size());
    for (Widget* child : shown) {
        sizes.push_back(child->measure(inner.width / static_cast<float>(mColumns), theme, canvas));
    }

    layout::GridSpec const  spec{.columns = mColumns, .gap = gap};
    std::vector<Rect> const cells = layout::grid(inner, sizes, spec);
    for (std::size_t i = 0; i < shown.size(); ++i) {
        shown[i]->layout(cells[i], theme, canvas);
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
    if (theme.background) {
        Widget::paintBox(canvas, mBounds, theme.borderRadius, *theme.background, 1.f, theme.borderWidth.value_or(0.f));
    }
    if (theme.border) {
        Widget::paintBorder(canvas, mBounds, theme.borderRadius, *theme.border, theme.borderWidth.value_or(1.f), 1.f);
    }
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


Grid& Grid::apply(spec::Grid const& node) {
    columns(node.columns);
    if (node.gap >= 0.f) {
        gap(node.gap);
    }
    for (auto const& child : node.children) {
        if (child) {
            add(child->build());
        }
    }
    return *this;
}
std::vector<Widget*> Grid::children() const {
    std::vector<Widget*> out;
    out.reserve(mChildren.size());
    for (auto const& child : mChildren) {
        out.push_back(child.get());
    }
    return out;
}

} // namespace bul

#include "bul/widget/Table.h"

#include <algorithm>
#include <utility>

#include "bul/Theme.h"

namespace bul {

namespace {

/// A rectangle shrunk by an inset on each side, never negative.
[[nodiscard]] Rect inset(Rect const& rect, Insets padding) noexcept {
    return {
        rect.x + padding.left,
        rect.y + padding.top,
        std::max(rect.width - padding.horizontal(), 0.f),
        std::max(rect.height - padding.vertical(), 0.f),
    };
}
/// Bounds minus the padding: the area the tracks are solved in.
[[nodiscard]] Rect paddedBounds(Rect const& bounds, Insets padding) noexcept {
    return {
        bounds.x + padding.left,
        bounds.y + padding.top,
        std::max(bounds.width - padding.horizontal(), 0.f),
        std::max(bounds.height - padding.vertical(), 0.f),
    };
}


/// Natural size of a track: fixed value, or the largest cell along it.
[[nodiscard]] float natural(layout::Track const& track, float largest) noexcept {
    return track.weight > 0.f ? largest : std::max(track.value, 0.f);
}

} // namespace

Table& Table::column(layout::Track track) {
    mColumns.push_back(track);
    return *this;
}

Table& Table::row(layout::Track track) {
    mRows.push_back(track);
    return *this;
}

void Table::ensure(std::size_t row, std::size_t columnValue) {
    while (mColumns.size() <= columnValue) {
        mColumns.push_back(layout::Track{.weight = 1.f});
    }
    while (mRows.size() <= row) {
        mRows.push_back(layout::Track{.weight = 1.f});
    }
    mCells.resize(mRows.size() * mColumns.size());
}

Table& Table::cell(std::size_t row, std::size_t columnValue, std::unique_ptr<Widget> child) {
    ensure(row, columnValue);
    mCells[row * mColumns.size() + columnValue] = std::move(child);
    return *this;
}

Size Table::measureContent(float availableWidth, Theme const& theme, Canvas const& canvas) const {
    if (mColumns.empty() || mRows.empty()) {
        return {mMargin.horizontal(), mMargin.vertical()};
    }

    std::vector<float> widest(mColumns.size(), 0.f);
    std::vector<float> tallest(mRows.size(), 0.f);
    for (std::size_t row = 0; row < mRows.size(); ++row) {
        for (std::size_t column = 0; column < mColumns.size(); ++column) {
            Widget* child = mCells[row * mColumns.size() + column].get();
            if (child == nullptr) {
                continue;
            }
            Size const size = child->measure(availableWidth, theme, canvas);
            widest[column]  = std::max(widest[column], size.width);
            tallest[row]    = std::max(tallest[row], size.height);
        }
    }

    // A track holds its content plus the cell padding, so the solved cell minus
    // that padding is exactly the content the child asked for.
    float width  = mColumnGap * static_cast<float>(mColumns.size() > 1 ? mColumns.size() - 1 : 0);
    float height = mRowGap * static_cast<float>(mRows.size() > 1 ? mRows.size() - 1 : 0);
    for (std::size_t i = 0; i < mColumns.size(); ++i) {
        width += natural(mColumns[i], widest[i]) + mCellPadding.horizontal();
    }
    for (std::size_t i = 0; i < mRows.size(); ++i) {
        height += natural(mRows[i], tallest[i]) + mCellPadding.vertical();
    }
    return {width + mPadding.horizontal() + mMargin.horizontal(), height + mPadding.vertical() + mMargin.vertical()};
}

void Table::layout(Rect const& outer, Theme const& theme, Canvas const& canvas) {
    mBounds = contentRect(outer);
    if (mColumns.empty() || mRows.empty()) {
        return;
    }

    layout::TableSpec const spec{.columnGap = mColumnGap, .rowGap = mRowGap};
    Rect const              inner = paddedBounds(mBounds, mPadding);

    // Fixed tracks grow by their cell padding, so a fixed size keeps meaning the
    // content size; flexible tracks already include it in their share.
    std::vector<layout::Track> columns = mColumns;
    for (auto& track : columns) {
        if (track.weight <= 0.f) {
            track.value += mCellPadding.horizontal();
        }
    }
    std::vector<layout::Track> rows = mRows;
    for (auto& track : rows) {
        if (track.weight <= 0.f) {
            track.value += mCellPadding.vertical();
        }
    }
    std::vector<Rect> const cells = layout::table(inner, columns, rows, spec);
    for (std::size_t row = 0; row < mRows.size(); ++row) {
        for (std::size_t column = 0; column < mColumns.size(); ++column) {
            Widget* child = mCells[row * mColumns.size() + column].get();
            if (child != nullptr) {
                Rect const cell = inset(cells[row * mColumns.size() + column], mCellPadding);
                child->layout(cell, theme, canvas);
            }
        }
    }
}

void Table::paintContent(Canvas& canvas, Theme const& theme) const {
    if (mBorder && !mColumns.empty() && !mRows.empty()) {
        Rgba const color = mBorderColor ? *mBorderColor : theme.panelBorder;
        canvas.outline(mBounds, color, 1.f);

        // Separators sit in the middle of the gap between two tracks, inside the
        // padding so the grid stays clear of the outline.
        layout::TableSpec const spec{.columnGap = mColumnGap, .rowGap = mRowGap};
        Rect const              inner = paddedBounds(mBounds, mPadding);
        std::vector<Rect> const cells = layout::table(inner, mColumns, mRows, spec);
        // The separators run border to border: the padding insets the cells, it
        // does not shorten the grid lines.
        for (std::size_t column = 1; column < mColumns.size(); ++column) {
            float const x = cells[column].x - mColumnGap * 0.5f;
            canvas.fill({x - 0.5f, mBounds.y, 1.f, mBounds.height}, color, 1.f);
        }
        for (std::size_t row = 1; row < mRows.size(); ++row) {
            float const y = cells[row * mColumns.size()].y - mRowGap * 0.5f;
            canvas.fill({mBounds.x, y - 0.5f, mBounds.width, 1.f}, color, 1.f);
        }
    }
    for (auto const& cell : mCells) {
        if (cell) {
            cell->paint(canvas, theme);
        }
    }
}

Widget* Table::hitTest(Point point) {
    for (auto it = mCells.rbegin(); it != mCells.rend(); ++it) {
        if (*it) {
            if (Widget* hit = (*it)->hitTest(point)) {
                return hit;
            }
        }
    }
    return mBounds.contains(point) ? this : nullptr;
}

bool Table::wheel(Point point, float delta) {
    if (!mBounds.contains(point)) {
        return false;
    }
    for (auto it = mCells.rbegin(); it != mCells.rend(); ++it) {
        if (*it && (*it)->bounds().contains(point) && (*it)->wheel(point, delta)) {
            return true;
        }
    }
    return false;
}
void Table::setControlState(ControlState const& state) {
    for (auto& cell : mCells) {
        if (cell) {
            cell->setControlState(state);
        }
    }
}

void Table::collectFocusables(std::vector<Widget*>& out) {
    for (auto& cell : mCells) {
        if (cell) {
            cell->collectFocusables(out);
        }
    }
}

void Table::collectOverlays(std::vector<Widget*>& out) {
    for (auto& cell : mCells) {
        if (cell) {
            cell->collectOverlays(out);
        }
    }
}

} // namespace bul

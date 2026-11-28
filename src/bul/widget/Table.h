#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "bul/Layout.h"
#include "bul/widget/Widget.h"

namespace bul::spec {
struct Table;
}

namespace bul {

/// A table of cells: columns and rows are `layout::Track`s (a fixed size, or a
/// share of the leftover), each cell holds one widget and stretches to fill it.
///
/// Cells are addressed by `(row, column)`; adding one past the declared tracks
/// appends a flexible track, so a table can be built by cells alone.
class BUL_API Table final : public Widget {
public:
    static constexpr std::string_view kTypeName{"Table"};

    [[nodiscard]] std::string_view typeName() const noexcept override { return kTypeName; }

    [[nodiscard]] std::vector<Widget*> children() const override;

    /// Applies a spec node; the spec layer calls this so the mapping lives here.
    Table& apply(spec::Table const& node);

    /// Appends a column track (fixed `value` or flexible `weight`).
    Table& column(layout::Track track);
    Table& row(layout::Track track);
    /// Draws the outer border and the row/column separators; on by default.
    Table& border(bool value) noexcept {
        mBorder = value;
        return *this;
    }
    /// Border colour; Theme::panelBorder when unset.
    Table& borderColor(Rgba value) {
        mBorderColor = value;
        return *this;
    }
    [[nodiscard]] bool border() const noexcept { return mBorder; }

    /// Inset of the whole grid from the border (0 by default): a non-zero value
    /// makes the first and last rows taller than the inner ones, so the content'
    /// own cellPadding is the usual way to keep text off the border.
    Table& padding(float all) noexcept {
        mPadding = {all, all, all, all};
        return *this;
    }
    Table& padding(float horizontal, float vertical) noexcept {
        mPadding = {horizontal, vertical, horizontal, vertical};
        return *this;
    }

    /// Inset of the cell content inside its cell (4 units by default), so text
    /// does not sit on the separator lines.
    Table& cellPadding(float all) noexcept {
        mCellPadding = {all, all, all, all};
        return *this;
    }
    Table& cellPadding(float horizontal, float vertical) noexcept {
        mCellPadding = {horizontal, vertical, horizontal, vertical};
        return *this;
    }

    Table& gap(float columnGap, float rowGap) noexcept {
        mColumnGap = columnGap;
        mRowGap    = rowGap;
        return *this;
    }

    Table& cell(std::size_t row, std::size_t column, std::unique_ptr<Widget> child);

    [[nodiscard]] Size    measureContent(float availableWidth, Theme const&, Canvas const&) const override;
    void                  layout(Rect const& outer, Theme const&, Canvas const&) override;
    void                  paintContent(Canvas&, Theme const&) const override;
    [[nodiscard]] Widget* hitTest(Point point) override;
    [[nodiscard]] bool    wheel(Point point, float delta) override;
    void                  setControlState(ControlState const& state) override;
    void                  collectFocusables(std::vector<Widget*>& out) override;
    void                  collectOverlays(std::vector<Widget*>& out) override;

private:
    void ensure(std::size_t row, std::size_t column);

    std::vector<layout::Track>           mColumns;
    std::vector<layout::Track>           mRows;
    std::vector<std::unique_ptr<Widget>> mCells; // row-major, may hold nullptr
    float                                mColumnGap{0.f};
    float                                mRowGap{0.f};
    Insets                               mPadding{};
    Insets                               mCellPadding{4.f, 4.f, 4.f, 4.f};
    bool                                 mBorder{true};
    std::optional<Rgba>                  mBorderColor{};
};

} // namespace bul

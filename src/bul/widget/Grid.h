#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "bul/widget/Widget.h"

namespace bul {

/// A grid of equal cells, row-major: items go left to right, then wrap.
///
/// The cells are solved by `layout::grid`, so the rows split the height evenly
/// and the grid always fills the rectangle it is given.
class BUL_API Grid final : public Widget {
public:
    [[nodiscard]] std::string_view typeName() const noexcept override { return "Grid"; }

    explicit Grid(std::size_t columns = 2);

    Grid& add(std::unique_ptr<Widget> child);
    Grid& columns(std::size_t value) noexcept {
        mColumns = value == 0 ? 1 : value;
        return *this;
    }
    Grid& gap(float value) noexcept {
        mGap = value;
        return *this;
    }

    [[nodiscard]] Size    measureContent(float availableWidth, Theme const&, Canvas const&) const override;
    void                  layout(Rect const& outer, Theme const&, Canvas const&) override;
    void                  paintContent(Canvas&, Theme const&) const override;
    [[nodiscard]] bool    wheel(Point, float) override;
    [[nodiscard]] Widget* hitTest(Point point) override;
    void                  setControlState(ControlState const& state) override;
    void                  collectFocusables(std::vector<Widget*>& out) override;
    void                  collectOverlays(std::vector<Widget*>& out) override;

private:
    std::size_t                          mColumns{2};
    std::vector<std::unique_ptr<Widget>> mChildren;
    float                                mGap{-1.f}; // < 0: use Theme::gap
};

} // namespace bul

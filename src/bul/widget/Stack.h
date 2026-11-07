#pragma once

#include <memory>
#include <vector>

#include "bul/widget/Widget.h"

namespace bul {

/// Vertical or horizontal stack of children (the "row / column" primitive).
class Stack final : public Widget {
public:
    enum class Direction { Vertical, Horizontal };

    explicit Stack(Direction direction = Direction::Vertical);

    Stack& add(std::unique_ptr<Widget> child);
    Stack& gap(float value) noexcept {
        mGap = value;
        return *this;
    }

    [[nodiscard]] Size    measure(float availableWidth, Theme const&, Canvas const&) const override;
    void                  layout(Rect const& outer, Theme const&, Canvas const&) override;
    void                  paint(Canvas&, Theme const&) const override;
    [[nodiscard]] Widget* hitTest(Point point) override;

private:
    Direction                            mDirection;
    std::vector<std::unique_ptr<Widget>> mChildren;
    float                                mGap{-1.f}; // < 0: use Theme::gap
};

} // namespace bul

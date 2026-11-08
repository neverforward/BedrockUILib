#pragma once

#include <memory>
#include <vector>

#include "bul/widget/Widget.h"

namespace bul {

/// Vertical or horizontal stack of children (the "row / column" primitive).
///
/// A child added with a positive stretch factor grows along the main axis when
/// the stack is given more room than its children need -- the "the input takes
/// the rest of the row" case of an inline search field.
class BUL_API Stack final : public Widget {
public:
    enum class Direction { Vertical, Horizontal };

    explicit Stack(Direction direction = Direction::Vertical);

    Stack& add(std::unique_ptr<Widget> child);
    /// `stretch` > 0 makes this child share the space left over by the others.
    Stack& add(std::unique_ptr<Widget> child, float stretch);
    Stack& gap(float value) noexcept {
        mGap = value;
        return *this;
    }

    [[nodiscard]] Size    measure(float availableWidth, Theme const&, Canvas const&) const override;
    void                  layout(Rect const& outer, Theme const&, Canvas const&) override;
    void                  paint(Canvas&, Theme const&) const override;
    [[nodiscard]] Widget* hitTest(Point point) override;
    void                  setControlState(ControlState const& state) override;
    void                  collectFocusables(std::vector<Widget*>& out) override;

private:
    struct Entry {
        std::unique_ptr<Widget> child;
        float                   stretch{};
    };

    Direction          mDirection;
    std::vector<Entry> mEntries;
    float              mGap{-1.f}; // < 0: use Theme::gap
};

} // namespace bul

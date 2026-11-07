#pragma once

#include <functional>
#include <string>

#include "bul/widget/Widget.h"

namespace bul {

/// A push button.
class Button final : public Widget {
public:
    explicit Button(std::string text, std::function<void()> onClick = {});

    [[nodiscard]] Size measure(float availableWidth, Theme const&, Canvas const&) const override;
    void               paint(Canvas&, Theme const&) const override;
    void               setHovered(bool hovered) override { mHovered = hovered; }
    void               activate() override;
    [[nodiscard]] bool interactive() const noexcept override { return true; }

private:
    std::string           mText;
    std::function<void()> mOnClick;
    bool                  mHovered{};
};

} // namespace bul

#pragma once

#include <string>

#include "bul/widget/Widget.h"

namespace bul {

/// A single line of text.
class Label final : public Widget {
public:
    explicit Label(std::string text, float scale = 1.f, Align align = Align::Left);

    Label& muted(bool value = true) noexcept {
        mMuted = value;
        return *this;
    }

    [[nodiscard]] Size measure(float availableWidth, Theme const&, Canvas const&) const override;
    void               paint(Canvas&, Theme const&) const override;

private:
    std::string mText;
    float       mScale{1.f};
    Align       mAlign{Align::Left};
    bool        mMuted{};
};

} // namespace bul

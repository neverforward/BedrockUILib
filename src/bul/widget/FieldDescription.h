#pragma once

#include <string>
#include <string_view>

#include "bul/widget/Widget.h"

namespace bul {

/// The helper line under a field's control. Muted and slightly smaller than the
/// label; dims with the field when it is disabled.
class BUL_API FieldDescription final : public Widget {
public:
    explicit FieldDescription(std::string text, float scale = 0.85f);

    [[nodiscard]] std::string_view typeName() const noexcept override { return "FieldDescription"; }

    [[nodiscard]] Size measureContent(float availableWidth, Theme const&, Canvas const&) const override;
    void               paintContent(Canvas&, Theme const&) const override;
    void               setControlState(ControlState const& state) override { mState = state; }

    [[nodiscard]] std::string const& text() const noexcept { return mText; }

private:
    std::string  mText;
    float        mScale{0.85f};
    ControlState mState{};
};

} // namespace bul

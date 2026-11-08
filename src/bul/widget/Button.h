#pragma once

#include <functional>
#include <string>

#include "bul/widget/Widget.h"

namespace bul {

/// A push button.
///
/// `Type` follows the usual variant set: a default filled button, a quieter
/// secondary one, a destructive one for actions that delete, an outlined one on
/// a transparent surface, and a link that is text only. `Size` changes the text
/// scale and the padding, not the colour.
class Button final : public Widget {
public:
    enum class Type { Default, Secondary, Destructive, Outline, Link };
    enum class Size { XSmall, Small, Default, Large };

    explicit Button(std::string text, std::function<void()> onClick = {});

    Button& type(Type value) noexcept {
        mType = value;
        return *this;
    }
    Button& size(Size value) noexcept {
        mSize = value;
        return *this;
    }
    Button& disabled(bool value) {
        mLocalDisabled = value;
        return *this;
    }

    [[nodiscard]] Type type() const noexcept { return mType; }
    [[nodiscard]] Size size() const noexcept { return mSize; }
    [[nodiscard]] bool disabled() const noexcept { return mLocalDisabled || mState.disabled; }

    // Qualified: Button::Size (the variant enum) shadows bul::Size inside the class.
    [[nodiscard]] bul::Size measure(float availableWidth, Theme const&, Canvas const&) const override;
    void                    paint(Canvas&, Theme const&) const override;
    void                    setHovered(bool hovered) override { mHovered = hovered; }
    void                    activate() override;
    [[nodiscard]] bool      interactive() const noexcept override { return !disabled(); }
    void                    setControlState(ControlState const& state) override { mState = state; }

private:
    struct Metrics {
        float textScale;
        float padX;
        float padY;
    };

    [[nodiscard]] Metrics metrics(Theme const&) const noexcept;

    std::string           mText;
    std::function<void()> mOnClick;
    Type                  mType{Type::Default};
    Size                  mSize{Size::Default};
    ControlState          mState{};
    bool                  mLocalDisabled{};
    bool                  mHovered{};
};

} // namespace bul

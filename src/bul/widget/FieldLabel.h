#pragma once

#include <memory>
#include <string>

#include "bul/widget/Widget.h"

namespace bul {

/// The label of a `Field`: one line of text plus two decorations the field can
/// impose on it -- a required marker and an optional badge ("Recommended").
///
/// The marker is drawn in `Theme::invalid`; the badge is any widget and sits
/// after the text, vertically centred, so `Badge` is the intended payload.
class FieldLabel final : public Widget {
public:
    explicit FieldLabel(std::string text, float scale = 1.f);

    /// Appends a decoration after the text, e.g. a `Badge`.
    FieldLabel& badge(std::unique_ptr<Widget> widget);
    /// Shows the required marker (`*`) after the text.
    FieldLabel& required(bool value) noexcept {
        mLocalRequired = value;
        return *this;
    }
    /// Renders the text in `Theme::textMuted`, like a hint.
    FieldLabel& muted(bool value = true) noexcept {
        mMuted = value;
        return *this;
    }

    [[nodiscard]] bool               required() const noexcept { return mLocalRequired || mState.required; }
    [[nodiscard]] std::string const& text() const noexcept { return mText; }

    [[nodiscard]] Size measure(float availableWidth, Theme const&, Canvas const&) const override;
    void               layout(Rect const& outer, Theme const&, Canvas const&) override;
    void               paint(Canvas&, Theme const&) const override;
    void               setControlState(ControlState const& state) override { mState = state; }

private:
    [[nodiscard]] float textWidth(Theme const&, Canvas const&) const noexcept;
    [[nodiscard]] bool  disabled() const noexcept { return mState.disabled; }

    std::string             mText;
    std::unique_ptr<Widget> mBadge;
    float                   mScale{1.f};
    bool                    mMuted{};
    bool                    mLocalRequired{};
    ControlState            mState{};
};

} // namespace bul

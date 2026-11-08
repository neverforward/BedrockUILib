#pragma once

#include <functional>
#include <string>
#include <string_view>

#include "bul/widget/Widget.h"

namespace bul {

/// A checkbox with an inline label.
///
/// Maps to the Base UI parts: the widget is `Checkbox.Root` (the box is its
/// bounds, `checked` / `onCheckedChange` and the `indeterminate` prop are the
/// root's) and it paints `Checkbox.Indicator` itself, both the tick and the
/// mixed dash. The inline text is the accessible label: Base UI expects the
/// checkbox to be wrapped in a label, and the field's own label (when it has
/// one) is the group heading.
class BUL_API Checkbox final : public Widget {
public:
    Checkbox();

    /// The inline label drawn right of the box (Base UI's enclosing label).
    Checkbox& text(std::string text);

    /// Programmatic state; does not fire `onCheckedChange`.
    Checkbox&          checked(bool value);
    [[nodiscard]] bool checked() const noexcept { return mChecked; }

    /// Mixed state: neither ticked nor unticked. Clicking a mixed box ticks it
    /// and clears the mixed state, like Base UI does.
    Checkbox&          indeterminate(bool value);
    [[nodiscard]] bool indeterminate() const noexcept { return mIndeterminate; }

    Checkbox& onCheckedChange(std::function<void(bool)> callback);

    Checkbox&          disabled(bool value);
    [[nodiscard]] bool disabled() const noexcept { return mLocalDisabled || mState.disabled; }

    Checkbox&          readOnly(bool value);
    [[nodiscard]] bool readOnly() const noexcept { return mLocalReadOnly; }

    Checkbox&          invalid(bool value);
    Checkbox&          required(bool value) noexcept;
    [[nodiscard]] bool required() const noexcept { return mLocalRequired || mState.required; }

    [[nodiscard]] Size measure(float availableWidth, Theme const&, Canvas const&) const override;
    void               paint(Canvas&, Theme const&) const override;
    void               setHovered(bool hovered) override { mHovered = hovered; }
    void               setFocused(bool focused) override { mFocused = focused; }
    void               activate() override;
    [[nodiscard]] bool interactive() const noexcept override { return !disabled(); }
    [[nodiscard]] bool focusable() const noexcept override { return !disabled(); }
    bool               key(KeyStroke const& stroke) override;
    void               setControlState(ControlState const& state) override { mState = state; }

private:
    void toggle();

    std::string               mText{};
    std::function<void(bool)> mOnCheckedChange{};
    ControlState              mState{};
    bool                      mChecked{};
    bool                      mIndeterminate{};
    bool                      mLocalDisabled{};
    bool                      mLocalReadOnly{};
    bool                      mLocalInvalid{};
    bool                      mLocalRequired{};
    bool                      mHovered{};
    bool                      mFocused{};
};

} // namespace bul

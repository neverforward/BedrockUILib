#pragma once

#include <functional>
#include <string>

#include "bul/widget/Widget.h"

namespace bul::spec {
struct Switch;
}

namespace bul {

/// An on/off switch: a track with a thumb that sits on the left or the right.
///
/// This is the HTML `input[type=checkbox][role=switch]` and the Qt Quick
/// `Switch` in one widget: `checked` / `onCheckedChange` are the element's
/// state, `text` is its label, and `disabled` / `readOnly` / `required` behave
/// like the ones on every other control. The state is carried by the colour of
/// the track, like the `Checkbox`.
class BUL_API Switch final : public Widget {
public:
    Switch();

    /// The inline label drawn right of the track.
    Switch& text(std::string text);

    /// Programmatic state; does not fire `onCheckedChange`.
    Switch&            checked(bool value);
    [[nodiscard]] bool checked() const noexcept { return mChecked; }

    Switch& onCheckedChange(std::function<void(bool)> callback);

    Switch&            disabled(bool value);
    [[nodiscard]] bool disabled() const noexcept { return mLocalDisabled || mState.disabled; }

    /// Focusable, but the state cannot change.
    Switch&            readOnly(bool value);
    [[nodiscard]] bool readOnly() const noexcept { return mLocalReadOnly; }

    Switch&            invalid(bool value);
    Switch&            required(bool value) noexcept;
    [[nodiscard]] bool required() const noexcept { return mLocalRequired || mState.required; }

    static constexpr std::string_view kTypeName{"Switch"};

    [[nodiscard]] std::string_view typeName() const noexcept override { return kTypeName; }

    [[nodiscard]] bool state(std::string_view name) const override { return name == "disabled" ? disabled() : name == "checked" ? checked() : name == "read-only" ? readOnly() : Widget::state(name); }

    /// Applies a spec node; the spec layer calls this so the mapping lives here.
    Switch& apply(spec::Switch const& node);

    [[nodiscard]] Size measureContent(float availableWidth, Theme const&, Canvas const&) const override;
    void               paintContent(Canvas&, Theme const&) const override;
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
    bool                      mLocalDisabled{};
    bool                      mLocalReadOnly{};
    bool                      mLocalInvalid{};
    bool                      mLocalRequired{};
    bool                      mHovered{};
    bool                      mFocused{};
};

} // namespace bul

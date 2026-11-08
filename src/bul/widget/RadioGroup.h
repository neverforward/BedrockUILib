#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "bul/widget/Stack.h"
#include "bul/widget/Widget.h"

namespace bul {

/// A group of radio buttons: exactly one option can be chosen.
///
/// Maps to the Base UI parts: the group is `Radio.Group` (value / onValueChange,
/// disabled, readOnly, required) and every `add` creates one `Radio.Root` with
/// its label; the chosen option paints a `Radio.Indicator` dot instead of a
/// tick. Like `Checkbox`, the state is carried by the fill, not by a frame.
class BUL_API RadioGroup final : public Widget {
public:
    RadioGroup();
    ~RadioGroup() override; // defined in the .cpp: the radios are incomplete here

    /// Vertical by default; `Horizontal` lays the options out in one row.
    RadioGroup& direction(Stack::Direction value);
    RadioGroup& gap(float value);

    /// Adds one option; returns the group so calls chain.
    RadioGroup& add(std::string value, std::string text);
    RadioGroup& add(std::string value, std::string text, bool disabled);

    /// Programmatic selection; does not fire `onValueChange`.
    RadioGroup&                      value(std::string value);
    [[nodiscard]] std::string const& value() const noexcept { return mValue; }

    /// Fires when the user picks a different option (click or Space/arrow keys).
    RadioGroup& onValueChange(std::function<void(std::string const&)> callback);

    RadioGroup&        disabled(bool value);
    [[nodiscard]] bool disabled() const noexcept { return mLocalDisabled || mState.disabled; }

    /// Focusable, but the selection cannot change.
    RadioGroup&        readOnly(bool value);
    [[nodiscard]] bool readOnly() const noexcept { return mLocalReadOnly; }

    RadioGroup&        invalid(bool value);
    [[nodiscard]] bool invalid() const noexcept { return mLocalInvalid || mState.invalid; }

    RadioGroup&        required(bool value) noexcept;
    [[nodiscard]] bool required() const noexcept { return mLocalRequired || mState.required; }

    [[nodiscard]] Size    measure(float availableWidth, Theme const&, Canvas const&) const override;
    void                  layout(Rect const& outer, Theme const&, Canvas const&) override;
    void                  paint(Canvas&, Theme const&) const override;
    [[nodiscard]] Widget* hitTest(Point point) override;
    void                  setControlState(ControlState const& state) override;
    void                  collectFocusables(std::vector<Widget*>& out) override;

private:
    class Radio;

    void pick(std::string const& value);
    void move(int step);

    std::vector<std::unique_ptr<Radio>>     mRadios;
    std::string                             mValue;
    std::function<void(std::string const&)> mOnValueChange;
    ControlState                            mState;
    Stack::Direction                        mDirection{Stack::Direction::Vertical};
    float                                   mGap{-1.f}; // < 0: use Theme::gap
    bool                                    mLocalDisabled{};
    bool                                    mLocalReadOnly{};
    bool                                    mLocalInvalid{};
    bool                                    mLocalRequired{};
};

} // namespace bul

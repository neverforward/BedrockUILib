#pragma once

#include <memory>
#include <string>

#include "bul/widget/Widget.h"

namespace bul {

/// A form field: a label, one control and a description, plus the state
/// attributes the web component set puts on the field element (`data-disabled`,
/// `data-invalid`, `required`).
///
/// Vertical (the default) stacks label, control and description. Horizontal puts
/// the label -- and the description under it -- on the left and gives the
/// control the rest of the width, which is what an inline search row needs.
///
/// The state a field carries is imposed on its children through
/// `setControlState`, so `field.disabled(true)` dims and disables the control,
/// the label and the description without each of them being told separately.
class BUL_API Field final : public Widget {
public:
    enum class Orientation { Vertical, Horizontal };

    Field();

    Field& orientation(Orientation value) noexcept;
    /// Space between the parts; negative means the field default.
    Field& gap(float value) noexcept;

    Field& label(std::unique_ptr<Widget> widget);
    /// Convenience: adds a `FieldLabel` with this text.
    Field& label(std::string text);
    Field& control(std::unique_ptr<Widget> widget);
    Field& description(std::unique_ptr<Widget> widget);
    /// Convenience: adds a `FieldDescription` with this text.
    Field& description(std::string text);

    Field& disabled(bool value);
    Field& invalid(bool value);
    Field& required(bool value);

    [[nodiscard]] Orientation orientation() const noexcept { return mOrientation; }
    [[nodiscard]] bool        disabled() const noexcept { return mEffective.disabled; }
    [[nodiscard]] bool        invalid() const noexcept { return mEffective.invalid; }
    [[nodiscard]] bool        required() const noexcept { return mEffective.required; }

    [[nodiscard]] Widget* labelWidget() const noexcept { return mLabel.get(); }
    [[nodiscard]] Widget* controlWidget() const noexcept { return mControl.get(); }
    [[nodiscard]] Widget* descriptionWidget() const noexcept { return mDescription.get(); }

    [[nodiscard]] Size    measure(float availableWidth, Theme const&, Canvas const&) const override;
    void                  layout(Rect const& outer, Theme const&, Canvas const&) override;
    void                  paint(Canvas&, Theme const&) const override;
    [[nodiscard]] Widget* hitTest(Point point) override;
    void                  setControlState(ControlState const& state) override;
    void                  collectFocusables(std::vector<Widget*>& out) override;

private:
    [[nodiscard]] float gapOf(Theme const& theme) const noexcept;
    void                adopt(std::unique_ptr<Widget>& slot, std::unique_ptr<Widget> widget);
    void                propagate();

    Orientation             mOrientation{Orientation::Vertical};
    float                   mGap{-1.f};
    ControlState            mLocal{};
    ControlState            mInherited{};
    ControlState            mEffective{};
    std::unique_ptr<Widget> mLabel{};
    std::unique_ptr<Widget> mControl{};
    std::unique_ptr<Widget> mDescription{};
};

} // namespace bul

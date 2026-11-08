#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "bul/widget/Widget.h"

namespace bul {

/// A dropdown: a trigger that shows the current value, and a popup list of
/// options drawn above the rest of the tree.
///
/// The parts of the Base UI select this maps to: the widget is the root and
/// owns the value and the open state (`value` / `onValueChange`, `open` /
/// `onOpenChange`); its bounds are the trigger and it paints `Select.Value` and
/// `Select.Icon` itself; each `item(...)` is one `Select.Item` with its
/// `ItemText` label, and the selected row gets an `ItemIndicator` marker.
///
/// The popup is an overlay: the modal host paints it after the whole tree and
/// gives it the pointer first, so it is not clipped by the layout and an
/// outside click closes it instead of activating the control behind it.
class BUL_API Select final : public Widget {
public:
    Select();

    /// Text shown while no value is selected (`Select.Value` placeholder).
    Select& placeholder(std::string text);

    /// Adds one option. The label defaults to the value.
    Select& item(std::string value, std::string label = {});
    Select& item(std::string value, std::string label, bool disabled);

    /// Programmatic value; does not fire `onValueChange`.
    Select& value(std::string value);

    /// The chosen value, empty when nothing is selected.
    [[nodiscard]] std::string const& value() const noexcept { return mValue; }

    /// Label of the selected option when it has one, the raw value otherwise,
    /// and the placeholder while nothing is selected.
    [[nodiscard]] std::string displayText() const;

    Select& onValueChange(std::function<void(std::string const&)> callback);
    Select& onOpenChange(std::function<void(bool)> callback);

    /// Opens or closes the popup programmatically.
    Select&            open(bool value);
    [[nodiscard]] bool open() const noexcept { return mOpen; }

    Select&            disabled(bool value);
    [[nodiscard]] bool disabled() const noexcept { return mLocalDisabled || mState.disabled; }

    Select&            required(bool value) noexcept;
    [[nodiscard]] bool required() const noexcept { return mLocalRequired || mState.required; }

    [[nodiscard]] Size    measure(float availableWidth, Theme const&, Canvas const&) const override;
    void                  layout(Rect const& outer, Theme const&, Canvas const&) override;
    void                  paint(Canvas&, Theme const&) const override;
    void                  setHovered(bool hovered) override { mHovered = hovered; }
    void                  setPointer(Point point) override { mPointer = point; }
    void                  activate() override;
    void                  activateAt(Point point) override;
    [[nodiscard]] bool    interactive() const noexcept override { return !disabled(); }
    [[nodiscard]] bool    focusable() const noexcept override { return !disabled(); }
    void                  setFocused(bool focused) override { mFocused = focused; }
    bool                  key(KeyStroke const& stroke) override;
    void                  setControlState(ControlState const& state) override { mState = state; }

    // The open popup participates in the host's overlay pass.
    void                  collectOverlays(std::vector<Widget*>& out) override;
    void                  paintOverlay(Canvas&, Theme const&) const override;
    [[nodiscard]] Widget* overlayHitTest(Point point) override;
    void                  dismissOverlay() override;

private:
    struct Item {
        std::string value;
        std::string label;
        bool        disabled{};
    };

    [[nodiscard]] int selectedIndex() const noexcept;
    [[nodiscard]] int itemAt(Point point) const noexcept;
    [[nodiscard]] int hoveredIndex() const noexcept;
    void              setOpen(bool value);
    void              moveHighlight(int step) noexcept;
    void              select(int index);

    std::string                             mPlaceholder{};
    std::vector<Item>                       mItems{};
    std::string                             mValue{};
    std::function<void(std::string const&)> mOnValueChange{};
    std::function<void(bool)>               mOnOpenChange{};
    ControlState                            mState{};
    Rect                                    mPopup{};
    float                                   mItemHeight{};
    Point                                   mPointer{};
    bool                                    mLocalDisabled{};
    bool                                    mLocalRequired{};
    bool                                    mOpen{};
    bool                                    mHovered{};
    bool                                    mFocused{};
    int                                     mHighlight{-1};
};

} // namespace bul

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "bul/Canvas.h"
#include "bul/Geometry.h"
#include "bul/Export.h"

namespace bul {

struct Theme;

/// Visual state a container imposes on its descendants.
///
/// This is the C++ spelling of the attributes a form field carries in the web
/// component set the library mirrors: `data-disabled` on the field, `data-invalid`
/// on the field, `required` on the label. A control combines what it was told
/// directly with what its container imposed; the container never reaches into a
/// control's own flags.
struct ControlState {
    bool disabled{};
    bool invalid{};
    bool required{};

    [[nodiscard]] constexpr ControlState merged(ControlState const& other) const noexcept {
        return {disabled || other.disabled, invalid || other.invalid, required || other.required};
    }

    friend constexpr bool operator==(ControlState const&, ControlState const&) = default;
};

/// Logical key a focusable widget understands. The host maps engine key codes to
/// this, so widgets stay free of the SDK.
enum class Key { None, Backspace, Delete, Left, Right, Home, End, Enter, Tab, Escape, A };

struct KeyStroke {
    Key  key{Key::None};
    bool ctrl{};
    bool shift{};
};

/// A node of the widget tree.
///
/// The tree is retained mode and layout is top-down, like Qt Quick: a parent
/// asks each child for its desired size, then arranges the children inside its
/// own bounds. Widgets only paint through `Canvas`, so the same tree can be
/// driven by the game, a test recorder, or an offline previewer.
class BUL_API Widget {
public:
    Widget()                         = default;
    Widget(Widget const&)            = delete;
    Widget& operator=(Widget const&) = delete;
    virtual ~Widget()                = default;

    Widget& margin(float all) noexcept {
        mMargin = {all, all, all, all};
        return *this;
    }
    Widget& margin(float horizontal, float vertical) noexcept {
        mMargin = {horizontal, vertical, horizontal, vertical};
        return *this;
    }

    [[nodiscard]] Rect const&   bounds() const noexcept { return mBounds; }
    [[nodiscard]] Insets const& margin() const noexcept { return mMargin; }

    /// Desired size including this widget's margin.
    [[nodiscard]] virtual Size measure(float availableWidth, Theme const&, Canvas const&) const = 0;

    /// Assigns bounds to this widget and its children.
    virtual void layout(Rect const& outer, Theme const&, Canvas const&);

    virtual void paint(Canvas&, Theme const&) const = 0;

    /// Deepest widget under `point`, or nullptr.
    [[nodiscard]] virtual Widget* hitTest(Point point);

    /// Hover feedback; the host clears it every time the pointer moves.
    virtual void setHovered(bool) {}

    /// Pointer click on an interactive widget.
    virtual void activate() {}

    /// Pointer click at a known position: a text field puts its caret under the
    /// pointer, everything else ignores the position.
    virtual void activateAt(Point point) {
        (void)point;
        activate();
    }

    [[nodiscard]] virtual bool interactive() const noexcept { return false; }

    // ------------------------------------------------------------- control state

    /// Called by the container when the state it imposes changes.
    virtual void setControlState(ControlState const&) {}

    // -------------------------------------------------------------------- focus

    /// Whether the host may give this widget keyboard focus (Tab order, clicks).
    [[nodiscard]] virtual bool focusable() const noexcept { return false; }

    virtual void setFocused(bool) {}

    /// Returns true when the widget consumed the stroke.
    virtual bool key(KeyStroke const&) { return false; }

    /// Whether typed characters should be routed to this widget.
    [[nodiscard]] virtual bool acceptsTextInput() const noexcept { return false; }

    /// Whether a click on this widget should open the host's file picker.
    [[nodiscard]] virtual bool wantsFilePicker() const noexcept { return false; }

    /// Applies a path chosen by the host's file picker.
    virtual void chooseFile(std::string_view) {}

    /// Inserts typed or pasted text at the caret, replacing the selection.
    virtual void textInput(std::string_view) {}

    /// The selection as text; used by the host for Ctrl+C / Ctrl+X.
    [[nodiscard]] virtual std::string selectedText() const { return {}; }

    /// Appends every focusable widget below this one in visual order (Tab order).
    virtual void collectFocusables(std::vector<Widget*>& out) {
        if (focusable()) {
            out.push_back(this);
        }
    }

protected:
    [[nodiscard]] Rect contentRect(Rect const& outer) const noexcept {
        return {
            outer.x + mMargin.left,
            outer.y + mMargin.top,
            outer.width - mMargin.horizontal(),
            outer.height - mMargin.vertical()
        };
    }

    Rect   mBounds{};
    Insets mMargin{};
};

} // namespace bul

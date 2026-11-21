#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "bul/Canvas.h"
#include "bul/Export.h"
#include "bul/Geometry.h"
#include "bul/Style.h"

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
enum class Key { None, Backspace, Delete, Left, Right, Up, Down, Home, End, Enter, Space, Tab, Escape, A };

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

    /// This widget's colour overrides, merged over the theme it inherits.
    Widget&                    style(Style value);
    [[nodiscard]] Style const& style() const noexcept { return mStyle; }

    /// The theme this widget paints with: the inherited theme, the rules of the
    /// active style sheet that match it, then its own inline `style()`.
    [[nodiscard]] Theme resolvedTheme(Theme const& theme) const;

    /// The element name a style-sheet rule can select, e.g. "Button".
    [[nodiscard]] virtual std::string_view typeName() const noexcept { return {}; }

    /// A class selector target, like the `class` attribute of an element.
    Widget&                          className(std::string value);
    [[nodiscard]] std::string const& className() const noexcept { return mClassName; }

    /// An id selector target, like the `id` attribute of an element.
    Widget&                          id(std::string value);
    [[nodiscard]] std::string const& id() const noexcept { return mId; }

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

    /// Size of the last layout, in GUI units.
    [[nodiscard]] Size  size() const noexcept { return {mBounds.width, mBounds.height}; }
    [[nodiscard]] float width() const noexcept { return mBounds.width; }
    [[nodiscard]] float height() const noexcept { return mBounds.height; }

    // ------------------------------------------------------- size constraints
    //
    // The Qt/HTML knobs every element has: a fixed size, or the minimum and
    // maximum the layout must respect. Values are content sizes (margin excluded)
    // and 0 means "no constraint".

    Widget& fixedSize(float width, float height) noexcept;
    Widget& fixedWidth(float value) noexcept;
    Widget& fixedHeight(float value) noexcept;
    Widget& minSize(float width, float height) noexcept;
    Widget& maxSize(float width, float height) noexcept;

    /// Desired size including this widget's margin and size constraints.
    [[nodiscard]] Size measure(float availableWidth, Theme const&, Canvas const&) const;

    /// The content size before the constraints; components override this one.
    [[nodiscard]] virtual Size measureContent(float availableWidth, Theme const&, Canvas const&) const = 0;

    /// Assigns bounds to this widget and its children.
    virtual void layout(Rect const& outer, Theme const&, Canvas const&);

    /// Paints with this widget's style applied over the theme it was handed. A
    /// container passes its already-resolved theme down, so a style on a
    /// container colours its whole subtree.
    void paint(Canvas& canvas, Theme const& theme) const;

    /// The component's own drawing; the theme already carries this widget's style.
    virtual void paintContent(Canvas&, Theme const&) const = 0;

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

    [[nodiscard]] virtual bool interactive() const noexcept { return mOnClick != nullptr; }

    // ------------------------------------------------------------------ events
    //
    // The element handlers the kit mirrors from HTML and Qt. They are optional
    // and independent of the widget's own kind: a `Label` with an `onClick`
    // becomes clickable even though its kind has no action of its own. The host
    // fires them through `notifyClick` / `notifyFocus` / `notifyHover`, which run
    // the widget's own reaction first.

    Widget& onClick(std::function<void()> callback);
    Widget& onFocus(std::function<void()> callback);
    Widget& onBlur(std::function<void()> callback);
    Widget& onHover(std::function<void(bool)> callback);

    /// Host entry points: run the widget's reaction, then the callback.
    void notifyClick(Point point);
    void notifyFocus(bool focused);
    void notifyHover(bool hovered);

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

    // ---------------------------------------------------------------- overlays

    /// A popup (a select list, a menu) is drawn over the whole tree instead of
    /// inside its parent's bounds: the host paints it after the tree and gives
    /// it the pointer first while it is open.

    /// Appends every open popup below this widget.
    virtual void collectOverlays(std::vector<Widget*>&) {}

    /// Paints the open popup; called after the tree has been painted.
    void paintOverlay(Canvas& canvas, Theme const& theme) const;

    /// The popup's own drawing; the theme already carries this widget's style.
    virtual void paintOverlayContent(Canvas&, Theme const&) const {}

    /// Claims the pointer while a popup is open (nullptr when closed). Claiming
    /// the whole screen is what makes the popup modal: an outside click closes
    /// it instead of activating the control behind it.
    [[nodiscard]] virtual Widget* overlayHitTest(Point) { return nullptr; }

    /// Closes the popup because the host asked for it (escape, click outside).
    virtual void dismissOverlay() {}

    /// Pointer position while this widget is hovered; a popup uses it for the
    /// highlighted row.
    virtual void setPointer(Point) {}

protected:
    [[nodiscard]] Rect contentRect(Rect const& outer) const noexcept {
        return {
            outer.x + mMargin.left,
            outer.y + mMargin.top,
            outer.width - mMargin.horizontal(),
            outer.height - mMargin.vertical()
        };
    }

    Rect        mBounds{};
    Insets      mMargin{};
    Style       mStyle{};
    std::string mClassName{};
    std::string mId{};

    std::function<void()>     mOnClick;
    std::function<void()>     mOnFocus;
    std::function<void()>     mOnBlur;
    std::function<void(bool)> mOnHover;

    float mFixedWidth{};
    float mFixedHeight{};
    float mMinWidth{};
    float mMinHeight{};
    float mMaxWidth{};
    float mMaxHeight{};
};

} // namespace bul

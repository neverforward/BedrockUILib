#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "bul/TextEdit.h"
#include "bul/widget/Widget.h"

namespace bul {

/// A single line text field.
///
/// `Kind` selects text, password, search or file rendering. `disabled` and
/// `invalid` are the two states a field carries; a container may also impose
/// them through `setControlState`, which is how `Field` styles its children.
///
/// Focus, the caret and the selection are owned by the host: it gives the widget
/// focus (`setFocused`), routes strokes through `key` and typed characters
/// through `textInput`. That split keeps the widget free of the SDK and fully
/// testable with a recording canvas.
class BUL_API Input final : public Widget {
public:
    enum class Kind { Text, Password, Search, File };

    Input();
    explicit Input(std::string value);

    Input&                           value(std::string text);
    [[nodiscard]] std::string const& value() const noexcept { return mEdit.text(); }

    Input& placeholder(std::string text);
    Input& kind(Kind value) noexcept;
    Input& disabled(bool value);
    Input& invalid(bool value);
    Input& required(bool value) noexcept;
    Input& maxLength(std::size_t codePoints) noexcept;

    Input& onChange(std::function<void(std::string const&)> callback);
    Input& onSubmit(std::function<void(std::string const&)> callback);

    /// Installs a check that receives the value after every user edit. The input
    /// is drawn invalid while the check returns false and becomes valid again as
    /// soon as it returns true; the initial and programmatic values are not
    /// checked, only edits are.
    Input& validator(std::function<bool(std::string const&)> check);

    /// Called after the host's file picker returned a path: the consumer can
    /// read it from `filePath()` instead of copying the string.
    Input& onChoose(std::function<void(std::string const&)> callback);
    /// Sets the chosen file without user interaction (no callbacks).
    Input&                           filePath(std::string path);
    [[nodiscard]] std::string const& filePath() const noexcept { return mFilePath; }
    Input&                           chooseText(std::string text);
    Input&                           emptyFileText(std::string text);

    [[nodiscard]] Kind kind() const noexcept { return mKind; }
    [[nodiscard]] bool disabled() const noexcept { return mLocalDisabled || mState.disabled; }
    [[nodiscard]] bool invalid() const noexcept { return mLocalInvalid || mState.invalid || mValidationFailed; }
    [[nodiscard]] bool validationFailed() const noexcept { return mValidationFailed; }
    [[nodiscard]] bool required() const noexcept { return mLocalRequired || mState.required; }
    [[nodiscard]] bool focused() const noexcept { return mFocused; }

    [[nodiscard]] Size        measure(float availableWidth, Theme const&, Canvas const&) const override;
    void                      layout(Rect const& outer, Theme const&, Canvas const&) override;
    void                      paint(Canvas&, Theme const&) const override;
    void                      setHovered(bool hovered) override { mHovered = hovered; }
    void                      activate() override;
    void                      activateAt(Point point) override;
    [[nodiscard]] bool        interactive() const noexcept override { return !disabled(); }
    [[nodiscard]] bool        focusable() const noexcept override { return mKind != Kind::File && !disabled(); }
    void                      setFocused(bool focused) override;
    bool                      key(KeyStroke const& stroke) override;
    [[nodiscard]] bool        acceptsTextInput() const noexcept override { return mKind != Kind::File && !disabled(); }
    [[nodiscard]] bool        wantsFilePicker() const noexcept override { return mKind == Kind::File && !disabled(); }
    void                      chooseFile(std::string_view path) override;
    void                      textInput(std::string_view text) override;
    [[nodiscard]] std::string selectedText() const override;
    void                      setControlState(ControlState const& state) override { mState = state; }

private:
    struct Metrics {
        float padX;
        float padY;
        float scale;
    };

    /// What is drawn for the current value: masked for passwords, plus the
    /// per-code-point advance table and the horizontal scroll that keeps the
    /// caret inside the box. The table is the single source of truth for the
    /// caret, the selection, the visible window and the glyph positions.
    struct TextLayout {
        std::string              display{};
        std::vector<std::size_t> offsets{}; // byte offset of each code point; last = size
        std::vector<float>       prefix{};  // width before that code point
        std::size_t              caret{};
        std::size_t              selectionBegin{};
        std::size_t              selectionEnd{};
        float                    scroll{};
    };

    [[nodiscard]] Metrics metrics(Theme const&) const noexcept;
    /// The text box: bounds without border and padding.
    [[nodiscard]] Rect        textArea(Metrics const&) const noexcept;
    [[nodiscard]] TextLayout  textLayout(Canvas const&, Theme const&, Rect const& area) const;
    [[nodiscard]] std::size_t caretAt(float pointerX, TextLayout const& layout, Rect const& area) const;
    void                      paintText(Canvas&, Theme const&, Metrics const&, float fade) const;
    void                      paintFile(Canvas&, Theme const&, Metrics const&, float fade) const;
    void                      notifyChanged(std::string const& previous);
    /// Runs the installed validator, if any, against the current value.
    void runValidation();

    Kind                                    mKind{Kind::Text};
    TextEdit                                mEdit{};
    ControlState                            mState{};
    std::string                             mPlaceholder{};
    std::string                             mFilePath{};
    std::string                             mChooseText{"Choose File"};
    std::string                             mEmptyFileText{"No file chosen"};
    std::function<void(std::string const&)> mOnChange{};
    std::function<void(std::string const&)> mOnSubmit{};
    std::function<void(std::string const&)> mOnChoose{};
    std::function<bool(std::string const&)> mValidator{};
    bool                                    mLocalDisabled{};
    bool                                    mLocalInvalid{};
    bool                                    mLocalRequired{};
    bool                                    mValidationFailed{};
    bool                                    mFocused{};
    bool                                    mHovered{};
    bool                                    mPendingClick{};
    float                                   mPendingClickX{};
};

} // namespace bul

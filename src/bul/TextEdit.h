#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace bul {

/// The editing state behind `Input`: one line of UTF-8 text, a caret and a
/// selection.
///
/// Offsets are byte offsets into the UTF-8 string and always sit on a code point
/// boundary, so a caret can never be placed in the middle of a character. The
/// selection is stored as an anchor plus the caret: the anchor is the end that
/// stays put while Shift extends the caret.
///
/// Pure data with no drawing and no engine types, so every editing rule is
/// unit-tested without a game.
class TextEdit {
public:
    /// Replaces the text and puts the caret at its end.
    void setText(std::string text);

    [[nodiscard]] std::string const& text() const noexcept { return mText; }
    [[nodiscard]] bool               empty() const noexcept { return mText.empty(); }

    /// Caret position in bytes. Default-constructed and `setText` carets sit at
    /// the end of the text.
    [[nodiscard]] std::size_t caret() const noexcept { return mCaret; }
    /// The fixed end of the selection; equal to the caret when nothing is selected.
    [[nodiscard]] std::size_t anchor() const noexcept { return mAnchor; }
    /// Moves the caret, collapsing the selection. The offset is snapped back to
    /// a code point boundary.
    void setCaret(std::size_t offset);

    /// Applies a text event at the caret, replacing the selection first.
    ///
    /// The event is a delta stream, not plain text: a backspace (`0x08`) erases
    /// the code point before the caret. An IME rewriting its composition sends
    /// exactly that -- backspaces for the characters it takes back, then the new
    /// composition text -- so handling it here is what keeps the pinyin from
    /// piling up in front of the committed characters. Other control characters
    /// (a pasted line break, a tab) are dropped: the field is single line.
    void insert(std::string_view text);
    /// Backspace: deletes the selection, or the code point before the caret.
    void eraseBackward();
    /// Delete: deletes the selection, or the code point after the caret.
    void eraseForward();

    /// Moves the caret by `codePoints` code points, collapsing the selection.
    void moveCaret(int codePoints);
    /// Moving variant used with Shift: the anchor stays, the selection grows.
    void extendCaret(int codePoints);
    void moveHome();
    void moveEnd();
    void selectAll();
    /// Moves the caret to `offset` keeping the anchor (Shift+click).
    void selectTo(std::size_t offset);
    void collapseSelection();

    [[nodiscard]] bool        hasSelection() const noexcept { return mAnchor != mCaret; }
    [[nodiscard]] std::size_t selectionBegin() const noexcept;
    [[nodiscard]] std::size_t selectionEnd() const noexcept;
    [[nodiscard]] std::string selectedText() const;
    void                      deleteSelection();

    /// Limits the text length in code points; 0 means unlimited.
    void                      setMaxLength(std::size_t codePoints) noexcept { mMaxLength = codePoints; }
    [[nodiscard]] std::size_t maxLength() const noexcept { return mMaxLength; }

    /// Text length in code points.
    [[nodiscard]] std::size_t length() const noexcept;
    /// Byte offset of the code point with index `index`, clamped to the end.
    [[nodiscard]] std::size_t offsetOfCodePoint(std::size_t index) const noexcept;
    /// Index of the code point that contains byte offset `offset`.
    [[nodiscard]] std::size_t codePointIndexOf(std::size_t offset) const noexcept;
    /// The text with one `*` per code point, used by password inputs.
    [[nodiscard]] std::string masked() const;

private:
    /// Inserts one code point at the caret, replacing the selection first and
    /// respecting the length limit.
    void insertPiece(std::string_view piece);

    std::string mText{};
    std::size_t mCaret{};
    std::size_t mAnchor{};
    std::size_t mMaxLength{}; // 0 = unlimited
};

} // namespace bul

#include "bul/TextEdit.h"

#include <algorithm>
#include <utility>

#include "bul/TextMetrics.h"

namespace bul {

namespace {

[[nodiscard]] bool isBoundary(std::string_view text, std::size_t offset) noexcept {
    return offset == 0 || offset >= text.size() || (static_cast<unsigned char>(text[offset]) & 0xC0u) != 0x80u;
}

/// Snaps an arbitrary offset back to the start of the code point that contains it.
[[nodiscard]] std::size_t clampBoundary(std::string_view text, std::size_t offset) noexcept {
    offset = std::min(offset, text.size());
    while (!isBoundary(text, offset)) {
        --offset;
    }
    return offset;
}

[[nodiscard]] std::size_t countCodePoints(std::string_view text) noexcept {
    std::size_t count = 0;
    for (std::size_t i = 0; i < text.size(); i = detail::nextCodePoint(text, i)) {
        ++count;
    }
    return count;
}

} // namespace

void TextEdit::setText(std::string text) {
    mText   = std::move(text);
    mCaret  = mText.size();
    mAnchor = mCaret;
}

void TextEdit::setCaret(std::size_t offset) {
    mCaret  = clampBoundary(mText, offset);
    mAnchor = mCaret;
}

void TextEdit::insert(std::string_view text) {
    for (std::size_t i = 0; i < text.size();) {
        std::size_t const next = detail::nextCodePoint(text, i);
        auto const        lead = static_cast<unsigned char>(text[i]);

        if (lead == 0x08u) {
            // The IME rewriting its composition sends a backspace per character
            // it takes back: apply it, or the pinyin piles up in front of the
            // committed text.
            eraseBackward();
        } else if (lead >= 0x20u && lead != 0x7Fu) {
            insertPiece(text.substr(i, next - i));
        }
        i = next;
    }
}

void TextEdit::insertPiece(std::string_view piece) {
    if (piece.empty()) {
        return;
    }
    std::size_t const begin = selectionBegin();
    std::size_t const end   = selectionEnd();

    if (mMaxLength != 0) {
        // The limit counts code points of the whole resulting text, not just of
        // the inserted part: those kept outside the selection plus the new text
        // have to fit.
        std::size_t const total    = countCodePoints(mText);
        std::size_t const selected = countCodePoints(std::string_view{mText}.substr(begin, end - begin));
        std::size_t const kept     = total - selected;
        if (kept + countCodePoints(piece) > mMaxLength) {
            return;
        }
    }

    mText.erase(begin, end - begin);
    mText.insert(begin, piece);
    mCaret  = begin + piece.size();
    mAnchor = mCaret;
}

void TextEdit::eraseBackward() {
    if (hasSelection()) {
        deleteSelection();
        return;
    }
    if (mCaret == 0) {
        return;
    }
    std::size_t const start = detail::previousCodePoint(mText, mCaret);
    mText.erase(start, mCaret - start);
    mCaret  = start;
    mAnchor = start;
}

void TextEdit::eraseForward() {
    if (hasSelection()) {
        deleteSelection();
        return;
    }
    if (mCaret >= mText.size()) {
        return;
    }
    std::size_t const end = detail::nextCodePoint(mText, mCaret);
    mText.erase(mCaret, end - mCaret);
    mAnchor = mCaret;
}

void TextEdit::moveCaret(int codePoints) {
    std::size_t target = mCaret;
    if (codePoints > 0) {
        for (int i = 0; i < codePoints; ++i) {
            target = detail::nextCodePoint(mText, target);
        }
    } else {
        for (int i = 0; i > codePoints; --i) {
            target = detail::previousCodePoint(mText, target);
        }
    }
    mCaret  = target;
    mAnchor = target;
}

void TextEdit::extendCaret(int codePoints) {
    std::size_t target = mCaret;
    if (codePoints > 0) {
        for (int i = 0; i < codePoints; ++i) {
            target = detail::nextCodePoint(mText, target);
        }
    } else {
        for (int i = 0; i > codePoints; --i) {
            target = detail::previousCodePoint(mText, target);
        }
    }
    mCaret = target;
}

void TextEdit::moveHome() {
    mCaret  = 0;
    mAnchor = 0;
}

void TextEdit::moveEnd() {
    mCaret  = mText.size();
    mAnchor = mCaret;
}

void TextEdit::selectAll() {
    mAnchor = 0;
    mCaret  = mText.size();
}

void TextEdit::selectTo(std::size_t offset) { mCaret = clampBoundary(mText, offset); }

void TextEdit::collapseSelection() { mAnchor = mCaret; }

std::size_t TextEdit::selectionBegin() const noexcept { return std::min(mAnchor, mCaret); }

std::size_t TextEdit::selectionEnd() const noexcept { return std::max(mAnchor, mCaret); }

std::string TextEdit::selectedText() const { return mText.substr(selectionBegin(), selectionEnd() - selectionBegin()); }

void TextEdit::deleteSelection() {
    if (!hasSelection()) {
        return;
    }
    std::size_t const begin = selectionBegin();
    std::size_t const end   = selectionEnd();
    mText.erase(begin, end - begin);
    mCaret  = begin;
    mAnchor = begin;
}

std::size_t TextEdit::length() const noexcept { return countCodePoints(mText); }

std::size_t TextEdit::offsetOfCodePoint(std::size_t index) const noexcept {
    std::size_t cursor = 0;
    while (cursor < mText.size() && index > 0) {
        cursor = detail::nextCodePoint(mText, cursor);
        --index;
    }
    return cursor;
}

std::size_t TextEdit::codePointIndexOf(std::size_t offset) const noexcept {
    return countCodePoints(std::string_view{mText}.substr(0, clampBoundary(mText, offset)));
}

std::string TextEdit::masked() const { return std::string(length(), '*'); }

} // namespace bul

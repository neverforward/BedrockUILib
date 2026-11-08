#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "bul/Canvas.h"
#include "bul/Color.h"
#include "bul/Geometry.h"
#include "bul/TextMetrics.h"

namespace bul::detail {

/// UTF-8 slice helper shared by the text-drawing widgets.
[[nodiscard]] inline std::string_view slice(std::string_view text, std::size_t begin, std::size_t end) {
    return text.substr(begin, end - begin);
}

/// Decodes the UTF-8 code point that starts at `offset`.
[[nodiscard]] inline char32_t codePointAt(std::string_view text, std::size_t offset) noexcept {
    auto const lead = static_cast<unsigned char>(text[offset]);
    if (lead < 0x80u) {
        return lead;
    }
    std::size_t const left = text.size() - offset;
    if ((lead & 0xE0u) == 0xC0u && left >= 2) {
        return static_cast<char32_t>(((lead & 0x1Fu) << 6) | (static_cast<unsigned char>(text[offset + 1]) & 0x3Fu));
    }
    if ((lead & 0xF0u) == 0xE0u && left >= 3) {
        return static_cast<char32_t>(
            ((lead & 0x0Fu) << 12) | ((static_cast<unsigned char>(text[offset + 1]) & 0x3Fu) << 6)
            | (static_cast<unsigned char>(text[offset + 2]) & 0x3Fu)
        );
    }
    if ((lead & 0xF8u) == 0xF0u && left >= 4) {
        return static_cast<char32_t>(
            ((lead & 0x07u) << 18) | ((static_cast<unsigned char>(text[offset + 1]) & 0x3Fu) << 12)
            | ((static_cast<unsigned char>(text[offset + 2]) & 0x3Fu) << 6)
            | (static_cast<unsigned char>(text[offset + 3]) & 0x3Fu)
        );
    }
    return lead;
}

/// Code points drawn from the "wide" fallback font (CJK, Hangul, fullwidth
/// forms). The engine measures them narrower than the glyphs it draws, so they
/// need their own advance.
[[nodiscard]] inline bool isWide(char32_t codePoint) noexcept {
    if (codePoint < 0x1100) {
        return false;
    }
    return (codePoint <= 0x115F)                        // Hangul Jamo
        || (codePoint >= 0x2E80 && codePoint <= 0x303E) // CJK radicals, punct.
        || (codePoint >= 0x3041 && codePoint <= 0x33FF) // kana, CJK compat
        || (codePoint >= 0x3400 && codePoint <= 0x4DBF) // CJK extension A
        || (codePoint >= 0x4E00 && codePoint <= 0x9FFF) // CJK unified
        || (codePoint >= 0xA000 && codePoint <= 0xA4CF) // Yi
        || (codePoint >= 0xAC00 && codePoint <= 0xD7A3) // Hangul syllables
        || (codePoint >= 0xF900 && codePoint <= 0xFAFF) // CJK compat ideographs
        || (codePoint >= 0xFE30 && codePoint <= 0xFE4F) // CJK compat forms
        || (codePoint >= 0xFF00 && codePoint <= 0xFF60) // fullwidth forms
        || (codePoint >= 0xFFE0 && codePoint <= 0xFFE6) || (codePoint >= 0x1F300 && codePoint <= 0x1FAFF) // emoji
        || (codePoint >= 0x20000 && codePoint <= 0x3FFFD); // CJK extension B+
}

/// Distance from one code point to the next in the widget's own layout.
///
/// A wide code point advances at least one line height. The engine reports a
/// narrower number for them, while the unicode fallback it actually draws with
/// is a square glyph, so trusting the measurement makes text wider than its box
/// and leaves a caret inside the text instead of at its end.
[[nodiscard]] inline float advanceOf(Canvas const& canvas, float scale, std::string_view codePoint) noexcept {
    float const measured = canvas.textWidth(codePoint, scale);
    if (!isWide(codePointAt(codePoint, 0))) {
        return measured;
    }
    return std::max(measured, std::max(canvas.lineHeight(scale) - 1.f, 0.f));
}

/// Per-code-point positions of one display string. Every drawing and caret
/// decision of a text widget goes through this table, so they cannot disagree
/// with each other even when the engine's own measurement is off for some
/// glyphs.
struct GlyphFrame {
    std::vector<std::size_t> offsets; // byte offset of each code point; last = size
    std::vector<float>       prefix;  // width before that code point
};

[[nodiscard]] inline GlyphFrame buildFrame(Canvas const& canvas, float scale, std::string_view text) {
    GlyphFrame  frame;
    std::size_t offset = 0;
    frame.offsets.push_back(0);
    frame.prefix.push_back(0.f);
    while (offset < text.size()) {
        std::size_t const next = nextCodePoint(text, offset);
        frame.prefix.push_back(frame.prefix.back() + advanceOf(canvas, scale, text.substr(offset, next - offset)));
        frame.offsets.push_back(next);
        offset = next;
    }
    return frame;
}

[[nodiscard]] inline float frameWidth(GlyphFrame const& frame) noexcept { return frame.prefix.back(); }

/// Longest prefix of `text` that fits in `maxWidth`, with a trailing ellipsis
/// when the whole text does not fit. Uses the same advance table as the drawing.
[[nodiscard]] inline std::string fitText(std::string_view text, float maxWidth, float scale, Canvas const& canvas) {
    if (maxWidth <= 0.f) {
        return {};
    }
    GlyphFrame const frame    = buildFrame(canvas, scale, text);
    float const      ellipsis = canvas.textWidth("...", scale);
    if (frameWidth(frame) <= maxWidth) {
        return std::string{text};
    }
    if (ellipsis > maxWidth) {
        return {};
    }
    std::string out;
    for (std::size_t index = 0; index + 1 < frame.offsets.size(); ++index) {
        if (frame.prefix[index + 1] + ellipsis > maxWidth) {
            break;
        }
        out.append(text.substr(frame.offsets[index], frame.offsets[index + 1] - frame.offsets[index]));
    }
    out.append("...");
    return out;
}

/// Draws code points [begin, end) of `text` at `prefix` positions starting at
/// `origin`. Runs of narrow glyphs are drawn as one string (the engine lays them
/// out exactly as measured); a wide glyph is placed on its own, because only its
/// cell position is known.
inline void drawFragment(
    Canvas&                         canvas,
    std::string const&              text,
    std::vector<std::size_t> const& offsets,
    std::vector<float> const&       prefix,
    std::size_t                     begin,
    std::size_t                     end,
    Point                           origin,
    float                           scale,
    Rgba                            color
) {
    std::size_t index = begin;
    while (index < end) {
        std::size_t runEnd = index;
        if (isWide(codePointAt(text, offsets[index]))) {
            runEnd = index + 1;
        } else {
            while (runEnd < end && !isWide(codePointAt(text, offsets[runEnd]))) {
                ++runEnd;
            }
        }
        float const x     = origin.x + prefix[index];
        float const width = prefix[runEnd] - prefix[index];
        canvas
            .text(slice(text, offsets[index], offsets[runEnd]), {x, origin.y}, width + 2.f, color, scale, Align::Left);
        index = runEnd;
    }
}

/// Draws `text` cut to `maxWidth` (ellipsis included) with the advance table.
inline void
drawFittedText(Canvas& canvas, std::string_view text, Point origin, float maxWidth, float scale, Rgba color) {
    std::string const shown = fitText(text, maxWidth, scale, canvas);
    if (shown.empty()) {
        return;
    }
    GlyphFrame const frame = buildFrame(canvas, scale, shown);
    drawFragment(canvas, shown, frame.offsets, frame.prefix, 0, frame.offsets.size() - 1, origin, scale, color);
}

} // namespace bul::detail

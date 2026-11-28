#pragma once

#include <string>
#include <string_view>

#include "bul/Canvas.h"

namespace bul {

/// Minecraft text formatting codes: `§l` bold, `§o` italic, `§k` obfuscated, and
/// `§r` to reset. The engine parses them inside a string, so styling is a matter
/// of wrapping the run rather than asking the font for a face (there is no bold
/// or oblique face to ask for).
namespace format {

constexpr char const* kSection = "\xC2\xA7"; // U+00A7 SECTION SIGN, UTF-8
constexpr char const* kReset   = "\xC2\xA7r";

[[nodiscard]] inline bool active(bool bold, bool italic, bool obfuscated) noexcept {
    return bold || italic || obfuscated;
}

[[nodiscard]] inline std::string apply(std::string_view text, bool bold, bool italic, bool obfuscated) {
    if (!active(bold, italic, obfuscated) || text.empty()) {
        return std::string{text};
    }
    std::string out;
    out.reserve(text.size() + 8);
    if (bold) {
        out += kSection;
        out += 'l';
    }
    if (italic) {
        out += kSection;
        out += 'o';
    }
    if (obfuscated) {
        out += kSection;
        out += 'k';
    }
    out += text;
    out += kReset;
    return out;
}

} // namespace format

/// A canvas that wraps every drawn string in the formatting codes of a style, so
/// a `font-weight: bold` on a container reaches every text run inside it without
/// each component knowing about the codes.
class FormatCanvas final : public Canvas {
public:
    FormatCanvas(Canvas& inner, bool bold, bool italic, bool obfuscated)
    : mInner(inner),
      mBold(bold),
      mItalic(italic),
      mObfuscated(obfuscated) {}

    void fill(Rect const& rect, Rgba color, float alpha = 1.f) override { mInner.fill(rect, color, alpha); }

    void outline(Rect const& rect, Rgba color, float thickness = 1.f) override {
        mInner.outline(rect, color, thickness);
    }

    void text(std::string_view text, Point topLeft, float boxWidth, Rgba color, float size, Align align = Align::Left)
        override {
        mInner.text(format::apply(text, mBold, mItalic, mObfuscated), topLeft, boxWidth, color, size, align);
    }

    [[nodiscard]] float textWidth(std::string_view text, float size) const override {
        return mInner.textWidth(text, size);
    }

    [[nodiscard]] float lineHeight(float size) const override { return mInner.lineHeight(size); }

    void pushClip(Rect const& rect) override { mInner.pushClip(rect); }
    void popClip() override { mInner.popClip(); }
    void flush() override { mInner.flush(); }

private:
    Canvas& mInner;
    bool    mBold{};
    bool    mItalic{};
    bool    mObfuscated{};
};

} // namespace bul

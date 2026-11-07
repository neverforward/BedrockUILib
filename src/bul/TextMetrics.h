#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace bul {

/// Text measurement, behind an interface so layout and truncation can be
/// unit-tested with a fake implementation instead of the engine font.
class TextMetrics {
public:
    TextMetrics()                              = default;
    TextMetrics(TextMetrics const&)            = delete;
    TextMetrics& operator=(TextMetrics const&) = delete;
    virtual ~TextMetrics()                     = default;

    /// Advance width of `text` at `scale`, in GUI units.
    [[nodiscard]] virtual float width(std::string_view text, float scale = 1.f) const = 0;

    /// Height of one line at `scale`, in GUI units.
    [[nodiscard]] virtual float lineHeight(float scale = 1.f) const = 0;
};

namespace detail {

/// Offset of the next UTF-8 code point after `i`. Never splits a sequence.
[[nodiscard]] constexpr std::size_t nextCodePoint(std::string_view text, std::size_t i) noexcept {
    if (i >= text.size()) {
        return text.size();
    }
    std::size_t next = i + 1;
    while (next < text.size() && (static_cast<unsigned char>(text[next]) & 0xC0u) == 0x80u) {
        ++next;
    }
    return next;
}

} // namespace detail

/// Longest prefix of `text` that fits in `maxWidth` once `ellipsis` is appended,
/// cut on a code point boundary. Returns an empty string when not even the
/// ellipsis fits.
[[nodiscard]] inline std::string truncateToWidth(
    TextMetrics const& metrics,
    std::string_view   text,
    float              maxWidth,
    float              scale    = 1.f,
    std::string_view   ellipsis = "..."
) {
    if (maxWidth <= 0.f) {
        return {};
    }
    if (metrics.width(text, scale) <= maxWidth) {
        return std::string{text};
    }
    if (metrics.width(ellipsis, scale) > maxWidth) {
        return {};
    }
    std::size_t      fitted = 0;
    std::string_view result{};
    for (std::size_t i = 0; i < text.size();) {
        std::size_t const next = detail::nextCodePoint(text, i);
        if (metrics.width(text.substr(0, next), scale) + metrics.width(ellipsis, scale) > maxWidth) {
            break;
        }
        fitted = next;
        i      = next;
    }
    result = text.substr(0, fitted);
    std::string out{result};
    out.append(ellipsis);
    return out;
}

} // namespace bul

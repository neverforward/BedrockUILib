#include "gtest/gtest.h"

#include "bul/TextMetrics.h"

#include <cstddef>
#include <string_view>

namespace {

/// Fake metrics: 6 units per ASCII code point, 12 per multi-byte one, times the
/// scale. Good enough to pin down truncation behaviour without the engine font.
class FakeMetrics final : public bul::TextMetrics {
public:
    [[nodiscard]] float width(std::string_view text, float scale = 1.f) const override {
        float total = 0.f;
        for (std::size_t i = 0; i < text.size();) {
            std::size_t const next = bul::detail::nextCodePoint(text, i);
            total += (next - i >= 3) ? 12.f : 6.f;
            i = next;
        }
        return total * scale;
    }

    [[nodiscard]] float lineHeight(float scale = 1.f) const override { return 9.f * scale; }
};

} // namespace

TEST(TextMetrics, fitsUnchanged) {
    FakeMetrics const m;
    EXPECT_EQ(bul::truncateToWidth(m, "abc", 18.f), "abc");
    EXPECT_EQ(bul::truncateToWidth(m, "", 100.f), "");
}

TEST(TextMetrics, truncatesAsciiAndAppendsEllipsis) {
    FakeMetrics const m;
    // "..." is 18 units, leaving 6 for one character at maxWidth 24.
    EXPECT_EQ(bul::truncateToWidth(m, "abcdef", 24.f), "a...");
}

TEST(TextMetrics, neverSplitsMultibyteCodePoints) {
    FakeMetrics const m;
    // Each CJK character is 12 units; "..." is 18, so one character fits in 30.
    std::string const cjk = "\xE4\xB8\xAD\xE6\x96\x87\xE5\xB1\x8F\xE5\xB9\x95";
    std::string const out = bul::truncateToWidth(m, cjk, 30.f);
    EXPECT_EQ(out, "\xE4\xB8\xAD...");
    // The result is valid UTF-8: the cut is on a boundary, not inside a glyph.
    EXPECT_EQ(out.size(), 3u + 3u);
}

TEST(TextMetrics, emptyWhenEllipsisDoesNotFit) {
    FakeMetrics const m;
    EXPECT_EQ(bul::truncateToWidth(m, "abcdef", 17.f), "");
    EXPECT_EQ(bul::truncateToWidth(m, "abcdef", 0.f), "");
    EXPECT_EQ(bul::truncateToWidth(m, "abcdef", -5.f), "");
}

TEST(TextMetrics, scaleChangesTheBudget) {
    FakeMetrics const m;
    EXPECT_EQ(bul::truncateToWidth(m, "abcdef", 48.f, 2.f), "a...");
    EXPECT_EQ(bul::truncateToWidth(m, "abcdef", 120.f, 2.f), "abcdef");
}

TEST(TextMetrics, customEllipsis) {
    FakeMetrics const m;
    EXPECT_EQ(bul::truncateToWidth(m, "abcdef", 30.f, 1.f, "~"), "abcd~");
}

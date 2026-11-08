#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "bul/Canvas.h"
#include "bul/TextMetrics.h"

namespace bul::test {

/// Canvas that records what was painted and gives deterministic metrics: one
/// code point is 6 units wide and one line is 9 units high at scale 1. Code
/// points, not bytes, so a Chinese character behaves like one glyph.
class FakeCanvas final : public Canvas {
public:
    struct Fill {
        Rect  rect;
        Rgba  color;
        float alpha;
    };
    struct Stroke {
        Rect  rect;
        Rgba  color;
        float thickness;
    };
    struct Text {
        std::string value;
        Point       topLeft;
        float       boxWidth;
        float       size;
        Align       align;
        Rgba        color;
    };

    std::vector<Fill>   fills;
    std::vector<Stroke> strokes;
    std::vector<Text>   texts;
    int                 flushes{};

    void fill(Rect const& rect, Rgba color, float alpha) override { fills.push_back({rect, color, alpha}); }

    void outline(Rect const& rect, Rgba color, float thickness) override {
        strokes.push_back({rect, color, thickness});
    }

    void text(std::string_view value, Point topLeft, float boxWidth, Rgba color, float size, Align align) override {
        texts.push_back({std::string{value}, topLeft, boxWidth, size, align, color});
    }

    [[nodiscard]] float textWidth(std::string_view value, float size) const override {
        return static_cast<float>(codePoints(value)) * 6.f * size;
    }

    [[nodiscard]] float lineHeight(float size) const override { return 9.f * size; }

    void flush() override { ++flushes; }

private:
    [[nodiscard]] static std::size_t codePoints(std::string_view value) {
        std::size_t count = 0;
        for (std::size_t i = 0; i < value.size(); i = bul::detail::nextCodePoint(value, i)) {
            ++count;
        }
        return count;
    }
};

} // namespace bul::test

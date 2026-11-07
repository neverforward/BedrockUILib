#include "gtest/gtest.h"

#include "bul/Theme.h"
#include "bul/Widget.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

/// Records what the widget tree paints, and gives deterministic text metrics so
/// layout numbers are stable in tests.
class FakeCanvas final : public bul::Canvas {
public:
    struct Fill {
        bul::Rect rect;
        bul::Rgba color;
        float     alpha;
    };
    struct Stroke {
        bul::Rect rect;
    };
    struct Text {
        std::string   value;
        bul::Point    topLeft;
        float         boxWidth;
        float         size;
        bul::Align    align;
    };

    std::vector<Fill>   fills;
    std::vector<Stroke> strokes;
    std::vector<Text>   texts;
    int                 flushes{};

    void fill(bul::Rect const& rect, bul::Rgba color, float alpha) override { fills.push_back({rect, color, alpha}); }
    void outline(bul::Rect const& rect, bul::Rgba, float) override { strokes.push_back({rect}); }
    void text(std::string_view value, bul::Point topLeft, float boxWidth, bul::Rgba, float size, bul::Align align)
        override {
        texts.push_back({std::string{value}, topLeft, boxWidth, size, align});
    }
    [[nodiscard]] float textWidth(std::string_view value, float size) const override {
        return static_cast<float>(value.size()) * 6.f * size;
    }
    [[nodiscard]] float lineHeight(float size) const override { return 9.f * size; }
    void                 flush() override { ++flushes; }
};

bul::Theme testTheme() {
    bul::Theme theme   = bul::Theme::neutral();
    theme.fontScale    = 1.f;
    theme.padding      = 4.f;
    theme.gap          = 3.f;
    return theme;
}

} // namespace

TEST(WidgetLayout, labelMeasuresText) {
    FakeCanvas canvas;
    bul::Theme theme = testTheme();
    bul::Label label{"abc"};
    bul::Size  size = label.measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 18.f);
    EXPECT_FLOAT_EQ(size.height, 9.f);
}

TEST(WidgetLayout, buttonAddsPadding) {
    FakeCanvas canvas;
    bul::Theme theme = testTheme();
    bul::Button button{"ok"};
    bul::Size   size = button.measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 12.f + 8.f);
    EXPECT_FLOAT_EQ(size.height, 9.f + 4.f);
}

TEST(WidgetLayout, columnStacksWithGap) {
    FakeCanvas canvas;
    bul::Theme theme = testTheme();
    bul::Stack column{bul::Stack::Direction::Vertical};
    column.add(std::make_unique<bul::Label>("a"));
    column.add(std::make_unique<bul::Label>("b"));

    bul::Size const size = column.measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 9.f + 3.f + 9.f);

    column.layout({0.f, 0.f, 100.f, 21.f}, theme, canvas);

    // First child at y=0, second at y=12 (9 + gap 3).
    bul::Widget* hit = column.hitTest({1.f, 13.f});
    ASSERT_NE(hit, nullptr);
    EXPECT_FLOAT_EQ(hit->bounds().y, 12.f);
}

TEST(WidgetLayout, rowLaysOutHorizontally) {
    FakeCanvas canvas;
    bul::Theme theme = testTheme();
    bul::Stack row{bul::Stack::Direction::Horizontal};
    row.add(std::make_unique<bul::Button>("aa"));
    row.add(std::make_unique<bul::Button>("bbb"));

    bul::Size const size = row.measure(200.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 9.f + 4.f);
    EXPECT_FLOAT_EQ(size.width, (12.f + 8.f) + 3.f + (18.f + 8.f));

    row.layout({0.f, 0.f, size.width, size.height}, theme, canvas);
    bul::Widget* second = row.hitTest({25.f, 2.f});
    ASSERT_NE(second, nullptr);
    EXPECT_FLOAT_EQ(second->bounds().x, 23.f);
}

TEST(WidgetLayout, clickOutsideDoesNotHit) {
    FakeCanvas canvas;
    bul::Theme theme = testTheme();
    bul::Stack column{bul::Stack::Direction::Vertical};
    column.add(std::make_unique<bul::Button>("ok"));
    column.layout({10.f, 10.f, 100.f, 40.f}, theme, canvas);
    EXPECT_EQ(column.hitTest({-5.f, -5.f}), nullptr);
    EXPECT_NE(column.hitTest({12.f, 12.f}), nullptr);
}

TEST(WidgetLayout, paintingGoesThroughCanvas) {
    FakeCanvas canvas;
    bul::Theme theme = testTheme();
    bul::Button button{"ok"};
    button.layout({0.f, 0.f, 40.f, 20.f}, theme, canvas);
    button.paint(canvas, theme);
    EXPECT_EQ(canvas.fills.size(), 1u);
    EXPECT_EQ(canvas.strokes.size(), 1u);
    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].value, "ok");
    EXPECT_EQ(canvas.texts[0].align, bul::Align::Center);
}

TEST(Theme, neutralDefaultsAreNeutral) {
    bul::Theme const theme = bul::Theme::neutral();
    EXPECT_FLOAT_EQ(theme.scrim, 0.22f);
    EXPECT_FLOAT_EQ(theme.fontScale, 1.f);
    EXPECT_GT(theme.gap, 0.f);
    EXPECT_GT(theme.padding, 0.f);
}

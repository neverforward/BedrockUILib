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
        bul::Rgba     color;
    };

    std::vector<Fill>   fills;
    std::vector<Stroke> strokes;
    std::vector<Text>   texts;
    int                 flushes{};

    void fill(bul::Rect const& rect, bul::Rgba color, float alpha) override { fills.push_back({rect, color, alpha}); }
    void outline(bul::Rect const& rect, bul::Rgba, float) override { strokes.push_back({rect}); }
    void text(std::string_view value, bul::Point topLeft, float boxWidth, bul::Rgba color, float size, bul::Align align)
        override {
        texts.push_back({std::string{value}, topLeft, boxWidth, size, align, color});
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

TEST(WidgetLayout, buttonUsesSizeProfile) {
    FakeCanvas canvas;
    bul::Theme theme = testTheme();

    // Default: 10 px horizontal padding, 5 px vertical padding around one line.
    bul::Button standard{"ok"};
    bul::Size   size = standard.measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 12.f + 20.f);
    EXPECT_FLOAT_EQ(size.height, 9.f + 10.f);

    bul::Button tiny{"ok"};
    tiny.size(bul::Button::Size::XSmall);
    bul::Size const tinySize = tiny.measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(tinySize.width, 9.6f + 12.f);
    EXPECT_FLOAT_EQ(tinySize.height, 7.2f + 2.f);

    bul::Button large{"ok"};
    large.size(bul::Button::Size::Large);
    bul::Size const largeSize = large.measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(largeSize.width, 13.8f + 28.f);

    EXPECT_LT(tinySize.height, size.height);
    EXPECT_LT(size.height, largeSize.height);
}

TEST(WidgetLayout, buttonTypesChooseSurface) {
    FakeCanvas canvas;
    bul::Theme theme = testTheme();

    bul::Button outline{"ok"};
    outline.type(bul::Button::Type::Outline);
    outline.layout({0.f, 0.f, 40.f, 20.f}, theme, canvas);
    outline.paint(canvas, theme);
    EXPECT_EQ(canvas.fills.size(), 0u); // transparent until hovered

    FakeCanvas linkCanvas;
    bul::Button link{"ok"};
    link.type(bul::Button::Type::Link);
    link.layout({0.f, 0.f, 40.f, 20.f}, theme, linkCanvas);
    link.paint(linkCanvas, theme);
    EXPECT_EQ(linkCanvas.fills.size(), 0u);
    EXPECT_EQ(linkCanvas.strokes.size(), 0u); // text only

    FakeCanvas destructiveCanvas;
    bul::Button destructive{"ok"};
    destructive.type(bul::Button::Type::Destructive);
    destructive.layout({0.f, 0.f, 40.f, 20.f}, theme, destructiveCanvas);
    destructive.paint(destructiveCanvas, theme);
    ASSERT_EQ(destructiveCanvas.fills.size(), 1u);
    EXPECT_EQ(destructiveCanvas.fills[0].color, theme.destructive);

    FakeCanvas secondaryCanvas;
    bul::Button secondary{"ok"};
    secondary.type(bul::Button::Type::Secondary);
    secondary.layout({0.f, 0.f, 40.f, 20.f}, theme, secondaryCanvas);
    secondary.paint(secondaryCanvas, theme);
    ASSERT_EQ(secondaryCanvas.fills.size(), 1u);
    EXPECT_EQ(secondaryCanvas.fills[0].color, theme.secondary);

    // Default is the primary action: a filled primary surface with its own text
    // colour, so it cannot be confused with the quiet secondary one.
    FakeCanvas primaryCanvas;
    bul::Button standard{"ok"};
    standard.layout({0.f, 0.f, 40.f, 20.f}, theme, primaryCanvas);
    standard.paint(primaryCanvas, theme);
    ASSERT_EQ(primaryCanvas.fills.size(), 1u);
    EXPECT_EQ(primaryCanvas.fills[0].color, theme.primary);
    ASSERT_EQ(primaryCanvas.texts.size(), 1u);
    EXPECT_EQ(primaryCanvas.texts[0].color, theme.primaryText);
    EXPECT_NE(theme.primary, theme.secondary);
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
    EXPECT_FLOAT_EQ(size.height, 9.f + 10.f);
    EXPECT_FLOAT_EQ(size.width, (12.f + 20.f) + 3.f + (18.f + 20.f));

    row.layout({0.f, 0.f, size.width, size.height}, theme, canvas);
    bul::Widget* second = row.hitTest({40.f, 2.f});
    ASSERT_NE(second, nullptr);
    EXPECT_FLOAT_EQ(second->bounds().x, 35.f);
}

TEST(WidgetLayout, rowKeepsChildHeights) {
    FakeCanvas canvas;
    bul::Theme theme = testTheme();

    auto tree   = std::make_unique<bul::Stack>(bul::Stack::Direction::Horizontal);
    auto small  = std::make_unique<bul::Button>("xs");
    small->size(bul::Button::Size::XSmall);
    auto large = std::make_unique<bul::Button>("lg");
    large->size(bul::Button::Size::Large);
    bul::Button* smallRaw = small.get();
    bul::Button* largeRaw = large.get();
    tree->add(std::move(small));
    tree->add(std::move(large));

    bul::Size const wanted = tree->measure(200.f, theme, canvas);
    tree->layout({0.f, 0.f, wanted.width, wanted.height}, theme, canvas);

    // Different sizes must keep their own height inside a row.
    EXPECT_FLOAT_EQ(smallRaw->bounds().height, 7.2f + 2.f);
    EXPECT_FLOAT_EQ(largeRaw->bounds().height, 10.35f + 16.f);
    EXPECT_LT(smallRaw->bounds().height, largeRaw->bounds().height);
    EXPECT_FLOAT_EQ(tree->bounds().height, wanted.height);
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

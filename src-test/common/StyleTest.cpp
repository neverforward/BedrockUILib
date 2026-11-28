#include "gtest/gtest.h"

#include <memory>

#include "FakeCanvas.h"

#include "bul/Theme.h"
#include "bul/Widget.h"

namespace {

[[nodiscard]] bul::Theme testTheme() {
    bul::Theme theme = bul::Theme::neutral();
    theme.fontScale  = 1.f;
    theme.gap        = 6.f;
    return theme;
}

constexpr bul::Rgba kRed   = bul::Rgba::fromRgb(0xE5484D);
constexpr bul::Rgba kGreen = bul::Rgba::fromRgb(0x46A758);

} // namespace

TEST(Style, EmptyByDefault) {
    bul::Style empty;
    EXPECT_TRUE(empty.empty());
    EXPECT_FALSE((bul::Style{.color = kRed}).empty());
}

TEST(Style, OverridesOnlyTheTokensItSets) {
    bul::Theme const base     = testTheme();
    bul::Theme const resolved = bul::applyStyle(base, bul::Style{.accentColor = kGreen});

    EXPECT_EQ(resolved.accent, kGreen);
    EXPECT_EQ(resolved.text, base.text);
    EXPECT_EQ(resolved.input, base.input);
    EXPECT_FLOAT_EQ(resolved.fontScale, base.fontScale);
}

TEST(Style, ColoursASingleWidget) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hello"};
    label.style({.color = kRed});
    label.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].color, kRed);
}

TEST(Style, AContainerStyleReachesItsChildren) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto stack = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    stack->style({.color = kRed});
    stack->add(std::make_unique<bul::Label>("Hello"));
    stack->layout({0.f, 0.f, 100.f, 9.f}, theme, canvas);
    stack->paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].color, kRed);
}

TEST(Style, AChildStyleWinsOverItsContainer) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto stack = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    stack->style({.color = kRed});
    auto label = std::make_unique<bul::Label>("Hello");
    label->style({.color = kGreen});
    stack->add(std::move(label));
    stack->layout({0.f, 0.f, 100.f, 9.f}, theme, canvas);
    stack->paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].color, kGreen);
}

TEST(Style, ColoursAButtonsSurface) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Button button{"OK"};
    button.style({.backgroundColor = kGreen, .color = theme.panel});
    button.layout({0.f, 0.f, 80.f, 19.f}, theme, canvas);
    button.paint(canvas, theme);

    ASSERT_FALSE(canvas.fills.empty());
    EXPECT_EQ(canvas.fills[0].color, kGreen);
}

TEST(Style, ACheckboxUsesTheStyledAccent) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Checkbox box;
    box.checked(true);
    box.style({.accentColor = kGreen});
    box.layout({0.f, 0.f, 100.f, 11.f}, theme, canvas);
    box.paint(canvas, theme);

    bool green = false;
    for (auto const& fill : canvas.fills) {
        green = green || fill.color == kGreen;
    }
    EXPECT_TRUE(green);
}

#include "gtest/gtest.h"

#include <memory>

#include "FakeCanvas.h"

#include "bul/StyleSheet.h"
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
constexpr bul::Rgba kBlue  = bul::Rgba::fromRgb(0x4C8BF5);

/// The active sheet is global; every test leaves it empty again.
struct ResetSheet {
    ~ResetSheet() { bul::setStyleSheet({}); }
};

} // namespace

TEST(StyleSheet, SelectorSpecificity) {
    EXPECT_EQ((bul::Selector{.type = "Button"}).specificity(), 1);
    EXPECT_EQ((bul::Selector{.className = "danger"}).specificity(), 10);
    EXPECT_EQ((bul::Selector{.id = "submit"}).specificity(), 100);
    EXPECT_EQ((bul::Selector{.type = "Button", .className = "danger", .id = "submit"}).specificity(), 111);
}

TEST(StyleSheet, SelectorMatchesWhatItSets) {
    bul::Selector const button{.type = "Button"};
    EXPECT_TRUE(button.matches("Button", "", ""));
    EXPECT_FALSE(button.matches("Label", "", ""));

    bul::Selector const danger{.className = "danger"};
    EXPECT_TRUE(danger.matches("Label", "danger", "x"));
    EXPECT_FALSE(danger.matches("Label", "", "danger"));

    bul::Selector const submit{.id = "submit"};
    EXPECT_TRUE(submit.matches("Input", "x", "submit"));
    EXPECT_FALSE(submit.matches("Input", "submit", ""));

    bul::Selector const any{};
    EXPECT_TRUE(any.matches("Whatever", "class", "id"));
}

TEST(StyleSheet, CascadesBySpecificity) {
    bul::StyleSheet sheet;
    sheet.rule({.type = "Button"}, {.primary = kRed});
    sheet.rule({.className = "danger"}, {.primary = kGreen});
    sheet.rule({.id = "submit"}, {.primary = kBlue});

    EXPECT_EQ(sheet.styleFor("Button", "", "").primary, kRed);
    EXPECT_EQ(sheet.styleFor("Button", "danger", "").primary, kGreen);
    EXPECT_EQ(sheet.styleFor("Button", "danger", "submit").primary, kBlue);
}

TEST(StyleSheet, DeclarationOrderBreaksTies) {
    bul::StyleSheet sheet;
    sheet.rule({.type = "Label"}, {.text = kRed});
    sheet.rule({.type = "Label"}, {.text = kGreen});

    EXPECT_EQ(sheet.styleFor("Label", "", "").text, kGreen);
}

TEST(StyleSheet, ARuleColoursMatchingWidgets) {
    ResetSheet const      reset;
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::StyleSheet sheet;
    sheet.rule({.type = "Checkbox"}, {.accent = kBlue});
    bul::setStyleSheet(std::move(sheet));

    bul::Checkbox box;
    box.checked(true);
    box.layout({0.f, 0.f, 100.f, 11.f}, theme, canvas);
    box.paint(canvas, theme);

    bool blue = false;
    for (auto const& fill : canvas.fills) {
        blue = blue || fill.color == kBlue;
    }
    EXPECT_TRUE(blue);

    // A label is not selected by a Checkbox rule.
    bul::Label label{"Hi"};
    label.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);
    bul::test::FakeCanvas painted;
    label.paint(painted, theme);
    ASSERT_EQ(painted.texts.size(), 1u);
    EXPECT_EQ(painted.texts[0].color, theme.text);
}

TEST(StyleSheet, ClassAndIdSelectTheWidget) {
    ResetSheet const      reset;
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::StyleSheet sheet;
    sheet.rule({.className = "danger"}, {.text = kRed});
    sheet.rule({.id = "title"}, {.text = kBlue});
    bul::setStyleSheet(std::move(sheet));

    bul::Label plain{"Plain"};
    bul::Label danger{"Danger"};
    danger.className("danger");
    bul::Label title{"Title"};
    title.id("title");

    plain.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);
    danger.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);
    title.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);

    bul::test::FakeCanvas painted;
    plain.paint(painted, theme);
    danger.paint(painted, theme);
    title.paint(painted, theme);

    ASSERT_EQ(painted.texts.size(), 3u);
    EXPECT_EQ(painted.texts[0].color, theme.text);
    EXPECT_EQ(painted.texts[1].color, kRed);
    EXPECT_EQ(painted.texts[2].color, kBlue);
}

TEST(StyleSheet, InlineStyleWinsOverTheSheet) {
    ResetSheet const      reset;
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::StyleSheet sheet;
    sheet.rule({.type = "Label"}, {.text = kRed});
    bul::setStyleSheet(std::move(sheet));

    bul::Label label{"Hello"};
    label.style({.text = kGreen});
    label.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].color, kGreen);
}

TEST(StyleSheet, TypeNamesAreTheElementSelectors) {
    bul::Button   button{"OK"};
    bul::Label    label{"Hi"};
    bul::Input    input;
    bul::Stack    stack{bul::Stack::Direction::Vertical};
    bul::Badge    badge{"New"};
    bul::Switch   switchWidget;
    bul::Checkbox box;

    EXPECT_EQ(button.typeName(), "Button");
    EXPECT_EQ(label.typeName(), "Label");
    EXPECT_EQ(input.typeName(), "Input");
    EXPECT_EQ(stack.typeName(), "Stack");
    EXPECT_EQ(badge.typeName(), "Badge");
    EXPECT_EQ(switchWidget.typeName(), "Switch");
    EXPECT_EQ(box.typeName(), "Checkbox");
}

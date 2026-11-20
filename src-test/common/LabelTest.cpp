#include "gtest/gtest.h"

#include <string>

#include "FakeCanvas.h"

#include "bul/Theme.h"
#include "bul/Widget.h"

namespace {

[[nodiscard]] bul::Theme testTheme() {
    bul::Theme theme = bul::Theme::neutral();
    theme.fontScale  = 1.f;
    return theme;
}

} // namespace

TEST(Label, MeasuresOneLine) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label      label{"Hello"};
    bul::Size const size = label.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 30.f); // five code points, six units each
    EXPECT_FLOAT_EQ(size.height, 9.f);
}

TEST(Label, SizeScalesTheText) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hello"};
    label.size(2.f);
    bul::Size const size = label.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 60.f);
    EXPECT_FLOAT_EQ(size.height, 18.f);
}

TEST(Label, AlignsEachLine) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hello"};
    label.align(bul::Align::Center);
    label.layout({0.f, 0.f, 100.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_FLOAT_EQ(canvas.texts[0].topLeft.x, 35.f); // (100 - 30) / 2

    bul::test::FakeCanvas right;
    label.align(bul::Align::Right);
    label.paint(right, theme);
    ASSERT_EQ(right.texts.size(), 1u);
    EXPECT_FLOAT_EQ(right.texts[0].topLeft.x, 70.f); // 100 - 30
}

TEST(Label, WrapsAtSpaces) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"aaaa bbbb cccc"};
    label.wrap(true);
    bul::Size const size = label.measure(60.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 60.f);  // a wrapping label takes its row
    EXPECT_FLOAT_EQ(size.height, 18.f); // two lines

    label.layout({0.f, 0.f, 60.f, 18.f}, theme, canvas);
    label.paint(canvas, theme);
    ASSERT_EQ(canvas.texts.size(), 2u);
    EXPECT_EQ(canvas.texts[0].value, "aaaa bbbb");
    EXPECT_EQ(canvas.texts[1].value, "cccc");
    EXPECT_FLOAT_EQ(canvas.texts[0].topLeft.y, 0.f);
    EXPECT_FLOAT_EQ(canvas.texts[1].topLeft.y, 9.f);
}

TEST(Label, WrapsCjkBetweenCharacters) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"中文测试"};
    label.wrap(true);
    // A wide code point advances one line height minus one, so eight units each:
    // sixteen units of room hold two characters.
    bul::Size const size = label.measure(16.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 16.f);
    EXPECT_FLOAT_EQ(size.height, 18.f);

    label.layout({0.f, 0.f, 16.f, 18.f}, theme, canvas);
    label.paint(canvas, theme);
    ASSERT_EQ(canvas.texts.size(), 4u); // each wide glyph is drawn on its own
    EXPECT_EQ(canvas.texts[0].value, "中");
    EXPECT_EQ(canvas.texts[1].value, "文");
    EXPECT_EQ(canvas.texts[2].value, "测");
    EXPECT_FLOAT_EQ(canvas.texts[1].topLeft.y, 0.f);
    EXPECT_FLOAT_EQ(canvas.texts[2].topLeft.y, 9.f);
}

TEST(Label, ExplicitNewlinesStartNewLines) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label      label{"one\ntwo"};
    bul::Size const size = label.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 18.f);

    label.layout({0.f, 0.f, 60.f, 18.f}, theme, canvas);
    label.paint(canvas, theme);
    ASSERT_EQ(canvas.texts.size(), 2u);
    EXPECT_EQ(canvas.texts[1].value, "two");
}

TEST(Label, MaxLinesElidesTheLastLine) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"aaaa bbbb cccc dddd"};
    label.wrap(true);
    label.maxLines(1);
    label.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_NE(canvas.texts[0].value.find("..."), std::string::npos);
}

TEST(Label, ElidesASingleLineThatDoesNotFit) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hello World"};
    label.layout({0.f, 0.f, 30.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].value, "He..."); // 30 units fit "He" plus the ellipsis

    bul::Label kept{"Hello World"};
    kept.elide(false);
    bul::test::FakeCanvas overflowing;
    kept.layout({0.f, 0.f, 30.f, 9.f}, theme, overflowing);
    kept.paint(overflowing, theme);
    ASSERT_EQ(overflowing.texts.size(), 1u);
    EXPECT_EQ(overflowing.texts[0].value, "Hello World");
}

TEST(Label, VerticalAlignCentersTheBlock) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"aaaa bbbb cccc"};
    label.wrap(true);
    label.verticalAlign(bul::VAlign::Center);
    label.layout({0.f, 0.f, 60.f, 40.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 2u);
    EXPECT_FLOAT_EQ(canvas.texts[0].topLeft.y, 11.f); // (40 - 18) / 2
    EXPECT_FLOAT_EQ(canvas.texts[1].topLeft.y, 20.f);
}

TEST(Label, LineSpacingAddsRoomBetweenLines) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"aaaa bbbb cccc"};
    label.wrap(true);
    label.lineSpacing(4.f);
    bul::Size const size = label.measure(60.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 22.f); // 9 + 4 + 9

    label.layout({0.f, 0.f, 60.f, 22.f}, theme, canvas);
    label.paint(canvas, theme);
    ASSERT_EQ(canvas.texts.size(), 2u);
    EXPECT_FLOAT_EQ(canvas.texts[1].topLeft.y, 13.f);
}

TEST(Label, MutedUsesTheMutedColour) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hello"};
    label.muted();
    label.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].color, theme.textMuted);
}

TEST(Label, MaxWidthCapsTheColumn) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"aaaa bbbb cccc dddd"};
    label.wrap(true);
    label.maxWidth(60.f);
    // The parent offers 200, but the paragraph keeps its own 60-unit column.
    bul::Size const size = label.measure(200.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 60.f);
    EXPECT_FLOAT_EQ(size.height, 18.f);
}
TEST(Label, ColorOverridesMuted) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Alert"};
    label.muted();
    label.color(bul::Rgba::fromRgb(0xE5484D));
    label.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].color, bul::Rgba::fromRgb(0xE5484D));
}
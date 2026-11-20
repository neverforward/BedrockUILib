#include "gtest/gtest.h"

#include <memory>
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

TEST(WidgetBase, FixedSizeOverridesTheContentSize) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hello"}; // the text is 30 x 9
    label.fixedSize(50.f, 20.f);

    bul::Size const size = label.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 50.f);
    EXPECT_FLOAT_EQ(size.height, 20.f);

    label.layout({10.f, 10.f, 200.f, 100.f}, theme, canvas);
    EXPECT_FLOAT_EQ(label.bounds().x, 10.f);
    EXPECT_FLOAT_EQ(label.width(), 50.f);
    EXPECT_FLOAT_EQ(label.height(), 20.f);
}

TEST(WidgetBase, MinAndMaxSizeClampTheContent) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hello"};
    label.minSize(60.f, 0.f);
    EXPECT_FLOAT_EQ(label.measure(0.f, theme, canvas).width, 60.f);

    label.minSize(0.f, 0.f);
    label.maxSize(20.f, 0.f);
    EXPECT_FLOAT_EQ(label.measure(0.f, theme, canvas).width, 20.f);
}

TEST(WidgetBase, MarginSitsOutsideTheFixedSize) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hello"};
    label.margin(2.f);
    label.fixedSize(40.f, 10.f);

    bul::Size const size = label.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 44.f);
    EXPECT_FLOAT_EQ(size.height, 14.f);

    label.layout({0.f, 0.f, 100.f, 100.f}, theme, canvas);
    EXPECT_FLOAT_EQ(label.bounds().x, 2.f);
    EXPECT_FLOAT_EQ(label.width(), 40.f);
}

TEST(WidgetBase, OnClickMakesAnyWidgetClickable) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Click"};
    int        clicks = 0;
    label.onClick([&clicks] { ++clicks; });
    EXPECT_TRUE(label.interactive());

    label.layout({0.f, 0.f, 100.f, 9.f}, theme, canvas);
    label.notifyClick({5.f, 4.f});
    EXPECT_EQ(clicks, 1);

    bul::Label plain{"Click"};
    EXPECT_FALSE(plain.interactive());
}

TEST(WidgetBase, FocusAndBlurRunTheHandlers) {
    bul::Label label{"x"};
    int        focused = 0;
    int        blurred = 0;
    label.onFocus([&focused] { ++focused; });
    label.onBlur([&blurred] { ++blurred; });

    label.notifyFocus(true);
    label.notifyFocus(false);
    EXPECT_EQ(focused, 1);
    EXPECT_EQ(blurred, 1);
}

TEST(WidgetBase, HoverRunsTheHandler) {
    bul::Label label{"x"};
    int        entered = 0;
    int        left    = 0;
    label.onHover([&entered, &left](bool hovered) {
        if (hovered) {
            ++entered;
        } else {
            ++left;
        }
    });

    label.notifyHover(true);
    label.notifyHover(false);
    EXPECT_EQ(entered, 1);
    EXPECT_EQ(left, 1);
}

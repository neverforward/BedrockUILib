#include "gtest/gtest.h"

#include <memory>
#include <string>
#include <vector>

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

TEST(Checkbox, MeasuresTheBoxAndItsLabel) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Checkbox box;
    box.text("Accept");
    bul::Size const size = box.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 11.f);             // the box, taller than one line
    EXPECT_FLOAT_EQ(size.width, 11.f + 6.f + 36.f); // box + gap + six code points

    // A parent can cap it; the label is cut at paint time.
    EXPECT_FLOAT_EQ(box.measure(40.f, theme, canvas).width, 40.f);
}

TEST(Checkbox, ClickTogglesAndReports) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    std::vector<bool> changes;
    bul::Checkbox     box;
    box.text("Accept");
    box.onCheckedChange([&changes](bool value) { changes.push_back(value); });
    box.layout({0.f, 0.f, 100.f, 11.f}, theme, canvas);

    box.activateAt({4.f, 5.f});
    EXPECT_TRUE(box.checked());
    box.activateAt({4.f, 5.f});
    EXPECT_FALSE(box.checked());
    ASSERT_EQ(changes.size(), 2u);
    EXPECT_TRUE(changes[0]);
    EXPECT_FALSE(changes[1]);
}

TEST(Checkbox, ClickingAMixedBoxTicksIt) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Checkbox box;
    box.indeterminate(true);
    box.layout({0.f, 0.f, 100.f, 11.f}, theme, canvas);

    box.activateAt({4.f, 5.f});
    EXPECT_TRUE(box.checked());
    EXPECT_FALSE(box.indeterminate());
}

TEST(Checkbox, SpaceTogglesButReadOnlyAndDisabledDoNot) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Checkbox box;
    box.layout({0.f, 0.f, 100.f, 11.f}, theme, canvas);
    EXPECT_TRUE(box.key({bul::Key::Space, false, false}));
    EXPECT_TRUE(box.checked());

    box.readOnly(true);
    EXPECT_TRUE(box.interactive());
    EXPECT_TRUE(box.focusable());
    box.activate();
    box.key({bul::Key::Space, false, false});
    EXPECT_TRUE(box.checked());

    box.readOnly(false);
    box.disabled(true);
    EXPECT_FALSE(box.interactive());
    EXPECT_FALSE(box.focusable());
    box.activate();
    EXPECT_TRUE(box.checked());
}

TEST(Checkbox, PaintsTheTickAndTheMixedDash) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();
    bul::Checkbox         box;
    box.text("Accept");
    box.layout({0.f, 0.f, 100.f, 11.f}, theme, canvas);

    // Unchecked: a filled square, no frame, no mark.
    bul::test::FakeCanvas plain;
    box.paint(plain, theme);
    ASSERT_FALSE(plain.fills.empty());
    EXPECT_EQ(plain.fills[0].color, theme.button);
    EXPECT_TRUE(plain.strokes.empty());

    // Checked: an accent box with the tick drawn in the panel colour.
    box.checked(true);
    bul::test::FakeCanvas ticked;
    box.paint(ticked, theme);
    EXPECT_EQ(ticked.fills[0].color, theme.accent);
    bool mark = false;
    for (auto const& fill : ticked.fills) {
        mark = mark || fill.color == theme.panel;
    }
    EXPECT_TRUE(mark);

    // Mixed: a single horizontal dash, no tick.
    box.checked(false);
    box.indeterminate(true);
    bul::test::FakeCanvas mixed;
    box.paint(mixed, theme);
    int bars = 0;
    for (auto const& fill : mixed.fills) {
        if (fill.color == theme.panel && fill.rect.height <= 1.f) {
            ++bars;
        }
    }
    EXPECT_EQ(bars, 1);
}

TEST(Checkbox, FieldStatesReachTheBox) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto field = std::make_unique<bul::Field>();
    field->label("Terms");
    auto box = std::make_unique<bul::Checkbox>();
    box->text("I agree");
    bul::Checkbox* raw = box.get();
    field->control(std::move(box));
    field->layout({0.f, 0.f, 160.f, 40.f}, theme, canvas);

    field->invalid(true);
    field->paint(canvas, theme);
    bool tinted = false;
    for (auto const& fill : canvas.fills) {
        tinted = tinted || fill.color == theme.invalid;
    }
    EXPECT_TRUE(tinted);

    field->disabled(true);
    EXPECT_FALSE(raw->interactive());
    EXPECT_FALSE(raw->focusable());
}

TEST(Checkbox, CheckedInvalidUsesTheInvalidColour) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Checkbox box;
    box.checked(true);
    box.invalid(true);
    box.layout({0.f, 0.f, 100.f, 11.f}, theme, canvas);
    box.paint(canvas, theme);

    ASSERT_FALSE(canvas.fills.empty());
    EXPECT_EQ(canvas.fills[0].color, theme.invalid); // a ticked invalid box is red, not a muddy accent
    EXPECT_TRUE(canvas.strokes.empty());
}

TEST(Checkbox, FocusLightensTheBoxWithoutABorder) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Checkbox box;
    box.layout({0.f, 0.f, 100.f, 11.f}, theme, canvas);
    box.paint(canvas, theme);
    EXPECT_TRUE(canvas.strokes.empty()); // no frame at all
    ASSERT_FALSE(canvas.fills.empty());
    EXPECT_EQ(canvas.fills[0].color, theme.button);

    // Focus is a colour change, not a border.
    bul::test::FakeCanvas focused;
    box.setFocused(true);
    box.paint(focused, theme);
    EXPECT_TRUE(focused.strokes.empty());
    ASSERT_FALSE(focused.fills.empty());
    EXPECT_EQ(focused.fills[0].color, theme.buttonHover);
}

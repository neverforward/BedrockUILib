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

[[nodiscard]] std::size_t countColor(bul::test::FakeCanvas const& canvas, bul::Rgba color) {
    std::size_t count = 0;
    for (auto const& fill : canvas.fills) {
        if (fill.color == color) {
            ++count;
        }
    }
    return count;
}

/// The leftmost x of the thumb pixels (they are drawn in theme.text).
[[nodiscard]] float thumbLeft(bul::test::FakeCanvas const& canvas, bul::Theme const& theme) {
    float left = 1e9f;
    for (auto const& fill : canvas.fills) {
        if (fill.color == theme.text) {
            left = std::min(left, fill.rect.x);
        }
    }
    return left;
}

} // namespace

TEST(Switch, MeasuresTheTrackAndItsLabel) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Switch switchWidget;
    switchWidget.text("Wi-Fi");
    bul::Size const size = switchWidget.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 12.f);             // the track is taller than one line
    EXPECT_FLOAT_EQ(size.width, 22.f + 6.f + 30.f); // track + gap + "Wi-Fi"

    EXPECT_FLOAT_EQ(switchWidget.measure(40.f, theme, canvas).width, 40.f);
}

TEST(Switch, ClickTogglesAndReports) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    std::vector<bool> changes;
    bul::Switch       switchWidget;
    switchWidget.text("Wi-Fi");
    switchWidget.onCheckedChange([&changes](bool value) { changes.push_back(value); });
    switchWidget.layout({0.f, 0.f, 120.f, 12.f}, theme, canvas);

    switchWidget.activateAt({11.f, 6.f});
    EXPECT_TRUE(switchWidget.checked());
    switchWidget.activateAt({11.f, 6.f});
    EXPECT_FALSE(switchWidget.checked());
    ASSERT_EQ(changes.size(), 2u);
    EXPECT_TRUE(changes[0]);
    EXPECT_FALSE(changes[1]);
}

TEST(Switch, SpaceTogglesButReadOnlyAndDisabledDoNot) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Switch switchWidget;
    switchWidget.layout({0.f, 0.f, 120.f, 12.f}, theme, canvas);

    EXPECT_TRUE(switchWidget.key({bul::Key::Space, false, false}));
    EXPECT_TRUE(switchWidget.checked());

    switchWidget.readOnly(true);
    EXPECT_TRUE(switchWidget.focusable());
    switchWidget.activate();
    switchWidget.key({bul::Key::Space, false, false});
    EXPECT_TRUE(switchWidget.checked());

    switchWidget.readOnly(false);
    switchWidget.disabled(true);
    EXPECT_FALSE(switchWidget.interactive());
    EXPECT_FALSE(switchWidget.focusable());
    switchWidget.activate();
    EXPECT_TRUE(switchWidget.checked());
}

TEST(Switch, ThumbMovesWithTheState) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Switch switchWidget;
    switchWidget.layout({0.f, 0.f, 120.f, 12.f}, theme, canvas);

    bul::test::FakeCanvas off;
    switchWidget.paint(off, theme);
    EXPECT_FLOAT_EQ(thumbLeft(off, theme), 1.f);
    EXPECT_EQ(countColor(off, theme.button), 12u); // one span per track row

    switchWidget.checked(true);
    bul::test::FakeCanvas on;
    switchWidget.paint(on, theme);
    EXPECT_FLOAT_EQ(thumbLeft(on, theme), 11.f);
    EXPECT_EQ(countColor(on, theme.accent), 12u); // one span per track row
    EXPECT_EQ(countColor(on, theme.button), 0u);
}

TEST(Switch, InvalidTintsTheTrack) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Switch switchWidget;
    switchWidget.invalid(true);
    switchWidget.layout({0.f, 0.f, 120.f, 12.f}, theme, canvas);
    switchWidget.paint(canvas, theme);
    EXPECT_GT(countColor(canvas, theme.invalid), 0u); // red cast on the off track

    switchWidget.checked(true);
    bul::test::FakeCanvas on;
    switchWidget.paint(on, theme);
    EXPECT_EQ(countColor(on, theme.invalid), 12u); // the track itself is red
    EXPECT_EQ(countColor(on, theme.accent), 0u);
}

TEST(Switch, FieldStatesReachTheSwitch) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto field = std::make_unique<bul::Field>();
    field->label("Wi-Fi");
    auto switchWidget = std::make_unique<bul::Switch>();
    switchWidget->text("On");
    bul::Switch* raw = switchWidget.get();
    field->control(std::move(switchWidget));
    field->layout({0.f, 0.f, 160.f, 40.f}, theme, canvas);

    field->disabled(true);
    std::vector<bul::Widget*> focusables;
    raw->collectFocusables(focusables);
    EXPECT_TRUE(focusables.empty());

    field->disabled(false);
    field->invalid(true);
    bul::test::FakeCanvas painted;
    field->paint(painted, theme);
    EXPECT_GT(countColor(painted, theme.invalid), 0u);
}

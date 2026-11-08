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
    theme.gap        = 6.f;
    return theme;
}

[[nodiscard]] std::unique_ptr<bul::RadioGroup> makeGroup(std::string* changed = nullptr) {
    auto group = std::make_unique<bul::RadioGroup>();
    group->add("a", "Alpha");
    group->add("b", "Beta");
    if (changed != nullptr) {
        group->onValueChange([changed](std::string const& value) { *changed = value; });
    }
    return group;
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
/// The radio rows are 11 high with a 6 gap: option 0 at y 0..11, option 1 at 17..28.
constexpr bul::Point kFirst{4.f, 5.f};
constexpr bul::Point kSecond{4.f, 22.f};

} // namespace

TEST(RadioGroup, MeasuresTheDiscsAndLabels) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto            group = makeGroup();
    bul::Size const size  = group->measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 11.f + 6.f + 11.f);
    EXPECT_FLOAT_EQ(size.width, 11.f + 6.f + 30.f); // disc + gap + "Alpha"
}

TEST(RadioGroup, ClickSelectsExactlyOneOption) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    std::string changed;
    auto        group = makeGroup(&changed);
    group->layout({0.f, 0.f, 120.f, 28.f}, theme, canvas);

    bul::Widget* second = group->hitTest(kSecond);
    ASSERT_NE(second, nullptr);
    second->activateAt(kSecond);
    EXPECT_EQ(group->value(), "b");
    EXPECT_EQ(changed, "b");

    bul::Widget* first = group->hitTest(kFirst);
    ASSERT_NE(first, nullptr);
    first->activateAt(kFirst);
    EXPECT_EQ(group->value(), "a");
    EXPECT_EQ(changed, "a");
}

TEST(RadioGroup, ChoosingTheSameOptionAgainDoesNotFire) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    int  changes = 0;
    auto group   = std::make_unique<bul::RadioGroup>();
    group->add("a", "Alpha");
    group->add("b", "Beta");
    group->onValueChange([&changes](std::string const&) { ++changes; });
    group->layout({0.f, 0.f, 120.f, 28.f}, theme, canvas);

    group->hitTest(kFirst)->activateAt(kFirst);
    group->hitTest(kFirst)->activateAt(kFirst);
    EXPECT_EQ(changes, 1);
    EXPECT_EQ(group->value(), "a");
}

TEST(RadioGroup, KeyboardSpaceAndArrows) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto group = makeGroup();
    group->layout({0.f, 0.f, 120.f, 28.f}, theme, canvas);
    bul::Widget* first = group->hitTest(kFirst);

    // With nothing chosen yet, Down picks the first option, then the next one;
    // Up walks back.
    EXPECT_TRUE(first->key({bul::Key::Down, false, false}));
    EXPECT_EQ(group->value(), "a");
    EXPECT_TRUE(first->key({bul::Key::Down, false, false}));
    EXPECT_EQ(group->value(), "b");
    EXPECT_TRUE(first->key({bul::Key::Up, false, false}));
    EXPECT_EQ(group->value(), "a");

    // Space picks the focused option.
    EXPECT_TRUE(first->key({bul::Key::Space, false, false}));
    EXPECT_EQ(group->value(), "a");
}

TEST(RadioGroup, DisabledGroupIgnoresEverything) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto group = makeGroup();
    group->disabled(true);
    group->layout({0.f, 0.f, 120.f, 28.f}, theme, canvas);

    bul::Widget* first = group->hitTest(kFirst);
    ASSERT_NE(first, nullptr);
    EXPECT_FALSE(first->interactive());
    EXPECT_FALSE(first->focusable());
    first->activateAt(kFirst);
    EXPECT_EQ(group->value(), "");

    std::vector<bul::Widget*> focusables;
    group->collectFocusables(focusables);
    EXPECT_TRUE(focusables.empty());
}

TEST(RadioGroup, ReadOnlyKeepsFocusButBlocksChanges) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto group = makeGroup();
    group->readOnly(true);
    group->layout({0.f, 0.f, 120.f, 28.f}, theme, canvas);

    bul::Widget* first = group->hitTest(kFirst);
    ASSERT_NE(first, nullptr);
    EXPECT_TRUE(first->focusable());
    first->activateAt(kFirst);
    EXPECT_EQ(group->value(), "");
}

TEST(RadioGroup, ArrowsSkipDisabledOptions) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto group = std::make_unique<bul::RadioGroup>();
    group->add("a", "Alpha");
    group->add("b", "Beta", true);
    group->add("c", "Gamma");
    group->value("a");
    group->layout({0.f, 0.f, 120.f, 45.f}, theme, canvas);

    bul::Widget* first = group->hitTest(kFirst);
    EXPECT_TRUE(first->key({bul::Key::Down, false, false}));
    EXPECT_EQ(group->value(), "c");
    EXPECT_TRUE(first->key({bul::Key::Down, false, false}));
    EXPECT_EQ(group->value(), "a"); // wraps around
}

TEST(RadioGroup, PaintsTheDotOnlyWhenChosen) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto group = makeGroup();
    group->layout({0.f, 0.f, 120.f, 28.f}, theme, canvas);
    group->paint(canvas, theme);

    // Unchosen: the disc, and nothing in the middle.
    EXPECT_GT(countColor(canvas, theme.button), 60u);
    EXPECT_EQ(countColor(canvas, theme.panel), 0u);

    group->value("b");
    bul::test::FakeCanvas chosen;
    group->paint(chosen, theme);
    EXPECT_GT(countColor(chosen, theme.accent), 60u); // the filled disc
    EXPECT_GT(countColor(chosen, theme.panel), 10u);  // the dot in the middle
}

TEST(RadioGroup, InvalidTintsTheDisc) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto group = makeGroup();
    group->invalid(true);
    group->layout({0.f, 0.f, 120.f, 28.f}, theme, canvas);
    group->paint(canvas, theme);

    // No choice yet: every disc carries the error tint.
    EXPECT_GT(countColor(canvas, theme.invalid), 60u);

    // Chosen and invalid: the disc itself is the invalid colour, not the accent.
    group->value("a");
    bul::test::FakeCanvas chosenInvalid;
    group->paint(chosenInvalid, theme);
    EXPECT_GT(countColor(chosenInvalid, theme.invalid), 60u);
    EXPECT_EQ(countColor(chosenInvalid, theme.accent), 0u);
}

TEST(RadioGroup, HorizontalLaysTheOptionsInARow) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto group = makeGroup();
    group->direction(bul::Stack::Direction::Horizontal);
    bul::Size const size = group->measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 11.f);
    // "Alpha" is 5 code points (30 wide) and "Beta" 4 (24): 47 + 6 + 41.
    EXPECT_FLOAT_EQ(size.width, 47.f + 6.f + 41.f);

    group->layout({0.f, 0.f, size.width, size.height}, theme, canvas);
    // The second option starts after the first one plus the gap.
    bul::Widget* second = group->hitTest({60.f, 5.f});
    ASSERT_NE(second, nullptr);
    second->activateAt({60.f, 5.f});
    EXPECT_EQ(group->value(), "b");
}

TEST(RadioGroup, FieldStatesReachTheGroup) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto field = std::make_unique<bul::Field>();
    field->label("Plan");
    auto group = std::make_unique<bul::RadioGroup>();
    group->add("a", "Alpha");
    group->add("b", "Beta");
    bul::RadioGroup* raw = group.get();
    field->control(std::move(group));
    field->layout({0.f, 0.f, 160.f, 60.f}, theme, canvas);

    field->disabled(true);
    std::vector<bul::Widget*> focusables;
    raw->collectFocusables(focusables);
    EXPECT_TRUE(focusables.empty());

    field->disabled(false);
    field->invalid(true);
    bul::test::FakeCanvas painted;
    field->paint(painted, theme);
    bool tinted = false;
    for (auto const& fill : painted.fills) {
        tinted = tinted || fill.color == theme.invalid;
    }
    EXPECT_TRUE(tinted);
}

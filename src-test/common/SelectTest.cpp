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

/// A select with three options, laid out at a known place.
[[nodiscard]] std::unique_ptr<bul::Select> makeSelect(std::string* changed = nullptr) {
    auto select = std::make_unique<bul::Select>();
    select->placeholder("Choose");
    select->item("apple", "Apple");
    select->item("banana", "Banana");
    select->item("cherry", "Cherry");
    if (changed != nullptr) {
        select->onValueChange([changed](std::string const& value) { *changed = value; });
    }
    return select;
}

} // namespace

TEST(Select, MeasuresLikeAField) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Select     select;
    bul::Size const size = select.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 19.f); // line 9 + 2 * padding 4 + 2 * border 1
    EXPECT_FLOAT_EQ(size.width, 120.f);

    // Like an input, the width follows the placeholder, not the value or the
    // option list; a parent can still cap it.
    select.placeholder(std::string(30, 'x'));
    EXPECT_GT(select.measure(0.f, theme, canvas).width, 120.f);
    EXPECT_FLOAT_EQ(select.measure(90.f, theme, canvas).width, 90.f);

    // Selecting a long option does not resize the field.
    float const wide = select.measure(0.f, theme, canvas).width;
    select.item("short", std::string(40, 'y'));
    select.value("short");
    EXPECT_FLOAT_EQ(select.measure(0.f, theme, canvas).width, wide);
}

TEST(Select, ShowsPlaceholderThenTheSelectedLabel) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto select = makeSelect();
    select->layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    select->paint(canvas, theme);

    ASSERT_FALSE(canvas.texts.empty());
    EXPECT_EQ(canvas.texts[0].value, "Choose");
    EXPECT_EQ(canvas.texts[0].color, theme.textMuted);

    select->value("banana");
    bul::test::FakeCanvas painted;
    select->paint(painted, theme);
    ASSERT_FALSE(painted.texts.empty());
    EXPECT_EQ(painted.texts[0].value, "Banana");
    EXPECT_EQ(painted.texts[0].color, theme.text);
}

TEST(Select, ClickOpensThePopupAndSelectsAnItem) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    std::string changed;
    auto        select = makeSelect(&changed);
    select->layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);

    // Closed: no overlay, a click on the trigger opens it.
    std::vector<bul::Widget*> overlays;
    select->collectOverlays(overlays);
    EXPECT_TRUE(overlays.empty());
    select->activateAt({70.f, 9.f});
    EXPECT_TRUE(select->open());

    overlays.clear();
    select->collectOverlays(overlays);
    ASSERT_EQ(overlays.size(), 1u);
    EXPECT_EQ(overlays[0], select.get());
    EXPECT_EQ(select->overlayHitTest({5.f, 5.f}), select.get()); // claims the whole screen

    // Popup: below the trigger, 2 padding + 3 rows of 13.
    // Second row centre: y = 19 + 2 + 2 + 13 + 6.
    select->activateAt({70.f, 42.f});
    EXPECT_EQ(select->value(), "banana");
    EXPECT_EQ(changed, "banana");
    EXPECT_FALSE(select->open());
}

TEST(Select, OutsideClickClosesWithoutChangingTheValue) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto select = makeSelect();
    select->layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    select->activateAt({70.f, 9.f});
    ASSERT_TRUE(select->open());

    select->activateAt({300.f, 300.f});
    EXPECT_FALSE(select->open());
    EXPECT_EQ(select->value(), "");
}

TEST(Select, DisabledIgnoresClicksAndFailsTheFocus) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto select = makeSelect();
    select->disabled(true);
    select->layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);

    EXPECT_FALSE(select->interactive());
    EXPECT_FALSE(select->focusable());
    select->activateAt({70.f, 9.f});
    EXPECT_FALSE(select->open());
}

TEST(Select, DisabledItemCannotBeChosen) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto select = std::make_unique<bul::Select>();
    select->item("one", "One", true);
    select->item("two", "Two");
    select->layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);

    select->activateAt({70.f, 9.f});
    ASSERT_TRUE(select->open());

    // First row is disabled: the click is swallowed and the popup stays open.
    select->activateAt({70.f, 29.f});
    EXPECT_EQ(select->value(), "");
    EXPECT_TRUE(select->open());

    select->activateAt({70.f, 42.f});
    EXPECT_EQ(select->value(), "two");
    EXPECT_FALSE(select->open());
}

TEST(Select, KeyboardOpensMovesAndCommits) {
    auto select = makeSelect();
    select->key({bul::Key::Down, false, false});
    EXPECT_TRUE(select->open());

    select->key({bul::Key::Down, false, false});
    select->key({bul::Key::Enter, false, false});
    EXPECT_EQ(select->value(), "banana");
    EXPECT_FALSE(select->open());

    select->key({bul::Key::Space, false, false});
    EXPECT_TRUE(select->open());
    select->key({bul::Key::Escape, false, false});
    EXPECT_FALSE(select->open());
}

TEST(Select, PaintsThePopupRows) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto select = makeSelect();
    select->layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    select->activateAt({70.f, 9.f});

    bul::test::FakeCanvas popup;
    select->paintOverlay(popup, theme);

    EXPECT_GE(popup.fills.size(), 2u); // panel + rows
    std::vector<std::string> labels;
    for (auto const& text : popup.texts) {
        labels.push_back(text.value);
    }
    EXPECT_NE(std::find(labels.begin(), labels.end(), "Apple"), labels.end());
    EXPECT_NE(std::find(labels.begin(), labels.end(), "Banana"), labels.end());
    EXPECT_NE(std::find(labels.begin(), labels.end(), "Cherry"), labels.end());
}

TEST(Select, PopupWidensForLongLabels) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto select = std::make_unique<bul::Select>();
    select->item("long", "A rather long option label");
    select->layout({0.f, 0.f, 120.f, 19.f}, theme, canvas);

    select->activateAt({60.f, 9.f});
    ASSERT_TRUE(select->open());

    // The list is wider than the trigger, so a click to the right of the trigger
    // still lands on the row.
    select->activateAt({150.f, 29.f});
    EXPECT_EQ(select->value(), "long");
    EXPECT_FALSE(select->open());
}

TEST(Select, FieldPropagatesItsStates) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto field = std::make_unique<bul::Field>();
    field->label("Fruit");
    auto select = std::make_unique<bul::Select>();
    select->placeholder("Choose");
    select->item("apple", "Apple");
    bul::Select* raw = select.get();
    field->control(std::move(select));

    field->layout({0.f, 0.f, 160.f, 40.f}, theme, canvas);
    field->invalid(true);
    field->paint(canvas, theme);
    ASSERT_FALSE(canvas.strokes.empty());
    EXPECT_EQ(canvas.strokes.back().color, theme.invalid);

    field->disabled(true);
    EXPECT_FALSE(raw->interactive());
    EXPECT_FALSE(raw->focusable());
}

TEST(Select, OverlayIsCollectedThroughContainers) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto column = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    auto field  = std::make_unique<bul::Field>();
    field->label("Fruit");
    auto select = std::make_unique<bul::Select>();
    select->item("apple", "Apple");
    bul::Select* raw = select.get();
    field->control(std::move(select));
    column->add(std::move(field));

    column->layout({0.f, 0.f, 160.f, 40.f}, theme, canvas);
    raw->activateAt({80.f, 20.f});
    ASSERT_TRUE(raw->open());

    std::vector<bul::Widget*> overlays;
    column->collectOverlays(overlays);
    ASSERT_EQ(overlays.size(), 1u);
    EXPECT_EQ(overlays[0], raw);
}

#include "gtest/gtest.h"

#include <memory>
#include <string>
#include <vector>

#include "FakeCanvas.h"

#include "bul/TextEdit.h"
#include "bul/Theme.h"
#include "bul/Widget.h"

namespace {

[[nodiscard]] bul::Theme testTheme() {
    bul::Theme theme = bul::Theme::neutral();
    theme.fontScale  = 1.f;
    theme.gap        = 8.f;
    return theme;
}

} // namespace

// ------------------------------------------------------------------- TextEdit

TEST(TextEdit, StartsEmptyWithTheCaretAtTheEnd) {
    bul::TextEdit edit;
    EXPECT_TRUE(edit.empty());
    EXPECT_EQ(edit.caret(), 0u);
    EXPECT_FALSE(edit.hasSelection());

    edit.setText("abc");
    EXPECT_EQ(edit.caret(), 3u);
    EXPECT_EQ(edit.length(), 3u);
}

TEST(TextEdit, InsertsAtTheCaret) {
    bul::TextEdit edit;
    edit.setText("ab");
    edit.insert("c");
    EXPECT_EQ(edit.text(), "abc");
    EXPECT_EQ(edit.caret(), 3u);

    edit.moveHome();
    edit.insert("x");
    EXPECT_EQ(edit.text(), "xabc");
    EXPECT_EQ(edit.caret(), 1u);
}

TEST(TextEdit, ErasesWholeCodePoints) {
    bul::TextEdit edit;
    edit.setText("你好");
    EXPECT_EQ(edit.length(), 2u);

    edit.eraseBackward();
    EXPECT_EQ(edit.text(), "你");
    EXPECT_EQ(edit.caret(), 3u);

    edit.eraseBackward();
    EXPECT_TRUE(edit.empty());
    EXPECT_EQ(edit.caret(), 0u);

    edit.eraseBackward(); // nothing left: no-op
    EXPECT_TRUE(edit.empty());
}

TEST(TextEdit, CaretMovesByCodePoint) {
    bul::TextEdit edit;
    edit.setText("你好a"); // 7 bytes, 3 code points
    EXPECT_EQ(edit.caret(), 7u);

    edit.moveCaret(-1);
    EXPECT_EQ(edit.caret(), 6u); // before 'a'
    edit.moveCaret(-2);
    EXPECT_EQ(edit.caret(), 0u);
    edit.moveCaret(1);
    EXPECT_EQ(edit.caret(), 3u); // after '你'
    edit.moveCaret(-5);          // clamped at the start
    EXPECT_EQ(edit.caret(), 0u);
}

TEST(TextEdit, SelectionIsReplacedByTyping) {
    bul::TextEdit edit;
    edit.setText("hello");
    edit.selectAll();
    EXPECT_TRUE(edit.hasSelection());
    EXPECT_EQ(edit.selectedText(), "hello");

    edit.insert("hi");
    EXPECT_EQ(edit.text(), "hi");
    EXPECT_EQ(edit.caret(), 2u);
    EXPECT_FALSE(edit.hasSelection());
}

TEST(TextEdit, ShiftExtendsTheSelectionFromTheAnchor) {
    bul::TextEdit edit;
    edit.setText("abcd");
    edit.moveHome();

    edit.extendCaret(2);
    EXPECT_EQ(edit.selectionBegin(), 0u);
    EXPECT_EQ(edit.selectionEnd(), 2u);
    EXPECT_EQ(edit.selectedText(), "ab");

    edit.extendCaret(-1);
    EXPECT_EQ(edit.selectedText(), "a");
}

TEST(TextEdit, DeleteForwardRemovesTheNextCodePoint) {
    bul::TextEdit edit;
    edit.setText("你好");
    edit.moveHome();
    edit.eraseForward();
    EXPECT_EQ(edit.text(), "好");
    EXPECT_EQ(edit.caret(), 0u);
}

TEST(TextEdit, MaxLengthCountsCodePoints) {
    bul::TextEdit edit;
    edit.setMaxLength(3);
    edit.insert("abcdef");
    EXPECT_EQ(edit.text(), "abc");

    edit.moveHome();
    edit.insert("Z"); // at the limit: nothing changes
    EXPECT_EQ(edit.text(), "abc");

    edit.moveEnd();
    edit.eraseBackward();
    edit.insert("XY"); // one more code point fits, the rest is dropped
    EXPECT_EQ(edit.text(), "abX");
}

TEST(TextEdit, ImeCompositionIsRewrittenByBackspaces) {
    bul::TextEdit edit;
    // What the IME sends while the pinyin composes: the composition text.
    edit.insert("a'a");
    EXPECT_EQ(edit.text(), "a'a");

    // What it sends when the composition is committed: one backspace per code
    // point it takes back, then the committed characters.
    edit.insert("\b\b\b啊啊");
    EXPECT_EQ(edit.text(), "啊啊");
    EXPECT_EQ(edit.caret(), 6u);

    // A backspace with nothing before it is harmless.
    edit.moveHome();
    edit.insert("\b");
    EXPECT_EQ(edit.text(), "啊啊");
}

TEST(TextEdit, LineBreaksAreDropped) {
    bul::TextEdit edit;
    edit.insert("a\nb\tc");
    EXPECT_EQ(edit.text(), "abc");
}

TEST(TextEdit, MaskedTextIsOneStarPerCodePoint) {
    bul::TextEdit edit;
    edit.setText("密码");
    EXPECT_EQ(edit.masked(), "**");
    EXPECT_EQ(edit.offsetOfCodePoint(1), 3u);
    EXPECT_EQ(edit.codePointIndexOf(3), 1u);
    EXPECT_EQ(edit.codePointIndexOf(100), 2u);
}

// ---------------------------------------------------------------------- Input

TEST(Input, MeasuresOneLineHigh) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input      input;
    bul::Size const size = input.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 120.f); // empty field keeps the minimum width
    EXPECT_FLOAT_EQ(size.height, 19.f); // line 9 + 2 * padding 4 + 2 * border 1

    input.placeholder(std::string(40, 'x'));
    EXPECT_FLOAT_EQ(input.measure(0.f, theme, canvas).width, 40.f * 6.f + 4.f + 14.f);
    // An explicit width wins when the parent has one.
    EXPECT_FLOAT_EQ(input.measure(90.f, theme, canvas).width, 90.f);
}

TEST(Input, PlaceholderIsPaintedWhenEmpty) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input input;
    input.placeholder("abc");
    input.layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    input.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].value, "abc");
    EXPECT_EQ(canvas.texts[0].color, theme.textMuted);

    ASSERT_EQ(canvas.fills.size(), 1u);
    EXPECT_EQ(canvas.fills[0].color, theme.input);
    ASSERT_EQ(canvas.strokes.size(), 1u);
    EXPECT_EQ(canvas.strokes[0].color, theme.inputBorder);
}

TEST(Input, DisabledDimsEverything) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input input;
    input.placeholder("abc");
    input.disabled(true);
    input.layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    input.paint(canvas, theme);

    ASSERT_FALSE(canvas.fills.empty());
    EXPECT_FLOAT_EQ(canvas.fills[0].alpha, 0.5f);
    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_FLOAT_EQ(canvas.texts[0].color.a, 0.5f);
    EXPECT_FALSE(input.interactive());
    EXPECT_FALSE(input.focusable());
    EXPECT_FALSE(input.acceptsTextInput());
}

TEST(Input, InvalidUsesTheInvalidToken) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input input;
    input.value("abc");
    input.invalid(true);
    input.layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    input.paint(canvas, theme);

    ASSERT_EQ(canvas.strokes.size(), 1u);
    EXPECT_EQ(canvas.strokes[0].color, theme.invalid);
}

TEST(Input, FocusDrawsTheCaret) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input input;
    input.value("abcd");
    input.setFocused(true);
    input.layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    input.paint(canvas, theme);

    // Surface plus the caret; the caret sits after the last character
    // (text starts at 1 border + 6 padding, 4 characters are 24 units wide).
    ASSERT_EQ(canvas.fills.size(), 2u);
    EXPECT_FLOAT_EQ(canvas.fills[1].rect.x, 7.f + 24.f);
    EXPECT_FLOAT_EQ(canvas.fills[1].rect.width, 1.f);
}

TEST(Input, WideGlyphsUseALineHighCell) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    // FakeCanvas measures every code point 6 wide. A wide (CJK) code point must
    // still advance a line-high cell (9 - 1 = 8): the engine under-reports them
    // and the unicode glyph it draws is square, so this table is what keeps the
    // caret on the glyph grid instead of inside the text.
    bul::Input input;
    input.value("啊啊");
    input.setFocused(true); // caret at the end
    input.layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    input.paint(canvas, theme);

    ASSERT_GE(canvas.texts.size(), 2u);
    EXPECT_EQ(canvas.texts[0].value, "啊");
    EXPECT_FLOAT_EQ(canvas.texts[0].topLeft.x, 7.f);
    EXPECT_EQ(canvas.texts[1].value, "啊");
    EXPECT_FLOAT_EQ(canvas.texts[1].topLeft.x, 15.f); // 7 + one cell (8)

    ASSERT_GE(canvas.fills.size(), 2u);
    EXPECT_FLOAT_EQ(canvas.fills[1].rect.x, 23.f); // after both cells
    EXPECT_FLOAT_EQ(canvas.fills[1].rect.width, 1.f);
}

TEST(Input, PasswordKindMasksGlyphs) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input input;
    input.kind(bul::Input::Kind::Password);
    input.value("secret");
    input.layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    input.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].value, "******");
}

TEST(Input, TypingEditsAndNotifies) {
    bul::Input  input;
    int         calls = 0;
    std::string last;
    input.onChange([&](std::string const& value) {
        ++calls;
        last = value;
    });

    input.textInput("ab");
    EXPECT_EQ(input.value(), "ab");
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(last, "ab");

    EXPECT_TRUE(input.key({bul::Key::Backspace, false, false}));
    EXPECT_EQ(input.value(), "a");
    EXPECT_EQ(calls, 2);

    input.key({bul::Key::Home, false, false});
    input.key({bul::Key::Delete, false, false});
    EXPECT_EQ(input.value(), "");
    EXPECT_EQ(calls, 3);
}

TEST(Input, ControlASelectsAll) {
    bul::Input input;
    input.value("hello");
    EXPECT_TRUE(input.key({bul::Key::A, true, false}));
    EXPECT_EQ(input.selectedText(), "hello");

    input.textInput("x");
    EXPECT_EQ(input.value(), "x");
}

TEST(Input, ClickPlacesTheCaret) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input input;
    input.value("abcd");
    input.layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    input.setFocused(true);        // caret at the end
    input.activateAt({19.f, 9.f}); // two characters in
    input.layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);

    input.key({bul::Key::Backspace, false, false});
    EXPECT_EQ(input.value(), "acd");
}

TEST(Input, FileKindShowsChooseButtonAndAcceptsAPath) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input input;
    input.kind(bul::Input::Kind::File);
    std::string chosen;
    int         chooses = 0;
    input.onChoose([&](std::string const& path) {
        ++chooses;
        chosen = path;
    });
    EXPECT_TRUE(input.wantsFilePicker());
    EXPECT_TRUE(input.interactive());
    EXPECT_FALSE(input.focusable());

    input.layout({0.f, 0.f, 200.f, 19.f}, theme, canvas);
    input.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 2u);
    EXPECT_EQ(canvas.texts[0].value, "Choose File");
    EXPECT_EQ(canvas.texts[1].value, "No file chosen");
    EXPECT_EQ(canvas.texts[1].color, theme.textMuted);

    // The host opens the dialog and hands the path over; the widget shows the
    // file name and reports the full path.
    input.chooseFile("C:\\pics\\avatar.png");
    EXPECT_EQ(chooses, 1);
    EXPECT_EQ(chosen, "C:\\pics\\avatar.png");
    EXPECT_EQ(input.filePath(), "C:\\pics\\avatar.png");
    EXPECT_EQ(input.value(), "avatar.png");

    bul::test::FakeCanvas painted;
    input.paint(painted, theme);
    ASSERT_EQ(painted.texts.size(), 2u);
    EXPECT_EQ(painted.texts[1].value, "avatar.png");
    EXPECT_EQ(painted.texts[1].color, theme.text);

    input.disabled(true);
    EXPECT_FALSE(input.wantsFilePicker());
}

TEST(Input, ValueDoesNotChangeTheDesiredWidth) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input input;
    input.placeholder("Email");
    bul::Size const before = input.measure(0.f, theme, canvas);

    // The value scrolls inside the box; it must not resize the form around it.
    input.value(std::string(200, 'x'));
    EXPECT_FLOAT_EQ(input.measure(0.f, theme, canvas).width, before.width);
}

TEST(Input, LongValueIsCutWithAnEllipsisWhenNotFocused) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input input;
    input.value(std::string(80, 'x'));
    input.layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    input.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_LT(canvas.texts[0].value.size(), 80u);
    EXPECT_EQ(canvas.texts[0].value.substr(canvas.texts[0].value.size() - 3u, 3u), "...");
    EXPECT_LE(canvas.textWidth(canvas.texts[0].value, 1.f), 140.f);
}

TEST(Input, ValidatorMarksTheFieldInvalidUntilItPasses) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Input input;
    input.placeholder("you@example.com");
    input.validator([](std::string const& value) { return value.find('@') != std::string::npos; });

    input.textInput("abc");
    EXPECT_TRUE(input.validationFailed());
    EXPECT_TRUE(input.invalid());

    input.layout({0.f, 0.f, 140.f, 19.f}, theme, canvas);
    input.paint(canvas, theme);
    ASSERT_FALSE(canvas.strokes.empty());
    EXPECT_EQ(canvas.strokes[0].color, theme.invalid);

    input.textInput("@");
    EXPECT_FALSE(input.validationFailed());
    EXPECT_FALSE(input.invalid());

    input.key({bul::Key::Backspace, false, false}); // back to "abc"
    EXPECT_TRUE(input.validationFailed());

    // Removing the validator clears the verdict.
    input.validator({});
    EXPECT_FALSE(input.invalid());
}

// ---------------------------------------------------------------------- Field

TEST(Field, VerticalLayoutStacksTheParts) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Field field;
    field.label("Email");
    auto         input   = std::make_unique<bul::Input>();
    bul::Widget* control = input.get();
    input->placeholder("x");
    field.control(std::move(input));
    field.description("Hint");

    bul::Size const size = field.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 120.f);                                 // the input is the widest part
    EXPECT_FLOAT_EQ(size.height, 9.f + 3.f + 19.f + 3.f + 9.f * 0.85f); // label, control, description

    field.layout({0.f, 0.f, size.width, size.height}, theme, canvas);
    EXPECT_FLOAT_EQ(field.labelWidget()->bounds().y, 0.f);
    EXPECT_FLOAT_EQ(control->bounds().y, 12.f);
    EXPECT_FLOAT_EQ(field.descriptionWidget()->bounds().y, 34.f);
    EXPECT_FLOAT_EQ(control->bounds().width, size.width);
}

TEST(Field, HorizontalLayoutGivesTheControlTheRestOfTheWidth) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Field field;
    field.orientation(bul::Field::Orientation::Horizontal);
    field.label("Search");
    auto         input   = std::make_unique<bul::Input>();
    bul::Widget* control = input.get();
    field.control(std::move(input));

    bul::Size const size = field.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 36.f + 3.f + 120.f);
    EXPECT_FLOAT_EQ(size.height, 19.f);

    field.layout({0.f, 0.f, 300.f, 19.f}, theme, canvas);
    EXPECT_FLOAT_EQ(control->bounds().x, 39.f);
    EXPECT_FLOAT_EQ(control->bounds().width, 261.f);
}

TEST(Field, StateReachesTheControl) {
    bul::Field  field;
    auto        input   = std::make_unique<bul::Input>();
    bul::Input* control = input.get();
    field.control(std::move(input));

    field.disabled(true);
    EXPECT_TRUE(control->disabled());
    EXPECT_FALSE(control->interactive());

    field.disabled(false);
    EXPECT_FALSE(control->disabled());
    EXPECT_TRUE(control->interactive());

    // A flag set on the control itself survives the field clearing its own.
    control->disabled(true);
    field.disabled(true);
    field.disabled(false);
    EXPECT_TRUE(control->disabled());
}

TEST(Field, InvalidStylesTheControl) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Field field;
    auto       input = std::make_unique<bul::Input>();
    input->placeholder("x");
    field.control(std::move(input));
    field.invalid(true);

    field.layout({0.f, 0.f, 120.f, 19.f}, theme, canvas);
    field.paint(canvas, theme);

    ASSERT_FALSE(canvas.strokes.empty());
    EXPECT_EQ(canvas.strokes[0].color, theme.invalid);
}

TEST(Field, RequiredMarksTheLabel) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Field field;
    field.label("Email");
    auto input = std::make_unique<bul::Input>();
    input->placeholder("x");
    field.control(std::move(input));
    field.required(true);

    field.layout({0.f, 0.f, 120.f, 40.f}, theme, canvas);
    field.paint(canvas, theme);

    ASSERT_GE(canvas.texts.size(), 2u);
    EXPECT_EQ(canvas.texts[0].value, "Email");
    EXPECT_EQ(canvas.texts[1].value, " *");
    EXPECT_EQ(canvas.texts[1].color, theme.invalid);
}

TEST(Field, CollectsTheFocusableControl) {
    bul::Field   field;
    auto         input   = std::make_unique<bul::Input>();
    bul::Widget* control = input.get();
    field.control(std::move(input));

    std::vector<bul::Widget*> focusables;
    field.collectFocusables(focusables);
    ASSERT_EQ(focusables.size(), 1u);
    EXPECT_EQ(focusables[0], control);
}

// ----------------------------------------------------------------- FieldLabel

TEST(FieldLabel, BadgeSitsAfterTheText) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::FieldLabel label("Password");
    bul::Size const base = label.measure(0.f, theme, canvas);

    label.badge(std::make_unique<bul::Badge>("Recommended", bul::Badge::Variant::Secondary));
    bul::Size const withBadge = label.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(withBadge.width, 48.f + 4.f + 59.5f);
    EXPECT_GT(withBadge.width, base.width);
    EXPECT_FLOAT_EQ(withBadge.height, 9.75f); // the badge is taller than the label line

    label.layout({0.f, 0.f, withBadge.width, withBadge.height}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 2u);
    EXPECT_EQ(canvas.texts[0].value, "Password");
    EXPECT_EQ(canvas.texts[1].value, "Recommended");
    EXPECT_EQ(canvas.texts[1].topLeft.x, 52.f);
}

TEST(FieldLabel, RequiredMarkerCanBeSetDirectly) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::FieldLabel label("Email");
    label.required(true);
    EXPECT_TRUE(label.required());
    EXPECT_FLOAT_EQ(label.measure(0.f, theme, canvas).width, 30.f + 12.f);
}

// ---------------------------------------------------------------------- Badge

TEST(Badge, MeasuresTextPlusPadding) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Badge badge("Recommended", bul::Badge::Variant::Secondary);
    bul::Size  size = badge.measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 11.f * 6.f * 0.75f + 10.f);
    EXPECT_FLOAT_EQ(size.height, 9.f * 0.75f + 3.f);

    badge.layout({0.f, 0.f, size.width, size.height}, theme, canvas);
    badge.paint(canvas, theme);
    ASSERT_EQ(canvas.fills.size(), 1u);
    EXPECT_EQ(canvas.fills[0].color, theme.secondary);
    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].value, "Recommended");
    EXPECT_EQ(canvas.texts[0].align, bul::Align::Center);
}

// ---------------------------------------------------------------------- Stack

TEST(Stack, StretchSharesTheLeftoverWidth) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Stack row{bul::Stack::Direction::Horizontal};
    row.gap(4.f);
    row.add(std::make_unique<bul::Label>("a")); // 6 wide

    auto        input   = std::make_unique<bul::Input>();
    bul::Input* control = input.get();
    row.add(std::move(input), 1.f);

    row.layout({0.f, 0.f, 300.f, 19.f}, theme, canvas);
    EXPECT_FLOAT_EQ(control->bounds().x, 10.f);
    EXPECT_FLOAT_EQ(control->bounds().width, 290.f);
}

TEST(Stack, WeightedColumnsStayInsideTheRow) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Stack row{bul::Stack::Direction::Horizontal};
    row.gap(10.f);

    auto  left     = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    auto  right    = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    auto* leftRaw  = left.get();
    auto* rightRaw = right.get();

    // A long value used to make this column claim the full available width and
    // push itself out of the row; weighted columns must share what is left.
    auto input = std::make_unique<bul::Input>();
    input->value(std::string(100, 'x'));
    right->add(std::move(input));

    row.add(std::move(left), 1.f);
    row.add(std::move(right), 1.f);

    row.layout({0.f, 0.f, 300.f, 60.f}, theme, canvas);
    EXPECT_FLOAT_EQ(leftRaw->bounds().width, 145.f);
    EXPECT_FLOAT_EQ(rightRaw->bounds().width, 145.f);
    EXPECT_FLOAT_EQ(rightRaw->bounds().x, 155.f);
    EXPECT_LE(rightRaw->bounds().right(), 300.f);
}

// ---------------------------------------------------------------------- Theme

TEST(Theme, InputTokensAreDistinct) {
    bul::Theme const theme = bul::Theme::neutral();
    EXPECT_NE(theme.input, theme.panel);
    EXPECT_NE(theme.inputHover, theme.input);
    EXPECT_NE(theme.inputBorder, theme.input);
    EXPECT_NE(theme.invalid, theme.destructive);
}

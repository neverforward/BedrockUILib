#include "gtest/gtest.h"

#include <memory>
#include <string>

#include "FakeCanvas.h"

#include "bul/Literals.h"
#include "bul/StyleSheet.h"
#include "bul/Theme.h"
#include "bul/Widget.h"

using namespace bul::literals;

namespace {

constexpr bul::Rgba kGreen = bul::Rgba::fromRgb(0x46A758);
constexpr bul::Rgba kBlue  = bul::Rgba::fromRgb(0x4C8BF5);

bul::Theme testTheme() {
    bul::Theme theme = bul::Theme::neutral();
    theme.fontScale  = 1.f;
    theme.gap        = 6.f;
    return theme;
}

struct ResetSheet {
    ~ResetSheet() { bul::setStyleSheet(bul::StyleSheet{}); }
};

} // namespace

TEST(StyleRules, LengthUnitsResolve) {
    float const base = 9.f;
    EXPECT_FLOAT_EQ((12_px).resolve(base), 12.f);
    EXPECT_FLOAT_EQ((1.5_px).resolve(base), 1.5f);
    EXPECT_FLOAT_EQ((2_em).resolve(base), 18.f);
    EXPECT_FLOAT_EQ((2_ex).resolve(base), 9.f);
    EXPECT_FLOAT_EQ((50_pct).resolve(base), 4.5f);
    EXPECT_FLOAT_EQ((1_in).resolve(base), 96.f);
    EXPECT_FLOAT_EQ((1_pc).resolve(base), 16.f);
    EXPECT_FLOAT_EQ((12_pt).resolve(base), 16.f);
    EXPECT_FLOAT_EQ((2.54_cm).resolve(base), 96.f);
    EXPECT_FLOAT_EQ((25.4_mm).resolve(base), 96.f);
}

TEST(StyleRules, BackgroundColourPaintsTheButton) {
    ResetSheet const      reset;
    bul::test::FakeCanvas canvas;

    bul::StyleSheet sheet;
    sheet.type("Button").set({.backgroundColor = kBlue});
    bul::setStyleSheet(std::move(sheet));

    bul::Button      button{"OK", nullptr};
    bul::Theme const theme = testTheme();
    button.layout({0.f, 0.f, 80.f, 19.f}, theme, canvas);
    button.paint(canvas, theme);

    ASSERT_FALSE(canvas.fills.empty());
    EXPECT_EQ(canvas.fills[0].color, kBlue);
}

TEST(StyleRules, InlineBackgroundWinsOverTheSheet) {
    ResetSheet const reset;

    bul::StyleSheet sheet;
    sheet.type("Button").set({.backgroundColor = kBlue});
    bul::setStyleSheet(std::move(sheet));

    bul::test::FakeCanvas canvas;
    bul::Button           button{"OK", nullptr};
    button.style({.backgroundColor = kGreen});
    bul::Theme const theme = testTheme();
    button.layout({0.f, 0.f, 80.f, 19.f}, theme, canvas);
    button.paint(canvas, theme);

    ASSERT_FALSE(canvas.fills.empty());
    EXPECT_EQ(canvas.fills[0].color, kGreen);
}

TEST(StyleRules, PaddingChangesTheMeasuredSize) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Button plain{"OK", nullptr};
    bul::Button padded{"OK", nullptr};
    padded.style({.padding = bul::Insets::all(20.f)});

    bul::Size const before = plain.measure(200.f, theme, canvas);
    bul::Size const after  = padded.measure(200.f, theme, canvas);
    // The default profile pads 10 horizontally and 5 vertically; the style asks
    // for 20 on each side.
    EXPECT_FLOAT_EQ(after.width, before.width + 20.f);
    EXPECT_FLOAT_EQ(after.height, before.height + 30.f);
}

TEST(StyleRules, StackPaddingInsetsItsChildren) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto stack = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    stack->style({.padding = bul::Insets::all(5.f), .gap = 0.f});
    stack->add(std::make_unique<bul::Label>("Hello"));

    bul::Size const size = stack->measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 5.f + 30.f + 5.f);
    EXPECT_FLOAT_EQ(size.height, 5.f + 9.f + 5.f);

    stack->layout({0.f, 0.f, size.width, size.height}, theme, canvas);
    bul::Widget* child = stack->hitTest({7.f, 7.f});
    ASSERT_NE(child, nullptr);
    EXPECT_FLOAT_EQ(child->bounds().x, 5.f);
    EXPECT_FLOAT_EQ(child->bounds().y, 5.f);
}

TEST(StyleRules, MarginAddsSpaceAndShiftsTheContent) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto stack = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    stack->style({.padding = bul::Padding::all(0.f), .gap = 0.f});
    auto label = std::make_unique<bul::Label>("Hi");
    label->style({.margin = bul::Padding::all(5.f)});
    bul::Label const* labelPtr = label.get();
    stack->add(std::move(label));

    bul::Size const size = stack->measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 12.f + 10.f);
    EXPECT_FLOAT_EQ(size.height, 9.f + 10.f);

    stack->layout({0.f, 0.f, size.width, size.height}, theme, canvas);
    EXPECT_FLOAT_EQ(labelPtr->bounds().x, 5.f);
    EXPECT_FLOAT_EQ(labelPtr->bounds().y, 5.f);
}

TEST(StyleRules, WidthFixesTheMeasuredSize) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"abc"};
    label.style({.width = 50.f, .height = 20.f});
    bul::Size const size = label.measure(200.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 50.f);
    EXPECT_FLOAT_EQ(size.height, 20.f);
}

TEST(StyleRules, TextAlignPlacesTheLine) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hi"};
    label.style({.textAlign = bul::Align::Center});
    label.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_GT(canvas.texts[0].topLeft.x, 0.f);
}

TEST(StyleRules, LineHeightChangesTheBlockHeight) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"a\nb"};
    label.style({.lineHeight = 20.f});
    bul::Size const size = label.measure(200.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 40.f);
}

TEST(StyleRules, VisibilityHidesThePaintingOnly) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label hidden{"Hi"};
    hidden.style({.visibility = bul::Visibility::Hidden});
    bul::Size const size = hidden.measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 9.f);

    hidden.layout({0.f, 0.f, 40.f, 9.f}, theme, canvas);
    hidden.paint(canvas, theme);
    EXPECT_TRUE(canvas.texts.empty());
}

TEST(StyleRules, TextTransformCasesTheText) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"ab"};
    label.style({.textTransform = bul::TextTransform::Uppercase});
    label.layout({0.f, 0.f, 40.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].value, "AB");
}

TEST(StyleRules, WhiteSpaceNoWrapKeepsOneLine) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label wrapped{"aa bb"};
    wrapped.wrap(true).elide(false);

    bul::Label nowrap{"aa bb"};
    nowrap.wrap(true).elide(false);
    nowrap.style({.whiteSpace = bul::WhiteSpace::NoWrap});

    EXPECT_FLOAT_EQ(wrapped.measure(12.f, theme, canvas).width, 12.f);
    EXPECT_FLOAT_EQ(nowrap.measure(12.f, theme, canvas).width, 30.f);
}

TEST(StyleRules, VerticalAlignPlacesTheBlock) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hi"};
    label.style({.verticalAlign = bul::TextVertical::Bottom});
    label.layout({0.f, 0.f, 40.f, 20.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_FLOAT_EQ(canvas.texts[0].topLeft.y, 11.f);
}

TEST(StyleRules, DisplayNoneDropsTheWidgetFromLayout) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto stack = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    auto shown = std::make_unique<bul::Label>("one");
    auto hidden = std::make_unique<bul::Label>("two");
    bul::Label const* hiddenPtr = hidden.get();
    hidden->style({.display = bul::Display::None});
    stack->add(std::move(shown));
    stack->add(std::move(hidden));

    bul::Size const size = stack->measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 9.f);

    stack->layout({0.f, 0.f, size.width, size.height}, theme, canvas);
    EXPECT_FLOAT_EQ(hiddenPtr->bounds().height, 0.f);
}

TEST(StyleRules, OpacityMultipliesThePaintAlpha) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label label{"Hi"};
    label.style({.opacity = 0.5f});
    label.layout({0.f, 0.f, 40.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_FLOAT_EQ(canvas.texts[0].color.a, 0.5f);
}

TEST(StyleRules, BorderRadiusRoundsTheFill) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Button button{"OK", nullptr};
    button.style({.borderRadius = 6.f});
    button.layout({10.f, 10.f, 80.f, 19.f}, theme, canvas);
    button.paint(canvas, theme);

    ASSERT_FALSE(canvas.fills.empty());
    // A rounded fill is drawn row by row, and the first row is inset.
    EXPECT_GT(canvas.fills[0].rect.x, 10.f);
}

TEST(StyleRules, LetterSpacingWidensAndSpreadsTheText) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label tight{"ab"};
    bul::Label spaced{"ab"};
    spaced.style({.letterSpacing = 4.f});

    EXPECT_FLOAT_EQ(tight.measure(100.f, theme, canvas).width, 12.f);
    EXPECT_FLOAT_EQ(spaced.measure(100.f, theme, canvas).width, 16.f);

    spaced.layout({0.f, 0.f, 50.f, 9.f}, theme, canvas);
    spaced.paint(canvas, theme);
    // One glyph at a time, so the spacing is real and not just a wider box.
    EXPECT_EQ(canvas.texts.size(), 2u);
}

TEST(StyleRules, TextStyleUsesTheFormatCodes) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Label bold{"Hi"};
    bold.style({.fontWeight = bul::FontWeight::Bold});
    bold.layout({0.f, 0.f, 40.f, 9.f}, theme, canvas);
    bold.paint(canvas, theme);
    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].value, "\xC2\xA7lHi\xC2\xA7r");

    bul::test::FakeCanvas italicCanvas;
    bul::Label italic{"Hi"};
    italic.style({.fontStyle = bul::FontStyle::Italic});
    italic.layout({0.f, 0.f, 40.f, 9.f}, theme, italicCanvas);
    italic.paint(italicCanvas, theme);
    ASSERT_EQ(italicCanvas.texts.size(), 1u);
    EXPECT_EQ(italicCanvas.texts[0].value, "\xC2\xA7oHi\xC2\xA7r");

    bul::test::FakeCanvas obfuscatedCanvas;
    bul::Label mixed{"Hi"};
    mixed.style({.obfuscated = true});
    mixed.layout({0.f, 0.f, 40.f, 9.f}, theme, obfuscatedCanvas);
    mixed.paint(obfuscatedCanvas, theme);
    ASSERT_EQ(obfuscatedCanvas.texts.size(), 1u);
    EXPECT_EQ(obfuscatedCanvas.texts[0].value, "\xC2\xA7kHi\xC2\xA7r");
}

TEST(StyleRules, ContainerPaintsItsBox) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto card = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    card->style({.backgroundColor = kBlue, .borderColor = kGreen, .borderWidth = 1.f, .borderRadius = 6.f});
    card->add(std::make_unique<bul::Label>("Hi"));
    card->layout({0.f, 0.f, 60.f, 30.f}, theme, canvas);
    card->paint(canvas, theme);

    bool hasBlue = false;
    for (auto const& fill : canvas.fills) {
        hasBlue = hasBlue || fill.color == kBlue;
    }
    EXPECT_TRUE(hasBlue);
}

TEST(StyleRules, FillStopsWhereTheBorderStarts) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Button button{"OK", nullptr};
    button.style({.backgroundColor = kBlue, .borderColor = kGreen, .borderWidth = 1.f, .borderRadius = 6.f});
    button.layout({10.f, 10.f, 80.f, 19.f}, theme, canvas);
    button.paint(canvas, theme);

    // The surface is inset by the border width, so no colour is left outside the
    // stroke at a corner.
    bool found = false;
    for (auto const& fill : canvas.fills) {
        if (fill.color == kBlue) {
            EXPECT_GE(fill.rect.x, 11.f);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST(StyleRules, EmPaddingResolvesAgainstTheFont) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme(); // base font 9, scale 1

    auto stack = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    stack->style({.padding = bul::Padding::all(1_em)});
    stack->add(std::make_unique<bul::Label>("Hi"));

    bul::Size const size = stack->measure(100.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 9.f + 12.f + 9.f);
    EXPECT_FLOAT_EQ(size.height, 9.f + 9.f + 9.f);
}

TEST(StyleRules, ColorReachesEveryTextWidget) {
    ResetSheet const      reset;
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::StyleSheet sheet;
    sheet.type("Label").set({.color = kGreen});
    bul::setStyleSheet(std::move(sheet));

    bul::Label label{"Hello"};
    label.layout({0.f, 0.f, 80.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].color, kGreen);
}

TEST(StyleRules, FontSizeScalesTheText) {
    ResetSheet const      reset;
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::StyleSheet sheet;
    sheet.type("Label").set({.fontSize = 2_em});
    bul::setStyleSheet(std::move(sheet));

    bul::Label label{"ab"};
    bul::Size const size = label.measure(80.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.height, 18.f);
}

TEST(StyleRules, AccordionSelectorEntersThroughTheWidgetClass) {
    bul::StyleSheet sheet;
    // `.type<bul::Label>()` reads the element name from the C++ class.
    sheet.type<bul::Label>().set({.fontSize = 12_px});
    EXPECT_EQ(sheet.rules()[0].selector.text(), "Label");
}

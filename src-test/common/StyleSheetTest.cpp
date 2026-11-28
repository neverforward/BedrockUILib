#include "gtest/gtest.h"

#include <memory>
#include <string>

#include "FakeCanvas.h"

#include "bul/StyleSheet.h"
#include "bul/Theme.h"
#include "bul/Widget.h"

namespace {

constexpr bul::Rgba kRed   = bul::Rgba::fromRgb(0xE5484D);
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

// --------------------------------------------------------------- selectors

TEST(SelectorParse, ReadsSelectorText) {
    auto const button = bul::Selector::parse("Button");
    ASSERT_TRUE(button.has_value());
    ASSERT_EQ(button->steps.size(), 1u);
    EXPECT_EQ(button->steps[0].compound.type, "Button");

    auto const danger = bul::Selector::parse(".danger");
    ASSERT_TRUE(danger.has_value());
    EXPECT_EQ(danger->steps[0].compound.className, "danger");

    auto const submit = bul::Selector::parse("#submit");
    ASSERT_TRUE(submit.has_value());
    EXPECT_EQ(submit->steps[0].compound.id, "submit");

    auto const compound = bul::Selector::parse("Button.danger#submit");
    ASSERT_TRUE(compound.has_value());
    EXPECT_EQ(compound->steps[0].compound.type, "Button");
    EXPECT_EQ(compound->steps[0].compound.className, "danger");
    EXPECT_EQ(compound->steps[0].compound.id, "submit");
    EXPECT_EQ(compound->specificity(), 111);

    auto const universal = bul::Selector::parse("*");
    ASSERT_TRUE(universal.has_value());
    EXPECT_TRUE(universal->steps[0].compound.universal);
}

TEST(SelectorParse, CombinatorsAndPseudoClasses) {
    auto const child = bul::Selector::parse("Stack > Label");
    ASSERT_TRUE(child.has_value());
    ASSERT_EQ(child->steps.size(), 2u);
    EXPECT_EQ(child->steps[1].combinator, bul::Combinator::Child);
    EXPECT_EQ(child->steps[1].compound.type, "Label");

    auto const misc = bul::Selector::parse("Input:disabled[type=text]:nth-child(2)");
    ASSERT_TRUE(misc.has_value());
    auto const& compound = misc->steps[0].compound;
    ASSERT_EQ(compound.pseudos.size(), 2u);
    EXPECT_EQ(compound.pseudos[0].name, "disabled");
    EXPECT_EQ(compound.pseudos[1].name, "nth-child");
    EXPECT_EQ(compound.pseudos[1].argument, "2");
    ASSERT_EQ(compound.attributes.size(), 1u);
    EXPECT_EQ(compound.attributes[0].name, "type");
    EXPECT_EQ(compound.attributes[0].value, "text");
}

TEST(SelectorParse, SelectorList) {
    auto const list = bul::Selector::parseList("Button, .danger, Label");
    ASSERT_EQ(list.size(), 3u);
    EXPECT_EQ(list[0].steps[0].compound.type, "Button");
    EXPECT_EQ(list[1].steps[0].compound.className, "danger");
}

TEST(SelectorParse, RejectsWhatIsNotASelector) {
    EXPECT_FALSE(bul::Selector::parse("").has_value());
    EXPECT_FALSE(bul::Selector::parse(".").has_value());
    EXPECT_FALSE(bul::Selector::parse("Button.").has_value());
    EXPECT_FALSE(bul::Selector::parse("Button??").has_value());
    EXPECT_FALSE(bul::Selector::parse("::before").has_value());
}

TEST(Selector, WritesBackAsText) {
    bul::StyleSheet sheet;
    sheet.type("Stack").type("Label").className("note").id("title").set({.color = kRed});
    ASSERT_EQ(sheet.rules().size(), 1u);
    EXPECT_EQ(sheet.rules()[0].selector.text(), "Stack Label.note#title");
}

// -------------------------------------------------------------------- rules

TEST(StyleSheetRules, FluentRulesChain) {
    bul::StyleSheet sheet;
    sheet.className("danger").set({.backgroundColor = kRed});
    sheet.id("submit").set({.color = kGreen});
    sheet.type("Button").set({.color = kBlue});

    ASSERT_EQ(sheet.rules().size(), 3u);
    EXPECT_EQ(sheet.styleFor("Button", "danger", "").backgroundColor, kRed);
    EXPECT_EQ(sheet.styleFor("Button", "", "submit").color, kGreen);
    EXPECT_EQ(sheet.styleFor("Button", "", "").color, kBlue);
}

TEST(StyleSheetRules, CascadeFollowsSpecificityThenOrder) {
    bul::StyleSheet sheet;
    sheet.type("Label").set({.color = kRed});
    sheet.className("note").set({.color = kGreen});
    sheet.type("Label").set({.color = kBlue});

    EXPECT_EQ(sheet.styleFor("Label", "", "").color, kBlue);
    EXPECT_EQ(sheet.styleFor("Label", "note", "").color, kGreen);
}

TEST(StyleSheetRules, RuleTakesASelectorList) {
    bul::StyleSheet sheet;
    sheet.rule("Button, Label", {.color = kBlue});

    ASSERT_EQ(sheet.rules().size(), 2u);
    EXPECT_EQ(sheet.styleFor("Button", "", "").color, kBlue);
    EXPECT_EQ(sheet.styleFor("Label", "", "").color, kBlue);
}

// --------------------------------------------------------------- attributes

TEST(StyleSheetRules, AttributeOperators) {
    bul::StyleSheet sheet;
    sheet.attr("src", "https://a", bul::Attribute::Match::StartsWith).set({.color = kRed});
    sheet.attr("name").set({.color = kGreen});

    bul::StyleSheet const& constSheet = sheet;
    EXPECT_TRUE(constSheet.empty() == false);
    ASSERT_EQ(sheet.rules().size(), 2u);
    EXPECT_EQ(sheet.rules()[0].selector.steps[0].compound.attributes[0].match, bul::Attribute::Match::StartsWith);
    EXPECT_EQ(sheet.rules()[1].selector.steps[0].compound.attributes[0].match, bul::Attribute::Match::Exists);
}

TEST(StyleSheetRules, AttributeSelectorMatchesAttrs) {
    bul::StyleSheet sheet;
    sheet.type("Input").attr("kind", "password").set({.color = kGreen});

    bul::Input plain;
    EXPECT_FALSE(sheet.styleFor(plain).color.has_value());

    bul::Input secret;
    secret.attr("kind", "password");
    ASSERT_TRUE(sheet.styleFor(secret).color.has_value());
    EXPECT_EQ(*sheet.styleFor(secret).color, kGreen);
}

TEST(StyleSheetRules, WordAttributeMatch) {
    bul::StyleSheet sheet;
    sheet.attr("tags", "primary", bul::Attribute::Match::Word).set({.color = kBlue});

    bul::Label label{"x"};
    label.attr("tags", "big primary quiet");
    EXPECT_EQ(sheet.styleFor(label).color, kBlue);
}

TEST(StyleSheetRules, TypeAttributeIsAutomatic) {
    bul::StyleSheet sheet;
    sheet.attr("type", "Label").set({.color = kBlue});

    bul::Label label{"Hi"};
    EXPECT_EQ(sheet.styleFor(label).color, kBlue);
}

// --------------------------------------------------------------- structure

TEST(StyleSheetRules, DescendantSelectorNeedsTheAncestor) {
    bul::StyleSheet sheet;
    sheet.type("Stack").type("Label").set({.color = kRed});

    auto stack  = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    auto inner  = std::make_unique<bul::Label>("inside");
    bul::Label const* innerPtr = inner.get();
    stack->add(std::move(inner));

    bul::Label outside{"outside"};

    EXPECT_TRUE(sheet.styleFor(*innerPtr).color.has_value());
    EXPECT_FALSE(sheet.styleFor(outside).color.has_value());
}

TEST(StyleSheetRules, ChildCombinatorNeedsTheDirectParent) {
    bul::StyleSheet sheet;
    sheet.type("Stack").child().type("Label").set({.color = kRed});

    auto stack = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    auto inner = std::make_unique<bul::Label>("inside");
    bul::Label const* innerPtr = inner.get();
    stack->add(std::move(inner));

    EXPECT_TRUE(sheet.styleFor(*innerPtr).color.has_value());
}

TEST(StyleSheetRules, StructuralPseudoClasses) {
    bul::StyleSheet sheet;
    sheet.type("Label").pseudo("first-child").set({.color = kRed});
    sheet.type("Label").pseudo("nth-child", "2").set({.color = kGreen});

    auto stack = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
    auto first = std::make_unique<bul::Label>("first");
    auto second = std::make_unique<bul::Label>("second");
    bul::Label const* firstPtr  = first.get();
    bul::Label const* secondPtr = second.get();
    stack->add(std::move(first));
    stack->add(std::move(second));

    EXPECT_EQ(sheet.styleFor(*firstPtr).color, kRed);
    EXPECT_EQ(sheet.styleFor(*secondPtr).color, kGreen);
}

TEST(StyleSheetRules, StatePseudoClasses) {
    bul::StyleSheet sheet;
    sheet.type("Button").pseudo("disabled").set({.color = kRed});
    sheet.type("Button").pseudo("hover").set({.color = kGreen});

    bul::Button disabled{"a"};
    disabled.disabled(true);
    EXPECT_EQ(sheet.styleFor(disabled).color, kRed);

    bul::Button hovered{"b"};
    hovered.notifyHover(true);
    EXPECT_EQ(sheet.styleFor(hovered).color, kGreen);
}

TEST(StyleSheetRules, NotAndHasPseudos) {
    bul::StyleSheet sheet;
    sheet.type("Label").pseudo("not", ".quiet").set({.color = kBlue});

    bul::Label loud{"loud"};
    bul::Label quiet{"quiet"};
    quiet.className("quiet");
    EXPECT_EQ(sheet.styleFor(loud).color, kBlue);
    EXPECT_FALSE(sheet.styleFor(quiet).color.has_value());
}

// ------------------------------------------------------ stylesheet + widget

TEST(StyleSheet, ARuleColoursMatchingWidgets) {
    ResetSheet const      reset;
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::StyleSheet sheet;
    sheet.type("Checkbox").set({.accentColor = kBlue});
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
}

TEST(StyleSheet, ClassAndIdSelectTheWidget) {
    ResetSheet const      reset;
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::StyleSheet sheet;
    sheet.className("danger").set({.color = kRed});
    sheet.id("title").set({.color = kBlue});
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
    sheet.type("Label").set({.color = kRed});
    bul::setStyleSheet(std::move(sheet));

    bul::Label label{"Hello"};
    label.style({.color = kGreen});
    label.layout({0.f, 0.f, 60.f, 9.f}, theme, canvas);
    label.paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 1u);
    EXPECT_EQ(canvas.texts[0].color, kGreen);
}

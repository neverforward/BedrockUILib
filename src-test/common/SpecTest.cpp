#include "gtest/gtest.h"

#include <memory>
#include <string>

#include "FakeCanvas.h"

#include "bul/Spec.h"
#include "bul/Theme.h"

namespace {

[[nodiscard]] bul::Theme testTheme() {
    bul::Theme theme = bul::Theme::neutral();
    theme.fontScale  = 1.f;
    theme.gap        = 6.f;
    return theme;
}

} // namespace

TEST(Spec, BuildsALabel) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build(
        bul::spec::Label{
            .text = "Hello",
            .font = {.size = 2.f},
        }
    );
    auto* label = dynamic_cast<bul::Label*>(tree.get());
    ASSERT_NE(label, nullptr);
    EXPECT_EQ(label->text(), "Hello");

    bul::Size const size = tree->measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 60.f); // five code points at double size
    EXPECT_FLOAT_EQ(size.height, 18.f);
}

TEST(Spec, AppliesTheCommonProperties) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    int  clicks = 0;
    auto tree   = bul::spec::build(
        bul::spec::Label{
            .common = {.fixedWidth = 50.f, .fixedHeight = 20.f, .onClick = [&clicks] { ++clicks; }},
            .text   = "Hello",
    }
    );

    bul::Size const size = tree->measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 50.f);
    EXPECT_FLOAT_EQ(size.height, 20.f);
    EXPECT_TRUE(tree->interactive());

    tree->layout({0.f, 0.f, 100.f, 40.f}, theme, canvas);
    tree->notifyClick({5.f, 5.f});
    EXPECT_EQ(clicks, 1);
}

TEST(Spec, BuildsAStackOfChildren) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build(
        bul::spec::Stack{
            .direction = bul::spec::Stack::Direction::Vertical,
            .gap       = 6.f,
            .children  = {
                          bul::spec::node(bul::spec::Label{.text = "Hello"}),
                          bul::spec::node(bul::spec::Label{.text = "World"}),
                          },
    }
    );

    bul::Size const size = tree->measure(0.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 30.f);
    EXPECT_FLOAT_EQ(size.height, 24.f); // 9 + 6 + 9

    tree->layout({0.f, 0.f, 100.f, 24.f}, theme, canvas);
    bul::test::FakeCanvas painted;
    tree->paint(painted, theme);
    ASSERT_EQ(painted.texts.size(), 2u);
    EXPECT_EQ(painted.texts[0].value, "Hello");
    EXPECT_EQ(painted.texts[1].value, "World");
    EXPECT_FLOAT_EQ(painted.texts[1].topLeft.y, 15.f);
}

TEST(Spec, BuildsAFieldWithAControl) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build(
        bul::spec::Field{
            .label       = "Email",
            .control     = bul::spec::node(bul::spec::Input{.placeholder = "you@example.com"}),
            .description = "Hint",
        }
    );
    auto* field = dynamic_cast<bul::Field*>(tree.get());
    ASSERT_NE(field, nullptr);
    EXPECT_NE(field->controlWidget(), nullptr);
    EXPECT_NE(field->descriptionWidget(), nullptr);

    bul::Size const size = tree->measure(0.f, theme, canvas);
    EXPECT_GT(size.height, 19.f); // label + gap + input + gap + hint

    tree->layout({0.f, 0.f, 200.f, size.height}, theme, canvas);
    bul::test::FakeCanvas painted;
    tree->paint(painted, theme);
    EXPECT_FALSE(painted.texts.empty());
}

TEST(Spec, BuildsAFormOfFields) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build(
        bul::spec::Stack{
            .direction = bul::spec::Stack::Direction::Vertical,
            .gap       = 8.f,
            .children  = {
                          bul::spec::node(bul::spec::Label{.text = "Settings", .font = {.size = 1.2f}}),
                          bul::spec::node(
                    bul::spec::Field{
                        .label   = "Name",
                        .control = bul::spec::node(bul::spec::Input{.placeholder = "Your name"}),
                    }
                ), bul::spec::node(bul::spec::Switch{.text = "Wi-Fi", .checked = true}),
                          },
    }
    );

    bul::Size const size = tree->measure(200.f, theme, canvas);
    EXPECT_FLOAT_EQ(size.width, 120.f); // the column reports its widest child (the input)
    EXPECT_GT(size.height, 40.f);

    tree->layout({0.f, 0.f, 200.f, size.height}, theme, canvas);
    bul::test::FakeCanvas painted;
    tree->paint(painted, theme);
    EXPECT_FALSE(painted.texts.empty());
}

TEST(Spec, BuildsTheStatefulControls) {
    auto  checkbox = bul::spec::build(bul::spec::Checkbox{.text = "Accept", .checked = true});
    auto* box      = dynamic_cast<bul::Checkbox*>(checkbox.get());
    ASSERT_NE(box, nullptr);
    EXPECT_TRUE(box->checked());

    auto  switchWidget = bul::spec::build(bul::spec::Switch{.text = "Wi-Fi"});
    auto* on           = dynamic_cast<bul::Switch*>(switchWidget.get());
    ASSERT_NE(on, nullptr);
    EXPECT_FALSE(on->checked());

    auto select = bul::spec::build(
        bul::spec::Select{
            .placeholder = "Choose",
            .value       = "b",
            .items       = {{"a", "Apple"}, {"b", "Banana"}},
    }
    );
    auto* dropdown = dynamic_cast<bul::Select*>(select.get());
    ASSERT_NE(dropdown, nullptr);
    EXPECT_EQ(dropdown->value(), "b");

    auto group = bul::spec::build(
        bul::spec::RadioGroup{
            .value   = "two",
            .options = {{"one", "One"}, {"two", "Two"}},
    }
    );
    auto* radios = dynamic_cast<bul::RadioGroup*>(group.get());
    ASSERT_NE(radios, nullptr);
    EXPECT_EQ(radios->value(), "two");
}

TEST(Spec, BuildsAGridOfNodes) {
    auto tree = bul::spec::build(
        bul::spec::Grid{
            .columns  = 2,
            .gap      = 4.f,
            .children = {
                         bul::spec::node(bul::spec::Label{.text = "One"}),
                         bul::spec::node(bul::spec::Label{.text = "Two"}),
                         },
    }
    );

    EXPECT_EQ(tree->typeName(), "Grid");
    bul::test::FakeCanvas canvas;
    tree->layout({0.f, 0.f, 100.f, 40.f}, testTheme(), canvas);
    EXPECT_EQ(tree->width(), 100.f);
}

TEST(Spec, BuildsATableWithTracksAndCells) {
    auto tree = bul::spec::build(
        bul::spec::Table{
            .columns = {{.value = 40.f}, {.weight = 1.f}},
            .rows    = {{.weight = 1.f}},
            .cells   = {
                        {.row = 0, .column = 0, .control = bul::spec::node(bul::spec::Label{.text = "Name"})},
                        {.row = 0, .column = 1, .control = bul::spec::node(bul::spec::Label{.text = "Value"})},
                        },
    }
    );

    EXPECT_EQ(tree->typeName(), "Table");
    bul::Table* table = dynamic_cast<bul::Table*>(tree.get());
    ASSERT_NE(table, nullptr);
    EXPECT_TRUE(table->border());
    auto noBorder = bul::spec::build(
        bul::spec::Table{
            .columns = {{.weight = 1.f}},
            .rows    = {{.weight = 1.f}},
            .border  = false,
            .cells   = {{.row = 0, .column = 0, .control = bul::spec::node(bul::spec::Label{.text = "x"})}},
        }
    );
    bul::Table* plain = dynamic_cast<bul::Table*>(noBorder.get());
    ASSERT_NE(plain, nullptr);
    EXPECT_FALSE(plain->border());
}

TEST(Spec, BuildsFieldParts) {
    auto label = bul::spec::build(bul::spec::FieldLabel{.text = "Name", .required = true});
    EXPECT_EQ(label->typeName(), "FieldLabel");

    auto description = bul::spec::build(bul::spec::FieldDescription{.text = "Hint"});
    EXPECT_EQ(description->typeName(), "FieldDescription");
}

TEST(Spec, BuildsAScrollView) {
    auto tree = bul::spec::build(
        bul::spec::ScrollView{
            .viewport = 100.f,
            .content  = bul::spec::node(bul::spec::Label{.text = "Row"}),
        }
    );

    EXPECT_EQ(tree->typeName(), "ScrollView");
    bul::ScrollView* view = dynamic_cast<bul::ScrollView*>(tree.get());
    ASSERT_NE(view, nullptr);
    EXPECT_TRUE(view->scrollbar());
}

TEST(Spec, AFactoryNodeJoinsAStructTree) {
    struct Divider final : bul::Widget {
        [[nodiscard]] std::string_view typeName() const noexcept override { return "Divider"; }
        [[nodiscard]] bul::Size        measureContent(float, bul::Theme const&, bul::Canvas const&) const override {
            return {0.f, 1.f};
        }
        void paintContent(bul::Canvas&, bul::Theme const&) const override {}
    };

    auto tree = bul::spec::build(
        bul::spec::Stack{
            .children = {bul::spec::factory([] { return std::make_unique<Divider>(); })},
        }
    );

    EXPECT_EQ(tree->typeName(), "Stack");
}

TEST(Spec, WeightedStackChildrenShareTheRoom) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build(
        bul::spec::Stack{
            .children = {
                         bul::spec::node(bul::spec::Label{.text = "A"}),
                         bul::spec::weighted(bul::spec::node(bul::spec::Label{.text = "B"}), 1.f),
                         },
    }
    );

    tree->layout({0.f, 0.f, 100.f, 100.f}, theme, canvas);
    tree->paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 2u);
    EXPECT_EQ(canvas.texts[0].topLeft.y, 0.f);
    // A is 9 tall, the gap is the theme's 6, so the weighted B starts at 15 and
    // takes the rest.
    EXPECT_EQ(canvas.texts[1].topLeft.y, 15.f);
}

TEST(Spec, CommonCarriesTheStyleSheetSelectors) {
    auto tree = bul::spec::build(
        bul::spec::Label{
            .common = {.className = "danger", .id = "submit"},
            .text   = "Hello",
    }
    );

    EXPECT_EQ(tree->className(), "danger");
    EXPECT_EQ(tree->id(), "submit");
}

TEST(Spec, LabelTakesLineSpacingAndElide) {
    auto tree = bul::spec::build(
        bul::spec::Label{
            .text        = "Hello",
            .lineSpacing = 1.5f,
            .elide       = true,
        }
    );

    auto* label = dynamic_cast<bul::Label*>(tree.get());
    ASSERT_NE(label, nullptr);
    EXPECT_FLOAT_EQ(label->lineSpacing(), 1.5f);
    EXPECT_TRUE(label->elide());
}

TEST(Spec, SelectTakesItsOpenState) {
    auto tree = bul::spec::build(
        bul::spec::Select{
            .items = {{.value = "a", .label = "A"}},
            .open  = true,
        }
    );

    auto* select = dynamic_cast<bul::Select*>(tree.get());
    ASSERT_NE(select, nullptr);
    EXPECT_TRUE(select->open());
}

TEST(Spec, InputTakesItsFileBits) {
    auto tree = bul::spec::build(
        bul::spec::Input{
            .kind     = bul::Input::Kind::File,
            .filePath = "C:/data/config.json",
        }
    );

    auto* input = dynamic_cast<bul::Input*>(tree.get());
    ASSERT_NE(input, nullptr);
    EXPECT_EQ(input->filePath(), "C:/data/config.json");
}

TEST(Spec, FieldCanUseACustomLabel) {
    auto tree = bul::spec::build(
        bul::spec::Field{
            .labelWidget = bul::spec::node(bul::spec::Badge{.text = "Beta"}),
            .control     = bul::spec::node(bul::spec::Input{.placeholder = "x"}),
        }
    );

    auto* field = dynamic_cast<bul::Field*>(tree.get());
    ASSERT_NE(field, nullptr);
    ASSERT_NE(field->labelWidget(), nullptr);
    EXPECT_EQ(field->labelWidget()->typeName(), "Badge");
}
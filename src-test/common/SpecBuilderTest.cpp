#include "gtest/gtest.h"

#include <memory>
#include <string>

#include "FakeCanvas.h"

#include "bul/SpecBuilder.h"
#include "bul/Theme.h"

namespace {

[[nodiscard]] bul::Theme testTheme() {
    bul::Theme theme = bul::Theme::neutral();
    theme.fontScale  = 1.f;
    theme.gap        = 6.f;
    return theme;
}

} // namespace
namespace {

/// A widget the kit does not know, used to prove the builder takes anything.
class Divider final : public bul::Widget {
public:
    [[nodiscard]] std::string_view typeName() const noexcept override { return "Divider"; }

    [[nodiscard]] bul::Size measureContent(float, bul::Theme const&, bul::Canvas const&) const override {
        return {0.f, 1.f};
    }
    void paintContent(bul::Canvas& canvas, bul::Theme const& theme) const override {
        canvas.fill(bounds(), theme.panelBorder, 1.f);
    }
};

} // namespace

TEST(SpecBuilder, BuildsAColumnOfLeaves) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build([](bul::spec::Builder& ui) {
        ui.label("Title", 1.4f);
        ui.toggle("Notifications", true);
        ui.checkbox("Accept terms");
        ui.button("Apply", [] {});
    });

    ASSERT_NE(tree, nullptr);
    EXPECT_EQ(tree->typeName(), "Stack");
    bul::Size const size = tree->measure(200.f, theme, canvas);
    EXPECT_GT(size.height, 0.f);

    tree->layout({0.f, 0.f, size.width, size.height}, theme, canvas);
    tree->paint(canvas, theme);
    EXPECT_GE(canvas.texts.size(), 4u); // every leaf drew its text
}

TEST(SpecBuilder, RowsNestAndRunSideBySide) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build([](bul::spec::Builder& ui) {
        ui.row([&] {
            ui.button("Yes", [] {});
            ui.button("No", [] {});
        });
    });

    bul::Size const size = tree->measure(200.f, theme, canvas);
    // Two buttons side by side: wider than a single one, and not taller.
    EXPECT_GT(size.width, 40.f);
    EXPECT_LT(size.height, 30.f);
}

TEST(SpecBuilder, FieldWrapsItsControl) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build([](bul::spec::Builder& ui) {
        ui.field("Name", [&] { ui.input("Ada Lovelace"); }, "Field + input.");
    });

    ASSERT_EQ(tree->typeName(), "Stack");
    bul::Size const size = tree->measure(200.f, theme, canvas);
    EXPECT_GT(size.height, 20.f); // label + control + description
}

TEST(SpecBuilder, ScrollRespectsTheViewport) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build([](bul::spec::Builder& ui) {
        ui.scroll(100.f, [&] {
            for (int i = 0; i < 20; ++i) {
                ui.label("Row " + std::to_string(i));
            }
        });
    });

    bul::Size const size = tree->measure(200.f, theme, canvas);
    EXPECT_EQ(size.height, 100.f);
}

TEST(SpecBuilder, GridPlacesItsChildren) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build([](bul::spec::Builder& ui) {
        ui.grid(3, [&] {
            for (int i = 0; i < 6; ++i) {
                ui.label("Cell");
            }
        });
    });

    ASSERT_EQ(tree->typeName(), "Stack");
    bul::Size const size = tree->measure(300.f, theme, canvas);
    EXPECT_GT(size.width, 30.f);
    EXPECT_GT(size.height, 18.f); // two rows
}

TEST(SpecBuilder, TakesAWidgetTheKitDoesNotKnow) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build([](bul::spec::Builder& ui) {
        ui.label("Above");
        ui.add(std::make_unique<Divider>());
        ui.label("Below");
    });

    tree->layout({0.f, 0.f, 100.f, 40.f}, theme, canvas);
    tree->paint(canvas, theme);
    EXPECT_FALSE(canvas.fills.empty()); // the divider painted its rule
}

TEST(SpecBuilder, WithAppliesTheBaseProperties) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build([](bul::spec::Builder& ui) { ui.with({.margin = 4.f}).label("Padded"); });

    // The label is 9 tall; the margin adds 8.
    EXPECT_EQ(tree->measure(0.f, theme, canvas).height, 17.f);
}

TEST(SpecBuilder, AddTakesAStretch) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build([](bul::spec::Builder& ui) {
        ui.label("A");
        ui.add(std::make_unique<bul::Label>("B"), 1.f);
    });

    tree->layout({0.f, 0.f, 100.f, 100.f}, theme, canvas);
    tree->paint(canvas, theme);

    ASSERT_EQ(canvas.texts.size(), 2u);
    EXPECT_EQ(canvas.texts[1].topLeft.y, 15.f); // 9 + the theme gap
}

TEST(SpecBuilder, TableTakesAConfiguration) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build([](bul::spec::Builder& ui) {
        ui.table([](bul::Table& table) {
            table.column({.value = 40.f});
            table.column({.weight = 1.f});
            table.row({.weight = 1.f});
            table.cell(0, 0, std::make_unique<bul::Label>("Name"));
            table.cell(0, 1, std::make_unique<bul::Label>("Value"));
        });
    });

    bul::Size const size = tree->measure(200.f, theme, canvas);
    EXPECT_GT(size.height, 0.f);
    tree->layout({0.f, 0.f, 200.f, size.height}, theme, canvas);
    tree->paint(canvas, theme);
    EXPECT_GE(canvas.texts.size(), 2u);
}

TEST(SpecBuilder, FieldPartsAreLeaves) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    auto tree = bul::spec::build([](bul::spec::Builder& ui) {
        ui.fieldLabel("Name", 1.f, true);
        ui.input("Ada");
        ui.fieldDescription("Hint");
    });

    EXPECT_GT(tree->measure(200.f, theme, canvas).height, 20.f);
}

TEST(SpecBuilder, FindReturnsTheWidgetById) {
    bul::spec::Builder ui;
    ui.id("fruit").select(
        "Pick a fruit",
        {
            {.value = "a", .label = "Apple"}
    }
    );
    auto tree = ui.take();

    auto* select = ui.find<bul::Select>("fruit");
    ASSERT_NE(select, nullptr);
    select->value("a");
    EXPECT_EQ(select->value(), "a");
    EXPECT_EQ(ui.find("missing"), nullptr);
}

TEST(SpecBuilder, ConfigureHookReachesTheWidget) {
    bul::spec::Builder ui;
    ui.id("pw").input("", "", [](bul::Input& input) { input.kind(bul::Input::Kind::Password); });
    auto tree = ui.take();

    auto* input = ui.find<bul::Input>("pw");
    ASSERT_NE(input, nullptr);
    EXPECT_EQ(input->kind(), bul::Input::Kind::Password);
}

TEST(SpecBuilder, CallbacksWorkThroughTheConfigureHook) {
    bul::spec::Builder ui;
    bool               changed = false;
    ui.id("box").checkbox("Accept", false, [&](bul::Checkbox& box) {
        box.onCheckedChange([&](bool value) { changed = value; });
    });
    auto tree = ui.take();

    auto* box = ui.find<bul::Checkbox>("box");
    ASSERT_NE(box, nullptr);
    box->activate();
    EXPECT_TRUE(changed);
}

TEST(SpecBuilder, ChainsCommonPropertiesAfterALeaf) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::spec::Builder ui;
    auto               tree = ui.id("a").label("Chained").margin(4.f).className("danger").build();

    auto* label = ui.find<bul::Label>("a");
    ASSERT_NE(label, nullptr);
    EXPECT_EQ(label->className(), "danger");
    EXPECT_EQ(label->margin().top, 4.f);
    // The label is 9 tall; the margin adds 8.
    EXPECT_EQ(tree->measure(0.f, theme, canvas).height, 17.f);
}

TEST(SpecBuilder, AddsInAChain) {
    auto tree = bul::spec::Builder{}.label("A").button("B", [] {}).build();
    EXPECT_EQ(tree->typeName(), "Stack");
}
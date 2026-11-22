#include "gtest/gtest.h"

#include <memory>

#include "FakeCanvas.h"

#include "bul/Layout.h"
#include "bul/Theme.h"
#include "bul/Widget.h"

namespace {

[[nodiscard]] bul::Theme testTheme() {
    bul::Theme theme = bul::Theme::neutral();
    theme.fontScale  = 1.f;
    theme.gap        = 6.f;
    return theme;
}

/// A cell that measures to a fixed size, so a container's maths is what is
/// under test rather than text metrics.
class Fixed final : public bul::Widget {
public:
    Fixed(float width, float height) { fixedSize(width, height); }

    [[nodiscard]] bul::Size measureContent(float, bul::Theme const&, bul::Canvas const&) const override {
        return {0.f, 0.f};
    }

    void paintContent(bul::Canvas&, bul::Theme const&) const override {}
};

/// A cell with a natural size but no fixed size, so the container's rectangle is
/// what it ends up with.
class Cell final : public bul::Widget {
public:
    [[nodiscard]] bul::Size measureContent(float, bul::Theme const&, bul::Canvas const&) const override {
        return {10.f, 10.f};
    }
    void paintContent(bul::Canvas&, bul::Theme const&) const override {}
};

} // namespace

TEST(Grid, LaysChildrenOutInEqualCells) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Grid grid{2};
    grid.gap(2.f);
    grid.add(std::make_unique<Fixed>(10.f, 10.f));
    grid.add(std::make_unique<Fixed>(10.f, 10.f));
    grid.add(std::make_unique<Fixed>(10.f, 10.f));
    grid.layout({0.f, 0.f, 100.f, 50.f}, theme, canvas);

    // Two columns: (100 - 2) / 2 = 49 wide; two rows: (50 - 2) / 2 = 24 high.
    EXPECT_EQ(grid.width(), 100.f);
    EXPECT_EQ(grid.height(), 50.f);
}

TEST(Grid, MeasureIsTheCellsOfTheLargestChild) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Grid grid{2};
    grid.gap(4.f);
    grid.add(std::make_unique<Fixed>(20.f, 10.f));
    grid.add(std::make_unique<Fixed>(20.f, 10.f));
    grid.add(std::make_unique<Fixed>(20.f, 10.f));

    bul::Size const size = grid.measure(0.f, theme, canvas);
    EXPECT_EQ(size.width, 44.f);  // 2 * 20 + 4
    EXPECT_EQ(size.height, 24.f); // 2 * 10 + 4
}

TEST(Grid, HitTestFindsTheChildUnderThePoint) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Grid grid{2};
    grid.gap(2.f);
    auto         first     = std::make_unique<Fixed>(10.f, 10.f);
    auto         second    = std::make_unique<Fixed>(10.f, 10.f);
    bul::Widget* firstRaw  = first.get();
    bul::Widget* secondRaw = second.get();
    grid.add(std::move(first));
    grid.add(std::move(second));
    grid.layout({0.f, 0.f, 100.f, 50.f}, theme, canvas);

    EXPECT_EQ(grid.hitTest({5.f, 5.f}), firstRaw);
    EXPECT_EQ(grid.hitTest({55.f, 5.f}), secondRaw);
}

TEST(Table, LaysCellsOutOnTheDeclaredTracks) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Table table;
    table.padding(0.f);
    table.cellPadding(0.f);
    table.column({.value = 40.f});
    table.column({.weight = 1.f});
    table.row({.value = 20.f});
    table.row({.weight = 1.f});

    auto         a    = std::make_unique<Cell>();
    auto         b    = std::make_unique<Cell>();
    auto         c    = std::make_unique<Cell>();
    auto         d    = std::make_unique<Cell>();
    bul::Widget* aRaw = a.get();
    bul::Widget* dRaw = d.get();
    table.cell(0, 0, std::move(a));
    table.cell(0, 1, std::move(b));
    table.cell(1, 0, std::move(c));
    table.cell(1, 1, std::move(d));

    table.layout({0.f, 0.f, 100.f, 100.f}, theme, canvas);

    EXPECT_EQ(aRaw->bounds(), (bul::Rect{0.f, 0.f, 40.f, 20.f}));
    EXPECT_EQ(dRaw->bounds(), (bul::Rect{40.f, 20.f, 60.f, 80.f}));
}

TEST(Table, MeasureSumsFixedTracksAndNaturalFlexibleOnes) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Table table;
    table.padding(0.f);
    table.cellPadding(0.f);
    table.column({.value = 40.f});
    table.column({.weight = 1.f});
    table.row({.weight = 1.f});
    table.cell(0, 0, std::make_unique<Fixed>(10.f, 9.f));
    table.cell(0, 1, std::make_unique<Fixed>(30.f, 9.f));

    bul::Size const size = table.measure(0.f, theme, canvas);
    EXPECT_EQ(size.width, 70.f); // 40 + 30
    EXPECT_EQ(size.height, 9.f);
}

TEST(Table, CellsAppendFlexibleTracks) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Table table;
    table.padding(0.f);
    table.cellPadding(0.f);
    table.cell(0, 0, std::make_unique<Fixed>(10.f, 10.f));
    table.cell(1, 2, std::make_unique<Fixed>(10.f, 10.f));

    bul::Size const size = table.measure(0.f, theme, canvas);
    EXPECT_EQ(size.width, 20.f);  // the middle flexible column has no cell to measure
    EXPECT_EQ(size.height, 20.f); // two flexible rows, 10 each
}

TEST(Table, HitTestFindsTheCellUnderThePoint) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Table table;
    table.padding(0.f);
    table.cellPadding(0.f);
    table.row({.weight = 1.f});
    table.row({.weight = 1.f});
    auto         top       = std::make_unique<Fixed>(10.f, 10.f);
    auto         bottom    = std::make_unique<Fixed>(10.f, 10.f);
    bul::Widget* topRaw    = top.get();
    bul::Widget* bottomRaw = bottom.get();
    table.cell(0, 0, std::move(top));
    table.cell(1, 0, std::move(bottom));
    table.layout({0.f, 0.f, 100.f, 100.f}, theme, canvas);

    EXPECT_EQ(table.hitTest({5.f, 5.f}), topRaw);
    EXPECT_EQ(table.hitTest({5.f, 55.f}), bottomRaw);
}

TEST(Table, DrawsABorderByDefault) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Table table;
    table.column({.weight = 1.f});
    table.column({.weight = 1.f});
    table.row({.weight = 1.f});
    table.row({.weight = 1.f});
    table.cell(0, 0, std::make_unique<Cell>());
    table.cell(1, 1, std::make_unique<Cell>());
    table.layout({0.f, 0.f, 100.f, 100.f}, theme, canvas);
    table.paint(canvas, theme);

    ASSERT_EQ(canvas.strokes.size(), 1u);
    EXPECT_EQ(canvas.strokes[0].rect, (bul::Rect{0.f, 0.f, 100.f, 100.f}));
    // One column separator and one row separator, border to border.
    ASSERT_EQ(canvas.fills.size(), 2u);
    EXPECT_EQ(canvas.fills[0].rect, (bul::Rect{49.5f, 0.f, 1.f, 100.f}));
    EXPECT_EQ(canvas.fills[1].rect, (bul::Rect{0.f, 49.5f, 100.f, 1.f}));
}

TEST(Table, BorderCanBeTurnedOff) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Table table;
    table.border(false);
    table.column({.weight = 1.f});
    table.row({.weight = 1.f});
    table.cell(0, 0, std::make_unique<Cell>());
    table.layout({0.f, 0.f, 100.f, 100.f}, theme, canvas);
    table.paint(canvas, theme);

    EXPECT_TRUE(canvas.strokes.empty());
    EXPECT_TRUE(canvas.fills.empty());
}

TEST(Table, PaddingInsetsTheCellsFromTheBorder) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Table table;
    table.padding(3.f, 2.f);
    table.cellPadding(0.f);
    table.column({.weight = 1.f});
    table.row({.weight = 1.f});
    auto         cell  = std::make_unique<Cell>();
    bul::Widget* cell_ = cell.get();
    table.cell(0, 0, std::move(cell));
    table.layout({0.f, 0.f, 100.f, 100.f}, theme, canvas);

    EXPECT_EQ(cell_->bounds(), (bul::Rect{3.f, 2.f, 94.f, 96.f}));
}

TEST(Table, CellPaddingInsetsTheContentInsideItsCell) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Table table;
    table.padding(0.f);
    table.cellPadding(3.f, 2.f);
    table.column({.weight = 1.f});
    table.row({.weight = 1.f});
    auto         cell  = std::make_unique<Cell>();
    bul::Widget* cell_ = cell.get();
    table.cell(0, 0, std::move(cell));
    table.layout({0.f, 0.f, 100.f, 100.f}, theme, canvas);

    EXPECT_EQ(cell_->bounds(), (bul::Rect{3.f, 2.f, 94.f, 96.f}));
}

TEST(Table, AFixedTrackIsTheContentSizePlusPadding) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Table table;
    table.column({.weight = 1.f});
    table.row({.value = 9.f});
    auto         cell  = std::make_unique<Cell>();
    bul::Widget* cell_ = cell.get();
    table.cell(0, 0, std::move(cell));

    bul::Size const size = table.measure(0.f, theme, canvas);
    EXPECT_EQ(size.height, 17.f); // 9 content + 8 cell padding

    table.layout({0.f, 0.f, 100.f, size.height}, theme, canvas);
    EXPECT_EQ(cell_->bounds(), (bul::Rect{4.f, 4.f, 92.f, 9.f}));
}
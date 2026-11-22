#include "gtest/gtest.h"

#include "bul/Layout.h"

using namespace bul;
using namespace bul::layout;

TEST(Layout, BoxStacksVerticallyAndStretchesAcross) {
    auto const rects =
        box({0.f, 0.f, 100.f, 100.f},
            {Item{{40.f, 10.f}}, Item{{60.f, 20.f}}},
            BoxSpec{.axis = Axis::Vertical, .gap = 4.f});

    ASSERT_EQ(rects.size(), 2u);
    EXPECT_EQ(rects[0], (Rect{0.f, 0.f, 100.f, 10.f}));
    EXPECT_EQ(rects[1], (Rect{0.f, 14.f, 100.f, 20.f}));
}

TEST(Layout, BoxGrowSharesTheLeftover) {
    auto const rects = box(
        {
            0.f,
            0.f,
            100.f,
            100.f
    },
        {Item{{10.f, 10.f}}, Item{{10.f, 20.f}, 1.f}},
        BoxSpec{.axis = Axis::Vertical, .gap = 4.f}
    );

    ASSERT_EQ(rects.size(), 2u);
    // 100 - 4 gap - 10 fixed = 86 for the grow item.
    EXPECT_EQ(rects[1], (Rect{0.f, 14.f, 100.f, 86.f}));
}

TEST(Layout, BoxGrowWeightsSplitProportionally) {
    auto const rects = box(
        {0.f, 0.f, 100.f, 90.f},
        {Item{{0.f, 0.f}, 1.f}, Item{{0.f, 0.f}, 2.f}},
        BoxSpec{.axis = Axis::Vertical}
    );

    ASSERT_EQ(rects.size(), 2u);
    EXPECT_EQ(rects[0].height, 30.f);
    EXPECT_EQ(rects[1].height, 60.f);
}

TEST(Layout, BoxRowKeepsChildHeights) {
    auto const rects =
        box({0.f, 0.f, 100.f, 100.f},
            {Item{{30.f, 20.f}}, Item{{20.f, 10.f}}},
            BoxSpec{.axis = Axis::Horizontal, .gap = 5.f, .align = Align::Start});

    ASSERT_EQ(rects.size(), 2u);
    EXPECT_EQ(rects[0], (Rect{0.f, 0.f, 30.f, 20.f}));
    EXPECT_EQ(rects[1], (Rect{35.f, 0.f, 20.f, 10.f}));
}

TEST(Layout, BoxAlignsOnTheCrossAxis) {
    auto const centered =
        box({0.f, 0.f, 100.f, 100.f}, {Item{{40.f, 10.f}}}, BoxSpec{.axis = Axis::Vertical, .align = Align::Center});
    ASSERT_EQ(centered.size(), 1u);
    EXPECT_EQ(centered[0], (Rect{30.f, 0.f, 40.f, 10.f}));

    auto const ended =
        box({0.f, 0.f, 100.f, 100.f}, {Item{{40.f, 10.f}}}, BoxSpec{.axis = Axis::Vertical, .align = Align::End});
    ASSERT_EQ(ended.size(), 1u);
    EXPECT_EQ(ended[0], (Rect{60.f, 0.f, 40.f, 10.f}));
}

TEST(Layout, BoxAppliesPadding) {
    auto const rects = box(
        {
            0.f,
            0.f,
            100.f,
            100.f
    },
        {Item{{0.f, 10.f}}},
        BoxSpec{.axis = Axis::Vertical, .padding = Insets{5.f, 6.f, 5.f, 6.f}}
    );

    ASSERT_EQ(rects.size(), 1u);
    EXPECT_EQ(rects[0], (Rect{5.f, 6.f, 90.f, 10.f}));
}

TEST(Layout, BoxHandlesNoItems) { EXPECT_TRUE(box({0.f, 0.f, 100.f, 100.f}, {}).empty()); }

TEST(Layout, ExtentsAddGapsAndTakeTheWidestCross) {
    std::vector<Size> const sizes{
        {10.f, 20.f},
        {30.f, 5.f }
    };

    EXPECT_EQ(mainExtent(Axis::Vertical, sizes, 4.f), 29.f);
    EXPECT_EQ(mainExtent(Axis::Horizontal, sizes, 4.f), 44.f);
    EXPECT_EQ(crossExtent(Axis::Vertical, sizes), 30.f);
    EXPECT_EQ(crossExtent(Axis::Horizontal, sizes), 20.f);
}

TEST(Layout, GridSplitsIntoEqualCells) {
    std::vector<Size> const sizes(5, Size{10.f, 10.f});
    auto const              rects = grid({0.f, 0.f, 100.f, 50.f}, sizes, GridSpec{.columns = 3, .gap = 2.f});

    ASSERT_EQ(rects.size(), 5u);
    // Columns: (100 - 4) / 3 = 32 wide, rows: (50 - 2) / 2 = 24 high.
    EXPECT_EQ(rects[0], (Rect{0.f, 0.f, 32.f, 24.f}));
    EXPECT_EQ(rects[1], (Rect{34.f, 0.f, 32.f, 24.f}));
    EXPECT_EQ(rects[2], (Rect{68.f, 0.f, 32.f, 24.f}));
    EXPECT_EQ(rects[3], (Rect{0.f, 26.f, 32.f, 24.f}));
    EXPECT_EQ(rects[4], (Rect{34.f, 26.f, 32.f, 24.f}));
}

TEST(Layout, GridAlignsSmallerItemsInsideTheirCell) {
    std::vector<Size> const sizes{
        {10.f, 10.f}
    };
    auto const rects = grid({0.f, 0.f, 100.f, 40.f}, sizes, GridSpec{.columns = 1, .align = Align::Center});

    ASSERT_EQ(rects.size(), 1u);
    EXPECT_EQ(rects[0], (Rect{45.f, 15.f, 10.f, 10.f}));
}

TEST(Layout, TableSolvesFixedAndFlexTracks) {
    std::vector<Track> const columns{{.value = 40.f}, {.weight = 1.f}, {.weight = 1.f}};
    std::vector<Track> const rows{{.value = 20.f}, {.weight = 1.f}};
    auto const               cells = table({0.f, 0.f, 100.f, 100.f}, columns, rows);

    ASSERT_EQ(cells.size(), 6u);
    EXPECT_EQ(cells[0], (Rect{0.f, 0.f, 40.f, 20.f}));
    EXPECT_EQ(cells[1], (Rect{40.f, 0.f, 30.f, 20.f}));
    EXPECT_EQ(cells[2], (Rect{70.f, 0.f, 30.f, 20.f}));
    EXPECT_EQ(cells[3], (Rect{0.f, 20.f, 40.f, 80.f}));
    EXPECT_EQ(cells[5], (Rect{70.f, 20.f, 30.f, 80.f}));
}

TEST(Layout, TableAppliesGaps) {
    std::vector<Track> const columns{{.weight = 1.f}, {.weight = 1.f}};
    std::vector<Track> const rows{{.weight = 1.f}};
    auto const cells = table({0.f, 0.f, 100.f, 50.f}, columns, rows, TableSpec{.columnGap = 10.f, .rowGap = 0.f});

    ASSERT_EQ(cells.size(), 2u);
    EXPECT_EQ(cells[0], (Rect{0.f, 0.f, 45.f, 50.f}));
    EXPECT_EQ(cells[1], (Rect{55.f, 0.f, 45.f, 50.f}));
}

TEST(Layout, ScrollClampsToTheContentRange) {
    ScrollRange const range{200.f, 100.f};

    EXPECT_EQ(clampScroll(-5.f, range), 0.f);
    EXPECT_EQ(clampScroll(50.f, range), 50.f);
    EXPECT_EQ(clampScroll(150.f, range), 100.f);
    EXPECT_EQ(clampScroll(50.f, ScrollRange{50.f, 100.f}), 0.f);
}

TEST(Layout, ScrollTellsWhichItemsAreVisible) {
    ScrollRange const range{200.f, 100.f};

    EXPECT_TRUE(isVisible(range, 0.f, 0.f, 20.f));
    EXPECT_TRUE(isVisible(range, 0.f, 95.f, 20.f));
    EXPECT_FALSE(isVisible(range, 0.f, 100.f, 20.f));
    EXPECT_TRUE(isVisible(range, 60.f, 100.f, 20.f));
}

TEST(Layout, ScrollThumbShrinksWithTheContent) {
    ScrollRange const range{200.f, 100.f};

    Thumb const top = scrollThumb(range, 0.f, 100.f);
    EXPECT_EQ(top.start, 0.f);
    EXPECT_EQ(top.length, 50.f);

    Thumb const bottom = scrollThumb(range, 100.f, 100.f);
    EXPECT_EQ(bottom.start, 50.f);
    EXPECT_EQ(bottom.length, 50.f);

    Thumb const whole = scrollThumb(ScrollRange{50.f, 100.f}, 0.f, 100.f);
    EXPECT_EQ(whole.start, 0.f);
    EXPECT_EQ(whole.length, 100.f);
}

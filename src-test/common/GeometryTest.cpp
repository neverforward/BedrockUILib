#include "gtest/gtest.h"

#include "bul/Geometry.h"

using bul::Point;
using bul::Rect;

TEST(Geometry, edgesAreDerived) {
    Rect const r{10.f, 20.f, 30.f, 40.f};
    EXPECT_FLOAT_EQ(r.right(), 40.f);
    EXPECT_FLOAT_EQ(r.bottom(), 60.f);
}

TEST(Geometry, containsExcludesRightAndBottomEdges) {
    Rect const r{0.f, 0.f, 10.f, 10.f};
    EXPECT_TRUE(r.contains({0.f, 0.f}));
    EXPECT_TRUE(r.contains({9.99f, 9.99f}));
    EXPECT_FALSE(r.contains({10.f, 5.f}));
    EXPECT_FALSE(r.contains({5.f, 10.f}));
    EXPECT_FALSE(r.contains({-0.01f, 5.f}));
}

TEST(Geometry, emptyWhenEitherSideCollapses) {
    EXPECT_TRUE((Rect{0.f, 0.f, 0.f, 10.f}).empty());
    EXPECT_TRUE((Rect{0.f, 0.f, 10.f, -1.f}).empty());
    EXPECT_FALSE((Rect{0.f, 0.f, 1.f, 1.f}).empty());
    EXPECT_TRUE((Rect{0.f, 0.f, 0.f, 0.f}).contains({0.f, 0.f}) == false);
}

TEST(Geometry, insetShrinksEverySide) {
    Rect const outer{0.f, 0.f, 100.f, 50.f};
    Rect const inner = outer.inset(10.f, 5.f);
    EXPECT_FLOAT_EQ(inner.x, 10.f);
    EXPECT_FLOAT_EQ(inner.y, 5.f);
    EXPECT_FLOAT_EQ(inner.width, 80.f);
    EXPECT_FLOAT_EQ(inner.height, 40.f);
    EXPECT_FLOAT_EQ(outer.inset(2.f).width, 96.f);
}

TEST(Geometry, fromCenterSizeCenters) {
    Rect const r = Rect::fromCenterSize({100.f, 80.f}, 40.f, 20.f);
    EXPECT_FLOAT_EQ(r.x, 80.f);
    EXPECT_FLOAT_EQ(r.y, 70.f);
    EXPECT_FLOAT_EQ(r.width, 40.f);
    EXPECT_FLOAT_EQ(r.height, 20.f);
    EXPECT_TRUE(r.contains({100.f, 80.f}));
}

TEST(Geometry, equalityIsStructural) {
    EXPECT_EQ((Rect{1.f, 2.f, 3.f, 4.f}), (Rect{1.f, 2.f, 3.f, 4.f}));
    EXPECT_NE((Rect{1.f, 2.f, 3.f, 4.f}), (Rect{1.f, 2.f, 3.f, 5.f}));
}

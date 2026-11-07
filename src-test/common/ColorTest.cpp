#include "gtest/gtest.h"

#include "bul/Color.h"

#include <cstdint>

using bul::Rgba;

TEST(Color, fromRgbSplitsChannels) {
    Rgba const c = Rgba::fromRgb(0x112233u);
    EXPECT_FLOAT_EQ(c.r, 0x11 / 255.f);
    EXPECT_FLOAT_EQ(c.g, 0x22 / 255.f);
    EXPECT_FLOAT_EQ(c.b, 0x33 / 255.f);
    EXPECT_FLOAT_EQ(c.a, 1.f);
}

TEST(Color, fromArgbKeepsAlpha) {
    Rgba const c = Rgba::fromArgb(0x80010203u);
    EXPECT_FLOAT_EQ(c.r, 1.f / 255.f);
    EXPECT_FLOAT_EQ(c.g, 2.f / 255.f);
    EXPECT_FLOAT_EQ(c.b, 3.f / 255.f);
    EXPECT_NEAR(c.a, 0x80 / 255.f, 1e-6);
}

TEST(Color, withAlphaReplacesOnlyAlpha) {
    Rgba const base  = Rgba::fromRgb(0xAABBCCu);
    Rgba const faded = base.withAlpha(0.25f);
    EXPECT_FLOAT_EQ(faded.r, base.r);
    EXPECT_FLOAT_EQ(faded.g, base.g);
    EXPECT_FLOAT_EQ(faded.b, base.b);
    EXPECT_FLOAT_EQ(faded.a, 0.25f);
}

TEST(Color, extremesAreExact) {
    Rgba const black = Rgba::fromRgb(0x000000u);
    Rgba const white = Rgba::fromRgb(0xFFFFFFu);
    EXPECT_FLOAT_EQ(black.r, 0.f);
    EXPECT_FLOAT_EQ(white.r, 1.f);
    EXPECT_FLOAT_EQ(white.a, 1.f);
}

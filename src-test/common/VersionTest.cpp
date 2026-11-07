#include "gtest/gtest.h"

#include "bul/Version.h"

#include <string>

TEST(Version, matchesMacros) {
    EXPECT_EQ(bul::version.major, BUL_VERSION_MAJOR);
    EXPECT_EQ(bul::version.minor, BUL_VERSION_MINOR);
    EXPECT_EQ(bul::version.patch, BUL_VERSION_PATCH);
}

TEST(Version, stringAgreesWithStruct) {
    std::string const expected = std::to_string(bul::version.major) + '.' + std::to_string(bul::version.minor) + '.'
                               + std::to_string(bul::version.patch);
    EXPECT_EQ(bul::versionString, expected);
    EXPECT_EQ(bul::versionString, BUL_VERSION_STRING);
}

TEST(Version, abiTracksMajor) { EXPECT_EQ(bul::abiVersion, bul::version.major); }

TEST(Version, equalityIsStructural) {
    EXPECT_EQ(bul::version, (bul::Version{BUL_VERSION_MAJOR, BUL_VERSION_MINOR, BUL_VERSION_PATCH}));
    EXPECT_NE(bul::version, (bul::Version{0, 0, BUL_VERSION_PATCH + 1}));
}

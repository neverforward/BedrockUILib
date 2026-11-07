#pragma once

#include <string_view>

// Version of the library. Keep it in sync with CHANGELOG.md and the git tag.
//
// 0.x policy: source compatibility only. A breaking change bumps the minor
// version and is recorded under ### Changed in CHANGELOG.md.
#define BUL_VERSION_MAJOR  0
#define BUL_VERSION_MINOR  0
#define BUL_VERSION_PATCH  0
#define BUL_VERSION_STRING "0.0.0"

namespace bul {

/// Semantic version, compile-time constant so tests and consumers can check it
/// without linking the library.
struct Version {
    unsigned major;
    unsigned minor;
    unsigned patch;

    friend constexpr bool operator==(Version const&, Version const&) = default;
};

inline constexpr Version version{
    Version{BUL_VERSION_MAJOR, BUL_VERSION_MINOR, BUL_VERSION_PATCH}
};

inline constexpr std::string_view versionString{BUL_VERSION_STRING};

/// ABI generation. Consumers must be rebuilt when this changes; within one
/// generation the exported headers stay compatible.
inline constexpr unsigned abiVersion{BUL_VERSION_MAJOR};

} // namespace bul

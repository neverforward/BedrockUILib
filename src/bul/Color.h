#pragma once

#include <cstdint>

namespace bul {

/// Straight (non pre-multiplied) RGBA, components in [0, 1].
///
/// The library never stores an engine colour type in its own model: the engine
/// adapter converts at the boundary, so theme values and tests stay independent
/// of the SDK.
struct Rgba {
    float r{};
    float g{};
    float b{};
    float a{};

    [[nodiscard]] static constexpr Rgba fromRgb(std::uint32_t hex, float alpha = 1.f) noexcept {
        return {
            static_cast<float>((hex >> 16) & 0xFFu) / 255.f,
            static_cast<float>((hex >> 8) & 0xFFu) / 255.f,
            static_cast<float>(hex & 0xFFu) / 255.f,
            alpha,
        };
    }

    /// Java-style 0xAARRGGBB.
    [[nodiscard]] static constexpr Rgba fromArgb(std::uint32_t hex) noexcept {
        return {
            static_cast<float>((hex >> 16) & 0xFFu) / 255.f,
            static_cast<float>((hex >> 8) & 0xFFu) / 255.f,
            static_cast<float>(hex & 0xFFu) / 255.f,
            static_cast<float>((hex >> 24) & 0xFFu) / 255.f,
        };
    }

    [[nodiscard]] constexpr Rgba withAlpha(float alpha) const noexcept { return {r, g, b, alpha}; }

    friend constexpr bool operator==(Rgba const&, Rgba const&) = default;
};

} // namespace bul

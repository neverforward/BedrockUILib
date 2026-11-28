#pragma once

namespace bul {

/// A point in GUI units (the coordinate space the game's UI pass uses).
struct Point {
    float x{};
    float y{};
};

/// An axis-aligned rectangle in GUI units.
struct Rect {
    float x{};
    float y{};
    float width{};
    float height{};

    [[nodiscard]] constexpr float right() const noexcept { return x + width; }
    [[nodiscard]] constexpr float bottom() const noexcept { return y + height; }

    [[nodiscard]] constexpr bool empty() const noexcept { return width <= 0.f || height <= 0.f; }

    [[nodiscard]] constexpr bool contains(Point p) const noexcept {
        return p.x >= x && p.x < right() && p.y >= y && p.y < bottom();
    }

    /// Shrinks the rectangle by the same amount on all four sides.
    [[nodiscard]] constexpr Rect inset(float dx, float dy) const noexcept {
        return {x + dx, y + dy, width - 2.f * dx, height - 2.f * dy};
    }

    /// Shrinks the rectangle by an inset on all four sides.
    [[nodiscard]] constexpr Rect inset(float all) const noexcept { return inset(all, all); }

    [[nodiscard]] static constexpr Rect fromCenterSize(Point center, float width, float height) noexcept {
        return {center.x - width / 2.f, center.y - height / 2.f, width, height};
    }

    friend constexpr bool operator==(Rect const&, Rect const&) = default;
};

/// Desired size produced by a widget's measure pass.
struct Size {
    float width{};
    float height{};

    [[nodiscard]] constexpr Size grownBy(float dx, float dy) const noexcept { return {width + dx, height + dy}; }
};

/// Four-sided spacing.
struct Insets {
    float left{};
    float top{};
    float right{};
    float bottom{};

    [[nodiscard]] constexpr float horizontal() const noexcept { return left + right; }
    [[nodiscard]] constexpr float vertical() const noexcept { return top + bottom; }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return left == 0.f && top == 0.f && right == 0.f && bottom == 0.f;
    }

    /// The same amount on all four sides, like `padding: n` in a stylesheet.
    [[nodiscard]] static constexpr Insets all(float value) noexcept { return {value, value, value, value}; }

    /// `horizontal` on the left and right, `vertical` on the top and bottom.
    [[nodiscard]] static constexpr Insets symmetric(float horizontal, float vertical) noexcept {
        return {horizontal, vertical, horizontal, vertical};
    }

    friend constexpr bool operator==(Insets const&, Insets const&) = default;
};

} // namespace bul

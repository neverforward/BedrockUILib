#pragma once

namespace bul {

/// A style length: a number and a unit. A bare number is a GUI unit, which is what
/// the engine's UI pass measures in, so `4.f` and `4_px` mean the same thing.
///
/// Absolute units follow the 96 dpi convention: `in` 96, `cm` 96/2.54, `mm` 96/25.4,
/// `pt` 96/72, `pc` 16. Relative units are resolved against the base font of the
/// theme that receives the style: `em` is one font height, `ex` half of it, `%`
/// a hundredth of it (the kit has no containing block to measure against at
/// style-resolution time).
struct Length {
    enum class Unit { Px, Em, Ex, Percent, Cm, Mm, In, Pt, Pc };

    float value{};
    Unit  unit{Unit::Px};

    constexpr Length() noexcept = default;
    /// A bare number is a GUI unit.
    constexpr Length(float value) noexcept
    : value(value) {}
    constexpr Length(float value, Unit unit) noexcept
    : value(value),
      unit(unit) {}

    /// The length in GUI units, given the base font height to resolve relative
    /// units against.
    [[nodiscard]] constexpr float resolve(float baseFont) const noexcept {
        switch (unit) {
        case Unit::Px:
            return value;
        case Unit::Em:
            return value * baseFont;
        case Unit::Ex:
            return value * baseFont * 0.5f;
        case Unit::Percent:
            return value * baseFont / 100.f;
        case Unit::Cm:
            return value * (96.f / 2.54f);
        case Unit::Mm:
            return value * (96.f / 25.4f);
        case Unit::In:
            return value * 96.f;
        case Unit::Pt:
            return value * (96.f / 72.f);
        case Unit::Pc:
            return value * 16.f;
        }
        return value;
    }

    friend constexpr bool operator==(Length const&, Length const&) = default;
};

} // namespace bul

namespace bul::literals {

[[nodiscard]] constexpr Length operator""_px(unsigned long long value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Px};
}
[[nodiscard]] constexpr Length operator""_px(long double value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Px};
}
[[nodiscard]] constexpr Length operator""_em(unsigned long long value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Em};
}
[[nodiscard]] constexpr Length operator""_em(long double value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Em};
}
[[nodiscard]] constexpr Length operator""_ex(unsigned long long value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Ex};
}
[[nodiscard]] constexpr Length operator""_ex(long double value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Ex};
}
/// The percent unit; `%` cannot be a literal suffix in C++.
[[nodiscard]] constexpr Length operator""_pct(unsigned long long value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Percent};
}
[[nodiscard]] constexpr Length operator""_pct(long double value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Percent};
}
[[nodiscard]] constexpr Length operator""_cm(unsigned long long value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Cm};
}
[[nodiscard]] constexpr Length operator""_cm(long double value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Cm};
}
[[nodiscard]] constexpr Length operator""_mm(unsigned long long value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Mm};
}
[[nodiscard]] constexpr Length operator""_mm(long double value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Mm};
}
[[nodiscard]] constexpr Length operator""_in(unsigned long long value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::In};
}
[[nodiscard]] constexpr Length operator""_in(long double value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::In};
}
[[nodiscard]] constexpr Length operator""_pt(unsigned long long value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Pt};
}
[[nodiscard]] constexpr Length operator""_pt(long double value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Pt};
}
[[nodiscard]] constexpr Length operator""_pc(unsigned long long value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Pc};
}
[[nodiscard]] constexpr Length operator""_pc(long double value) noexcept {
    return Length{static_cast<float>(value), Length::Unit::Pc};
}

} // namespace bul::literals

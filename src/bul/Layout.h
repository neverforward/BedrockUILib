#pragma once

#include <cstddef>
#include <vector>

#include "bul/Export.h"
#include "bul/Geometry.h"

/// Layout solving: pure geometry, no widgets and no engine types, so every rule
/// here is unit tested. A container measures its children, then asks this module
/// where they go.
namespace bul::layout {

/// Which way a box lays its children out.
enum class Axis { Horizontal, Vertical };

/// Placement on the axis the box does not grow along.
enum class Align { Start, Center, End, Stretch };

/// One child of a box: what it measured, and whether it shares the space the
/// fixed children leave over.
struct Item {
    Size size{};
    /// > 0: the child takes this share of the leftover main-axis space instead
    /// of its measured size -- the "the field takes the rest of the row" case.
    float grow{0.f};
};

struct BoxSpec {
    Axis   axis{Axis::Vertical};
    float  gap{0.f};
    Insets padding{};
    /// Cross-axis placement. `Stretch` gives the child the full cross extent,
    /// which is what a column does to its rows (the usual full-width row).
    Align align{Align::Stretch};
};

/// Lays `items` out inside `area`: main-axis sizes (grow items share what the
/// fixed ones leave), `gap` between neighbours, `padding` inside `area`, and
/// cross-axis placement by `spec.align`.
[[nodiscard]] BUL_API std::vector<Rect> box(Rect const& area, std::vector<Item> const& items, BoxSpec const& spec = {});

/// Main-axis extent of `sizes` plus `gap` between them.
[[nodiscard]] BUL_API float mainExtent(Axis axis, std::vector<Size> const& sizes, float gap);

/// Largest cross-axis extent of `sizes`.
[[nodiscard]] BUL_API float crossExtent(Axis axis, std::vector<Size> const& sizes);

struct GridSpec {
    std::size_t columns{1};
    float       gap{0.f};
    Align       align{Align::Stretch};
};

/// Equal cells, row-major: the rows split the height evenly, so a grid of
/// `n` items always fills `area` completely.
[[nodiscard]] BUL_API std::vector<Rect>
grid(Rect const& area, std::vector<Size> const& sizes, GridSpec const& spec = {});

/// A table track: a fixed `value`, or a share of the leftover when `weight` > 0.
struct Track {
    float value{0.f};
    float weight{0.f};
};

struct TableSpec {
    float columnGap{0.f};
    float rowGap{0.f};
};

/// Cell rectangles, row-major (`row * columns.size() + column`).
[[nodiscard]] BUL_API std::vector<Rect>
table(Rect const& area, std::vector<Track> const& columns, std::vector<Track> const& rows, TableSpec const& spec = {});

// --------------------------------------------------------------- scrolling

/// How much content a viewport holds and how much of it is visible.
struct ScrollRange {
    float content{0.f};
    float viewport{0.f};
};

/// `offset` pulled back into [0, max(0, content - viewport)].
[[nodiscard]] BUL_API float clampScroll(float offset, ScrollRange const& range);

/// Whether [start, start + extent) intersects the band visible at `offset`.
[[nodiscard]] BUL_API bool isVisible(ScrollRange const& range, float offset, float start, float extent);

/// Thumb geometry of a scrollbar: the bar is `track` units long, the thumb never
/// gets shorter than `minLength`.
struct Thumb {
    float start{0.f};
    float length{0.f};
};

[[nodiscard]] BUL_API Thumb scrollThumb(ScrollRange const& range, float offset, float track, float minLength = 8.f);

} // namespace bul::layout

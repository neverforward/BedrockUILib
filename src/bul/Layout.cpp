#include "bul/Layout.h"

#include <algorithm>
#include <cmath>

namespace bul::layout {

namespace {

[[nodiscard]] float mainOf(Axis axis, Size size) noexcept { return axis == Axis::Vertical ? size.height : size.width; }

[[nodiscard]] float crossOf(Axis axis, Size size) noexcept { return axis == Axis::Vertical ? size.width : size.height; }

/// Inner rectangle after `padding`, never negative.
[[nodiscard]] Rect padded(Rect const& area, Insets padding) noexcept {
    return {
        area.x + padding.left,
        area.y + padding.top,
        std::max(area.width - padding.horizontal(), 0.f),
        std::max(area.height - padding.vertical(), 0.f),
    };
}

/// Splits the leftover of a track list (fixed values plus flex weights).
[[nodiscard]] std::vector<float> solveTracks(float available, std::vector<Track> const& tracks, float gap) {
    std::vector<float> sizes;
    sizes.reserve(tracks.size());
    float fixed  = 0.f;
    float weight = 0.f;
    for (Track const& track : tracks) {
        if (track.weight > 0.f) {
            weight += track.weight;
        } else {
            fixed += track.value;
        }
    }
    float const gaps     = gap * static_cast<float>(tracks.size() > 1 ? tracks.size() - 1 : 0);
    float const leftover = std::max(available - gaps - fixed, 0.f);
    for (Track const& track : tracks) {
        if (track.weight > 0.f) {
            sizes.push_back(weight > 0.f ? leftover * (track.weight / weight) : 0.f);
        } else {
            sizes.push_back(std::max(track.value, 0.f));
        }
    }
    return sizes;
}

} // namespace

std::vector<Rect> box(Rect const& area, std::vector<Item> const& items, BoxSpec const& spec) {
    std::vector<Rect> out;
    if (items.empty()) {
        return out;
    }
    bool const  vertical  = spec.axis == Axis::Vertical;
    Rect const  inner     = padded(area, spec.padding);
    float const available = vertical ? inner.height : inner.width;

    float fixed  = 0.f;
    float weight = 0.f;
    for (Item const& item : items) {
        if (item.grow > 0.f) {
            weight += item.grow;
        } else {
            fixed += mainOf(spec.axis, item.size);
        }
    }
    float const gaps     = spec.gap * static_cast<float>(items.size() - 1);
    float const leftover = std::max(available - gaps - fixed, 0.f);

    out.reserve(items.size());
    float cursor = vertical ? inner.y : inner.x;
    for (Item const& item : items) {
        float const main =
            item.grow > 0.f && weight > 0.f ? leftover * (item.grow / weight) : mainOf(spec.axis, item.size);
        float cross = crossOf(spec.axis, item.size);
        if (spec.align == Align::Stretch) {
            cross = vertical ? inner.width : inner.height;
        }
        float offset = 0.f;
        if (spec.align == Align::Center) {
            offset = ((vertical ? inner.width : inner.height) - cross) * 0.5f;
        } else if (spec.align == Align::End) {
            offset = (vertical ? inner.width : inner.height) - cross;
        }

        if (vertical) {
            out.push_back({inner.x + offset, cursor, cross, main});
        } else {
            out.push_back({cursor, inner.y + offset, main, cross});
        }
        cursor += main + spec.gap;
    }
    return out;
}

float mainExtent(Axis axis, std::vector<Size> const& sizes, float gap) {
    float extent = 0.f;
    for (std::size_t i = 0; i < sizes.size(); ++i) {
        extent += mainOf(axis, sizes[i]);
        if (i + 1 < sizes.size()) {
            extent += gap;
        }
    }
    return extent;
}

float crossExtent(Axis axis, std::vector<Size> const& sizes) {
    float extent = 0.f;
    for (Size const& size : sizes) {
        extent = std::max(extent, crossOf(axis, size));
    }
    return extent;
}

std::vector<Rect> grid(Rect const& area, std::vector<Size> const& sizes, GridSpec const& spec) {
    std::vector<Rect> out;
    if (sizes.empty()) {
        return out;
    }
    std::size_t const columns = std::max<std::size_t>(spec.columns, 1);
    std::size_t const rows    = (sizes.size() + columns - 1) / columns;

    float const columnGap  = spec.gap * static_cast<float>(columns > 1 ? columns - 1 : 0);
    float const rowGap     = spec.gap * static_cast<float>(rows > 1 ? rows - 1 : 0);
    float const cellWidth  = (area.width - columnGap) / static_cast<float>(columns);
    float const cellHeight = (area.height - rowGap) / static_cast<float>(rows);

    out.reserve(sizes.size());
    for (std::size_t i = 0; i < sizes.size(); ++i) {
        std::size_t const row    = i / columns;
        std::size_t const column = i % columns;
        float const       cellX  = area.x + static_cast<float>(column) * (cellWidth + spec.gap);
        float const       cellY  = area.y + static_cast<float>(row) * (cellHeight + spec.gap);

        float width  = cellWidth;
        float height = cellHeight;
        if (spec.align != Align::Stretch) {
            width  = std::min(sizes[i].width, cellWidth);
            height = std::min(sizes[i].height, cellHeight);
        }
        float x = cellX;
        float y = cellY;
        if (spec.align == Align::Center) {
            x += (cellWidth - width) * 0.5f;
            y += (cellHeight - height) * 0.5f;
        } else if (spec.align == Align::End) {
            x += cellWidth - width;
            y += cellHeight - height;
        }
        out.push_back({x, y, width, height});
    }
    return out;
}

std::vector<Rect>
table(Rect const& area, std::vector<Track> const& columns, std::vector<Track> const& rows, TableSpec const& spec) {
    std::vector<Rect> out;
    if (columns.empty() || rows.empty()) {
        return out;
    }
    std::vector<float> const widths  = solveTracks(area.width, columns, spec.columnGap);
    std::vector<float> const heights = solveTracks(area.height, rows, spec.rowGap);

    out.reserve(widths.size() * heights.size());
    float y = area.y;
    for (std::size_t row = 0; row < heights.size(); ++row) {
        float x = area.x;
        for (std::size_t column = 0; column < widths.size(); ++column) {
            out.push_back({x, y, widths[column], heights[row]});
            x += widths[column] + spec.columnGap;
        }
        y += heights[row] + spec.rowGap;
    }
    return out;
}

float clampScroll(float offset, ScrollRange const& range) {
    float const maxOffset = std::max(range.content - range.viewport, 0.f);
    return std::clamp(offset, 0.f, maxOffset);
}

bool isVisible(ScrollRange const& range, float offset, float start, float extent) {
    return start + extent > offset && start < offset + range.viewport;
}

Thumb scrollThumb(ScrollRange const& range, float offset, float track, float minLength) {
    if (track <= 0.f) {
        return {};
    }
    if (range.content <= range.viewport) {
        return {0.f, track};
    }
    float const fraction  = std::clamp(range.viewport / range.content, 0.f, 1.f);
    float const length    = std::clamp(track * fraction, std::min(minLength, track), track);
    float const maxOffset = std::max(range.content - range.viewport, 0.f);
    float const start     = (track - length) * (maxOffset > 0.f ? std::clamp(offset / maxOffset, 0.f, 1.f) : 0.f);
    return {start, length};
}

} // namespace bul::layout

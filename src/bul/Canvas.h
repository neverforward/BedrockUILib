#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>

#include "bul/Color.h"
#include "bul/Geometry.h"

namespace bul {

enum class Align { Left, Center, Right };

/// Drawing surface the widget tree paints into.
///
/// The library's model code only knows this interface: the game implements it
/// over `MinecraftUIRenderContext`, tests implement it with a recorder, and the
/// offline preview could implement it with GDI. Nothing here depends on the SDK.
class Canvas {
public:
    Canvas()                         = default;
    Canvas(Canvas const&)            = delete;
    Canvas& operator=(Canvas const&) = delete;
    virtual ~Canvas()                = default;

    virtual void fill(Rect const& rect, Rgba color, float alpha = 1.f)        = 0;
    virtual void outline(Rect const& rect, Rgba color, float thickness = 1.f) = 0;
    virtual void
    text(std::string_view text, Point topLeft, float boxWidth, Rgba color, float size, Align align = Align::Left) = 0;

    /// Advance width of `text` at `size`, in GUI units.
    [[nodiscard]] virtual float textWidth(std::string_view text, float size) const = 0;
    [[nodiscard]] virtual float lineHeight(float size) const                       = 0;

    /// Confines the drawing to `rect` until `popClip`; clips nest by
    /// intersection. The default does nothing, so a canvas without scissor
    /// support still draws (a scroll container then paints whole children).
    virtual void pushClip(Rect const&) {}
    virtual void popClip() {}

    /// Submits everything recorded since the previous flush.
    virtual void flush() = 0;

    /// A rounded rectangle, from a `borderRadius` style. The corners
    /// are approximated one pixel row at a time, which is the resolution the game
    /// draws at anyway, so no engine support is needed.
    void fillRounded(Rect rect, float radius, Rgba color, float alpha = 1.f) {
        float const limit = std::min({radius, rect.width / 2.f, rect.height / 2.f});
        if (limit <= 0.f) {
            fill(rect, color, alpha);
            return;
        }
        int const   rows      = std::max(1, static_cast<int>(limit));
        float const rowHeight = limit / static_cast<float>(rows);
        for (int i = 0; i < rows; ++i) {
            float const rowMiddle = (static_cast<float>(i) + 0.5f) * rowHeight;
            float const dy        = limit - rowMiddle;
            float const inset     = limit - std::sqrt(std::max(limit * limit - dy * dy, 0.f));
            float const y         = static_cast<float>(i) * rowHeight;
            fill({rect.x + inset, rect.y + y, rect.width - 2.f * inset, rowHeight}, color, alpha);
            fill({rect.x + inset, rect.bottom() - y - rowHeight, rect.width - 2.f * inset, rowHeight}, color, alpha);
        }
        fill({rect.x, rect.y + limit, rect.width, rect.height - 2.f * limit}, color, alpha);
    }

    /// The border of a rounded rectangle, drawn by sampling the four corner arcs
    /// and drawing the straight sides as strips.
    void outlineRounded(Rect rect, float radius, Rgba color, float thickness = 1.f) {
        float const limit = std::min({radius, rect.width / 2.f, rect.height / 2.f});
        if (limit <= 0.f || thickness <= 0.f) {
            outline(rect, color, thickness);
            return;
        }
        fill({rect.x + limit, rect.y, rect.width - 2.f * limit, thickness}, color);
        fill({rect.x + limit, rect.bottom() - thickness, rect.width - 2.f * limit, thickness}, color);
        fill({rect.x, rect.y + limit, thickness, rect.height - 2.f * limit}, color);
        fill({rect.right() - thickness, rect.y + limit, thickness, rect.height - 2.f * limit}, color);

        // The stroke sits inside the box: sampling the centre line at
        // `limit - thickness / 2` puts the outer edge of the arc on the same
        // outline as the straight sides, instead of poking half a pixel out.
        float const   arcRadius = std::max(limit - thickness / 2.f, 0.f);
        int const     samples   = std::max(8, static_cast<int>(limit * 4.f));
        float const   half      = thickness / 2.f;
        for (int i = 0; i <= samples; ++i) {
            float const angle = (static_cast<float>(i) / static_cast<float>(samples)) * 1.5707963f;
            float const dx    = arcRadius * std::cos(angle);
            float const dy    = arcRadius * std::sin(angle);
            // top-left, top-right, bottom-right, bottom-left corner arcs.
            fill({rect.x + limit - dx - half, rect.y + limit - dy - half, thickness, thickness}, color);
            fill({rect.right() - limit + dx - half, rect.y + limit - dy - half, thickness, thickness}, color);
            fill({rect.right() - limit + dx - half, rect.bottom() - limit + dy - half, thickness, thickness}, color);
            fill({rect.x + limit - dx - half, rect.bottom() - limit + dy - half, thickness, thickness}, color);
        }
    }
};

} // namespace bul

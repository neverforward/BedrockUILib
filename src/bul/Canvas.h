#pragma once

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

    /// Submits everything recorded since the previous flush.
    virtual void flush() = 0;
};

} // namespace bul

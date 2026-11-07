#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "bul/Color.h"
#include "bul/Geometry.h"

#include "mc/client/gui/TextAlignment.h"

class Font;
class MinecraftUIRenderContext;

namespace bul::gfx {

/// Immediate-mode wrapper over the engine's UI render context.
///
/// The game batches its UI in a fixed order: rectangles, then `flushImages`,
/// then text, then `flushText`. A Painter keeps that order for the caller:
/// record fills and text in any order, then call flush() once per z band.
/// Everything recorded after a flush() is drawn above what came before.
///
/// Coordinates are GUI units. A Painter is only valid inside a UI render
/// callback and must not be kept across frames.
class Painter {
public:
    explicit Painter(MinecraftUIRenderContext& context);
    ~Painter();

    Painter(Painter const&)            = delete;
    Painter& operator=(Painter const&) = delete;

    void fillRect(Rect const& rect, Rgba color);
    void outline(Rect const& rect, Rgba color, float thickness = 1.f);

    /// Draws one line. `width` of 0 means "as wide as the text"; with an
    /// alignment other than Left the box is aligned inside that width.
    void text(
        std::string_view  text,
        Point             topLeft,
        Rgba              color,
        float             scale = 1.f,
        ui::TextAlignment align = ui::TextAlignment::Left,
        float             width = 0.f
    );

    void flush();

    /// Size of the UI screen in GUI units.
    [[nodiscard]] Rect screen() const;

    [[nodiscard]] float textWidth(std::string_view text, float scale = 1.f) const;
    [[nodiscard]] float lineHeight(float scale = 1.f) const;

private:
    MinecraftUIRenderContext& mContext;
    // The engine hands out its font inside a FontHandle (a shared_ptr<Font> plus
    // an id); keep the owner alive so the reference stays valid for this painter.
    std::shared_ptr<Font> mFontOwner;
    Font&                 mFont;
    bool                  mHasFills{};
    bool                  mHasText{};
};

} // namespace bul::gfx

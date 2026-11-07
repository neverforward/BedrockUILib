#pragma once

#include <string_view>

#include "bul/Canvas.h"

#include "mc/client/renderer/screen/MinecraftUIRenderContext.h"

#include "bul/gfx/Painter.h"

namespace bul::gfx {

/// `bul::Canvas` over the engine's UI render context. This is the only place
/// where the widget tree meets the game's drawing API.
class GameCanvas final : public Canvas {
public:
    explicit GameCanvas(MinecraftUIRenderContext& context);

    void fill(Rect const& rect, Rgba color, float alpha) override;
    void outline(Rect const& rect, Rgba color, float thickness) override;
    void text(std::string_view text, Point topLeft, float boxWidth, Rgba color, float size, Align align) override;

    [[nodiscard]] float textWidth(std::string_view text, float size) const override;
    [[nodiscard]] float lineHeight(float size) const override;
    void                flush() override;

    /// The whole UI surface in GUI units.
    [[nodiscard]] Rect screen() const;

private:
    Painter mPainter;
};

} // namespace bul::gfx

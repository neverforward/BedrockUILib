#include "bul/gfx/GameCanvas.h"

#include "mc/client/gui/GuiData.h"

#include "bul/gfx/Painter.h"

namespace bul::gfx {

namespace {

[[nodiscard]] ::ui::TextAlignment toTextAlignment(Align align) {
    switch (align) {
    case Align::Center:
        return ::ui::TextAlignment::Center;
    case Align::Right:
        return ::ui::TextAlignment::Right;
    default:
        return ::ui::TextAlignment::Left;
    }
}

} // namespace

GameCanvas::GameCanvas(MinecraftUIRenderContext& context) : mPainter(context) {}

void GameCanvas::fill(Rect const& rect, Rgba color, float alpha) { mPainter.fillRect(rect, color.withAlpha(alpha)); }

void GameCanvas::outline(Rect const& rect, Rgba color, float thickness) { mPainter.outline(rect, color, thickness); }

void GameCanvas::text(std::string_view value, Point topLeft, float boxWidth, Rgba color, float size, Align align) {
    mPainter.text(value, topLeft, color, size, toTextAlignment(align), boxWidth);
}

float GameCanvas::textWidth(std::string_view value, float size) const { return mPainter.textWidth(value, size); }

float GameCanvas::lineHeight(float size) const { return mPainter.lineHeight(size); }

void GameCanvas::flush() { mPainter.flush(); }

Rect GameCanvas::screen() const { return mPainter.screen(); }

} // namespace bul::gfx

#include "bul/gfx/Painter.h"

#include <optional>
#include <utility>

#include "mc/client/game/IClientInstance.h"
#include "mc/client/gui/CaretMeasureData.h"
#include "mc/client/gui/Font.h"
#include "mc/client/gui/FontHandle.h"
#include "mc/client/gui/GuiData.h"
#include "mc/client/gui/TextMeasureData.h"
#include "mc/client/renderer/screen/MinecraftUIRenderContext.h"
#include "mc/deps/core/math/Color.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/deps/input/RectangleArea.h"

namespace bul::gfx {

namespace {

constexpr float kLineHeight = 9.f;

[[nodiscard]] RectangleArea toArea(Rect const& rect) {
    return RectangleArea{rect.x, rect.right(), rect.y, rect.bottom()};
}

[[nodiscard]] mce::Color toMce(Rgba const& color) { return mce::Color{color.r, color.g, color.b, color.a}; }

/// Material the vanilla UI uses to flush solid coloured quads.
[[nodiscard]] HashedString const& fillMaterial() {
    static HashedString const material{"ui_fillColor"};
    return material;
}

[[nodiscard]] mce::Color const& white() {
    static mce::Color const color{1.f, 1.f, 1.f, 1.f};
    return color;
}

} // namespace

Painter::Painter(MinecraftUIRenderContext& context)
: mContext(context),
  mFontOwner(mContext.mClient.getFontHandle().mDefaultFont),
  mFont(*mFontOwner) {}

Painter::~Painter() { flush(); }

void Painter::fillRect(Rect const& rect, Rgba color) {
    if (rect.empty() || color.a <= 0.f) {
        return;
    }
    mContext.fillRectangle(toArea(rect), toMce(color), color.a);
    mHasFills = true;
}

void Painter::outline(Rect const& rect, Rgba color, float thickness) {
    if (rect.empty() || color.a <= 0.f || thickness <= 0.f) {
        return;
    }
    fillRect({rect.x, rect.y, rect.width, thickness}, color);
    fillRect({rect.x, rect.bottom() - thickness, rect.width, thickness}, color);
    fillRect({rect.x, rect.y + thickness, thickness, rect.height - 2.f * thickness}, color);
    fillRect({rect.right() - thickness, rect.y + thickness, thickness, rect.height - 2.f * thickness}, color);
}

void Painter::text(
    std::string_view  text,
    Point             topLeft,
    Rgba              color,
    float             scale,
    ui::TextAlignment align,
    float             width
) {
    if (text.empty() || color.a <= 0.f) {
        return;
    }
    float boxWidth = width > 0.f ? width : textWidth(text, scale) + 2.f;
    float x0       = topLeft.x;
    if (width <= 0.f && align == ui::TextAlignment::Center) {
        x0 -= boxWidth / 2.f;
    }
    if (width <= 0.f && align == ui::TextAlignment::Right) {
        x0 -= boxWidth;
    }
    RectangleArea const area{x0, x0 + boxWidth, topLeft.y, topLeft.y + lineHeight(scale) + 2.f};
    // Shadow is on for light text, matching the vanilla UI convention.
    TextMeasureData const  measure{scale, 0.f, color.r > 0.9f && color.g > 0.9f, false, false, align};
    CaretMeasureData const caret{-1, false};
    mContext.drawText(mFont, area, std::string{text}, toMce(color), color.a, align, measure, caret);
    mHasText = true;
}

void Painter::flush() {
    if (mHasFills) {
        mContext.flushImages(white(), 1.f, fillMaterial());
        mHasFills = false;
    }
    if (mHasText) {
        mContext.flushText(0.f, std::nullopt);
        mHasText = false;
    }
}

Rect Painter::screen() const {
    auto gui = mContext.mClient.getGuiData();
    auto sz  = gui->mScreenSizeData->clientUIScreenSize;
    return {0.f, 0.f, sz->x, sz->y};
}

float Painter::textWidth(std::string_view text, float scale) const {
    return static_cast<float>(mContext.getLineLength(mFont, std::string{text}, scale, false));
}

float Painter::lineHeight(float scale) const { return kLineHeight * scale; }

} // namespace bul::gfx

#pragma once

#include <algorithm>
#include <string_view>

#include "bul/Canvas.h"

namespace bul {

/// A canvas that multiplies every alpha by `opacity`, the way an `opacity` style
/// affects a whole subtree: an element paints through this wrapper, its children
/// receive it as their canvas, and the factor composes.
class AlphaCanvas final : public Canvas {
public:
    AlphaCanvas(Canvas& inner, float opacity)
    : mInner(inner),
      mOpacity(std::clamp(opacity, 0.f, 1.f)) {}

    void fill(Rect const& rect, Rgba color, float alpha = 1.f) override {
        mInner.fill(rect, color, alpha * mOpacity);
    }

    void outline(Rect const& rect, Rgba color, float thickness = 1.f) override {
        mInner.outline(rect, color.withAlpha(color.a * mOpacity), thickness);
    }

    void text(std::string_view text, Point topLeft, float boxWidth, Rgba color, float size, Align align = Align::Left)
        override {
        mInner.text(text, topLeft, boxWidth, color.withAlpha(color.a * mOpacity), size, align);
    }

    [[nodiscard]] float textWidth(std::string_view text, float size) const override {
        return mInner.textWidth(text, size);
    }

    [[nodiscard]] float lineHeight(float size) const override { return mInner.lineHeight(size); }

    void pushClip(Rect const& rect) override { mInner.pushClip(rect); }
    void popClip() override { mInner.popClip(); }
    void flush() override { mInner.flush(); }

private:
    Canvas& mInner;
    float   mOpacity{1.f};
};

} // namespace bul

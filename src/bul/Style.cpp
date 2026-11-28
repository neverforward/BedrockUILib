#include "bul/Style.h"

#include "bul/Theme.h"

namespace bul {

namespace {

/// Copies `from` over `to` when the style set it.
template <class T>
void take(std::optional<T>& to, std::optional<T> const& from) noexcept {
    if (from) {
        to = *from;
    }
}

} // namespace

Style& Style::overlaidBy(Style const& over) noexcept {
    take(color, over.color);
    take(backgroundColor, over.backgroundColor);
    take(borderColor, over.borderColor);
    take(borderWidth, over.borderWidth);
    take(padding, over.padding);
    take(gap, over.gap);
    take(fontSize, over.fontSize);
    take(accentColor, over.accentColor);
    take(margin, over.margin);
    take(width, over.width);
    take(height, over.height);
    take(minWidth, over.minWidth);
    take(minHeight, over.minHeight);
    take(maxWidth, over.maxWidth);
    take(maxHeight, over.maxHeight);
    take(textAlign, over.textAlign);
    take(lineHeight, over.lineHeight);
    take(visibility, over.visibility);
    take(textTransform, over.textTransform);
    take(whiteSpace, over.whiteSpace);
    take(verticalAlign, over.verticalAlign);
    take(display, over.display);
    take(borderRadius, over.borderRadius);
    take(opacity, over.opacity);
    take(letterSpacing, over.letterSpacing);
    take(fontWeight, over.fontWeight);
    take(fontStyle, over.fontStyle);
    take(obfuscated, over.obfuscated);
    return *this;
}

bool Style::empty() const noexcept {
    return !color && !backgroundColor && !borderColor && !borderWidth && !padding && !gap && !fontSize
        && !accentColor && !margin && !width && !height && !minWidth && !minHeight && !maxWidth && !maxHeight
        && !textAlign && !lineHeight && !visibility && !textTransform && !whiteSpace && !verticalAlign
        && !display && !borderRadius && !opacity && !letterSpacing && !fontWeight && !fontStyle && !obfuscated;
}

Theme applyStyle(Theme const& base, Style const& style) {
    Theme resolved = base;
    // Relative units resolve against the font this style starts from.
    float const baseFont = resolved.fontSize * resolved.fontScale;

    // style layer: each property sets both the generic slot and the palette token a
    // component would otherwise read, so one `color` reaches every text-drawing
    // component and one `background-color` reaches every surface.
    if (style.color) {
        resolved.color = style.color;
        resolved.text  = *style.color;
    }
    if (style.backgroundColor) {
        resolved.background = style.backgroundColor;
        resolved.panel      = *style.backgroundColor;
    }
    if (style.borderColor) {
        resolved.border      = style.borderColor;
        resolved.panelBorder = *style.borderColor;
        resolved.inputBorder = *style.borderColor;
    }
    if (style.borderWidth) {
        resolved.borderWidth = style.borderWidth->resolve(baseFont);
    }
    if (style.padding) {
        resolved.padding = style.padding->resolve(baseFont);
    }
    if (style.gap) {
        resolved.gap = style.gap->resolve(baseFont);
    }
    if (style.fontSize) {
        resolved.fontScale = style.fontSize->resolve(baseFont) / resolved.fontSize;
    }
    if (style.accentColor) {
        resolved.accent = *style.accentColor;
    }
    if (style.textAlign) {
        resolved.textAlign = style.textAlign;
    }
    if (style.lineHeight) {
        resolved.lineHeight = style.lineHeight->resolve(baseFont);
    }
    if (style.visibility) {
        resolved.visible = *style.visibility == Visibility::Visible;
    }
    if (style.textTransform) {
        resolved.textTransform = style.textTransform;
    }
    if (style.whiteSpace) {
        resolved.whiteSpace = style.whiteSpace;
    }
    if (style.verticalAlign) {
        resolved.verticalAlign = style.verticalAlign;
    }
    if (style.display) {
        resolved.displayed = *style.display == Display::Block;
    }
    if (style.borderRadius) {
        resolved.borderRadius = style.borderRadius->resolve(baseFont);
    }
    if (style.opacity) {
        resolved.opacity = *style.opacity < 0.f ? 0.f : (*style.opacity > 1.f ? 1.f : *style.opacity);
    }
    if (style.letterSpacing) {
        resolved.letterSpacing = style.letterSpacing->resolve(baseFont);
    }
    if (style.fontWeight) {
        resolved.bold = *style.fontWeight == FontWeight::Bold;
    }
    if (style.fontStyle) {
        resolved.italic = *style.fontStyle == FontStyle::Italic;
    }
    if (style.obfuscated) {
        resolved.obfuscated = *style.obfuscated;
    }
    return resolved;
}

} // namespace bul

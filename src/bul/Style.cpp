#include "bul/Style.h"

#include "bul/Theme.h"

namespace bul {

Style& Style::overlaidBy(Style const& over) noexcept {
    if (over.text) {
        text = *over.text;
    }
    if (over.textMuted) {
        textMuted = *over.textMuted;
    }
    if (over.panel) {
        panel = *over.panel;
    }
    if (over.panelBorder) {
        panelBorder = *over.panelBorder;
    }
    if (over.accent) {
        accent = *over.accent;
    }
    if (over.primary) {
        primary = *over.primary;
    }
    if (over.primaryHover) {
        primaryHover = *over.primaryHover;
    }
    if (over.primaryText) {
        primaryText = *over.primaryText;
    }
    if (over.button) {
        button = *over.button;
    }
    if (over.buttonHover) {
        buttonHover = *over.buttonHover;
    }
    if (over.secondary) {
        secondary = *over.secondary;
    }
    if (over.secondaryHover) {
        secondaryHover = *over.secondaryHover;
    }
    if (over.destructive) {
        destructive = *over.destructive;
    }
    if (over.destructiveHover) {
        destructiveHover = *over.destructiveHover;
    }
    if (over.destructiveText) {
        destructiveText = *over.destructiveText;
    }
    if (over.input) {
        input = *over.input;
    }
    if (over.inputHover) {
        inputHover = *over.inputHover;
    }
    if (over.inputBorder) {
        inputBorder = *over.inputBorder;
    }
    if (over.invalid) {
        invalid = *over.invalid;
    }
    return *this;
}

bool Style::empty() const noexcept {
    return !text && !textMuted && !panel && !panelBorder && !accent && !primary && !primaryHover && !primaryText
        && !button && !buttonHover && !secondary && !secondaryHover && !destructive && !destructiveHover
        && !destructiveText && !input && !inputHover && !inputBorder && !invalid;
}

Theme applyStyle(Theme const& base, Style const& style) {
    Theme resolved = base;
    if (style.text) {
        resolved.text = *style.text;
    }
    if (style.textMuted) {
        resolved.textMuted = *style.textMuted;
    }
    if (style.panel) {
        resolved.panel = *style.panel;
    }
    if (style.panelBorder) {
        resolved.panelBorder = *style.panelBorder;
    }
    if (style.accent) {
        resolved.accent = *style.accent;
    }
    if (style.primary) {
        resolved.primary = *style.primary;
    }
    if (style.primaryHover) {
        resolved.primaryHover = *style.primaryHover;
    }
    if (style.primaryText) {
        resolved.primaryText = *style.primaryText;
    }
    if (style.button) {
        resolved.button = *style.button;
    }
    if (style.buttonHover) {
        resolved.buttonHover = *style.buttonHover;
    }
    if (style.secondary) {
        resolved.secondary = *style.secondary;
    }
    if (style.secondaryHover) {
        resolved.secondaryHover = *style.secondaryHover;
    }
    if (style.destructive) {
        resolved.destructive = *style.destructive;
    }
    if (style.destructiveHover) {
        resolved.destructiveHover = *style.destructiveHover;
    }
    if (style.destructiveText) {
        resolved.destructiveText = *style.destructiveText;
    }
    if (style.input) {
        resolved.input = *style.input;
    }
    if (style.inputHover) {
        resolved.inputHover = *style.inputHover;
    }
    if (style.inputBorder) {
        resolved.inputBorder = *style.inputBorder;
    }
    if (style.invalid) {
        resolved.invalid = *style.invalid;
    }
    return resolved;
}

} // namespace bul

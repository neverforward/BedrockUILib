# Theming

Every colour and metric is a `Theme` token with a neutral placeholder default
(`bul::Theme::neutral()`). The input-specific tokens are `input`, `inputHover`,
`inputBorder` and `invalid`; the rest are shared with the buttons and the panel.

```cpp
bul::Theme theme = bul::Theme::neutral();
theme.accent = bul::Rgba::fromRgb(0x59A9FF);
theme.input  = bul::Rgba::fromRgb(0x101319);
bul::setTheme(theme);
```

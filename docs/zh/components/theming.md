# 主题

每个颜色、每个度量都是 `Theme` token，默认值是中性的占位值（`bul::Theme::neutral()`）。Input 专属 token 是 `input`、`inputHover`、`inputBorder`、`invalid`；其余和按钮、面板共用。

```cpp
bul::Theme theme = bul::Theme::neutral();
theme.accent = bul::Rgba::fromRgb(0x59A9FF);
theme.input  = bul::Rgba::fromRgb(0x101319);
bul::setTheme(theme);
```

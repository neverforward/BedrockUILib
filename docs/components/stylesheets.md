# Stylesheets

A `StyleSheet` is the kit's version of a CSS file: rules select components and
declare colours. The theme stays the base palette, the sheet colours whole
components, and a widget's inline `Style` always wins.

```cpp
bul::StyleSheet sheet;
sheet.rule({.type = "Checkbox"}, {.accent = bul::Rgba::fromRgb(0x2DD4BF)});
sheet.rule({.className = "danger"}, {.primary = bul::Rgba::fromRgb(0xE5484D)});
sheet.rule({.id = "submit"}, {.primary = bul::Rgba::fromRgb(0x46A758)});
bul::setStyleSheet(std::move(sheet));
```

- A `Selector` has `type` (the element: `Button`, `Label`, `Input`, ...),
  `className` and `id`; an unset part matches anything, like CSS.
- Specificity is CSS's: id 100, class 10, element 1; a tie goes to the rule
  declared later.
- A widget resolves its colours as: the inherited theme, then the rules that
  match it, then its own inline `style()`. A container passes its resolved theme
  down, so a rule on a container reaches its whole subtree.
- Elements are named by `Widget::typeName()`; `className(...)` and `id(...)` set
  the other two selector parts.

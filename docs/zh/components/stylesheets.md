# 样式表

`StyleSheet` 相当于套件版 CSS 文件：规则选中组件并声明颜色。主题仍是基础调色板，样式表给整个组件上色，控件内联的 `Style` 永远优先。

```cpp
bul::StyleSheet sheet;
sheet.rule({.type = "Checkbox"}, {.accent = bul::Rgba::fromRgb(0x2DD4BF)});
sheet.rule({.className = "danger"}, {.primary = bul::Rgba::fromRgb(0xE5484D)});
sheet.rule({.id = "submit"}, {.primary = bul::Rgba::fromRgb(0x46A758)});
bul::setStyleSheet(std::move(sheet));
```

- `Selector` 有 `type`（元素：`Button`、`Label`、`Input`……）、`className`、`id`；没设的部分匹配任意，和 CSS 一样。
- 优先级按 CSS：id 100、class 10、元素 1；同分后声明的赢。
- 控件解析颜色的顺序：继承的主题 -> 匹配的规则 -> 自己的内联 `style()`。容器把解析后的主题往下传，所以容器上的规则能到整个子树。
- 元素名来自 `Widget::typeName()`；`className(...)` 和 `id(...)` 设另外两个选择器部分。

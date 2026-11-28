# Style

视觉来自两个方面：

- `Theme` 是文档本身，是整棵树出发的调色板（`setTheme`）；
- `Style` 是单个控件的覆盖，相当于套件里的 `style=""`。它只替换自己设置的属性，其余继承自主题。

```cpp
auto group = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
group->style({.color = bul::Rgba::fromRgb(0x8CC4FF), .gap = 4_px});
group->add(std::make_unique<bul::Label>("继承文字颜色")); // 用 style 的颜色绘制
```

## 属性

| 属性 | 样式名 | 含义 |
| --- | --- | --- |
| `color` | `color` | 文字颜色 |
| `backgroundColor` | `background-color` | 控件自己的表面色；悬停色写在 `:hover` 规则里 |
| `borderColor` | `border-color` | 边框颜色，面板和输入框共用 |
| `borderWidth` | `border-width` | 边框宽度 |
| `padding` | `padding` | 内边距（`Padding`，四边各一个长度） |
| `margin` | `margin` | 外边距，布局为控件保留的空间 |
| `gap` | `gap` | 容器子控件之间的间距 |
| `width` / `height` | `width` / `height` | 固定尺寸 |
| `minWidth` / `minHeight` / `maxWidth` / `maxHeight` | `min-*` / `max-*` | 尺寸界限 |
| `fontSize` | `font-size` | 文字高度，`Length` |
| `lineHeight` | `line-height` | 行前进距离，`Length` |
| `textAlign` | `text-align` | `Align::Left` / `Center` / `Right` |
| `verticalAlign` | `vertical-align` | `TextVertical::Top` / `Middle` / `Bottom`，文本块位置 |
| `textTransform` | `text-transform` | `TextTransform::Uppercase` / `Lowercase` / `Capitalize` |
| `whiteSpace` | `white-space` | `WhiteSpace::Normal` 换行，`NoWrap` / `Pre` 不换 |
| `visibility` | `visibility` | `Hidden` 不绘制但保留占位 |
| `accentColor` | `accent-color` | 选中态填充：勾选框、开关、焦点环 |
| `borderRadius` | `border-radius` | 圆角表面与边框 |
| `opacity` | `opacity` | 控件及其绘制内容的整体透明度 |
| `display` | `display` | `Display::None` 从布局中移除 |
| `letterSpacing` | `letter-spacing` | 字间额外间距 |
| `fontWeight` | `font-weight` | `Bold` 把文本画成 `§l...§r` |
| `fontStyle` | `font-style` | `Italic` 把文本画成 `§o...§r` |
| `obfuscated` | （套件扩展） | `true` 把文本画成 `§k...§r`，即随机乱码 |

容器设置了 `backgroundColor` / `borderColor` 时会在子控件后面画出这个盒子，和 `div` 一样；目前支持的是 `Stack` 和 `Grid`。

悬停不是一个属性，而是规则里的 `:hover` 伪类，和样式表一样：

```cpp
bul::setStyleSheet(bul::StyleSheet{
    {"Button", {.backgroundColor = ..., .color = ...}},
    {"Button:hover", {.backgroundColor = ...}},
});
```

## 调色板

按角色取色的颜色在 `Theme` 上，不在 `Style` 里，用 `setTheme` 改，或者在样式表里设映射到它的样式属性，见 [样式表](/zh/components/stylesheets)。

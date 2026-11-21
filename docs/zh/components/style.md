# 样式

颜色分两层：

- `Theme` 是文档：整棵树的起始调色板（`setTheme`）；
- `Style` 是单个控件自己的覆盖：只替换它设置的 token，其余从继承的主题来。

```cpp
auto group = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
group->style({.text = bul::Rgba::fromRgb(0x8CC4FF), .accent = bul::Rgba::fromRgb(0xFFB454)});
group->add(std::make_unique<bul::Label>("Inherited text colour")); // 用 style 颜色绘制
```

容器先解析自己的样式，再把结果传给子控件，所以 `Stack` 上的样式会染色整个子树；子控件设置了同一个 token 则子控件赢。spec 树里同样的事放在 `Common::style` 上。

| Token | 用于 |
| --- | --- |
| `text` / `textMuted` | 文本和说明文字 |
| `accent` / `invalid` | 焦点态和错误态 |
| `primary` / `primaryHover` / `primaryText` | 默认按钮 |
| `secondary` / `secondaryHover` | 次要按钮 |
| `destructive` / `destructiveHover` / `destructiveText` | 危险按钮 |
| `button` / `buttonHover` | 中性表面 |
| `input` / `inputHover` / `inputBorder` | 文本框和 select 触发器 |
| `panel` / `panelBorder` | 面板和弹出层 |

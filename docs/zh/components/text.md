# 文本

`Label` 是文本块，能换行、缩放、对齐、限高，测量和绘制共用同一套布局。

```cpp
auto label = std::make_unique<bul::Label>("Long enough to need a second line.");
label->wrap(true);          // 在空格处断行，CJK 字间也可断
label->maxWidth(160.f);     // 保持可读的栏宽
label->lineSpacing(2.f);
label->maxLines(2);         // 第二行以 "..." 结尾
label->align(bul::Align::Center);
label->verticalAlign(bul::VAlign::Center);
```

| 成员 | 含义 |
| --- | --- |
| `text(t)` / `text()` | 字符串。显式 `\n` 总是另起一行。 |
| `size(s)` / `size()` | 相对主题基础字号的缩放（1 = 正文）。 |
| `align(a)` / `align()` | 每行水平对齐：`Left`、`Center`、`Right`。 |
| `verticalAlign(v)` / `verticalAlign()` | 文本块在边界内的纵向位置：`Top`、`Center`、`Bottom`。 |
| `wrap(true)` / `wrap()` | 按可用宽度自动换行。空格是断点；CJK 没有空格，任意两字之间都可断。 |
| `maxWidth(w)` / `maxWidth()` | 限制文本占用的宽度，即使父控件给的更多；控件对外报 `w` 作为宽度，栏宽保持稳定。 |
| `lineSpacing(v)` / `lineSpacing()` | 行距附加值，按主题缩放 1 时的单位。 |
| `maxLines(n)` / `maxLines()` | 截断到 `n` 行，最后一行省略。 |
| `color(c)` / `color()` | 显式文字颜色（`Rgba`）；设置后覆盖 `muted()`。 |
| `elide(b)` / `elide()` | 单行放不下时在末尾截断加 `...`（默认开）。 |
| `muted()` | 用 `Theme::textMuted` 绘制，即说明文字颜色。 |

换行是贪心的：每行收码点到下一个收不下为止，再回退到上一个断点。比栏宽还宽的单个字形允许溢出，不消失。文本用 Input 同一张逐码点步进表测量，所以 CJK 断在哪，实际就画在哪。

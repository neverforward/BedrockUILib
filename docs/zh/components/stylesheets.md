# 样式表

`StyleSheet` 就是套件的样式表：规则选中组件并声明属性。

```cpp
using namespace bul::literals;

bul::StyleSheet sheet;
sheet.className("danger").set({.backgroundColor = bul::Rgba::fromRgb(0xE5484D)});
sheet.type("Label").set({.fontSize = 14_px});
sheet.type("Stack").child().type("Label").set({.color = ...});
sheet.type("Input").pseudo("disabled").set({.color = ...});
bul::setStyleSheet(std::move(sheet));
```

## 选择器

| 写法 | 选择器 | 匹配 |
| --- | --- | --- |
| `.type("Button")`、`.type<bul::Button>()` | `Button` | 元素类型 |
| `.className("danger")` | `.danger` | 类名 |
| `.id("submit")` | `#submit` | id |
| `.any()` | `*` | 全部 |
| `.type("A").type("B")` | `A B` | 上层有 `A` 的 `B` |
| `.type("A").child().type("B")` | `A > B` | 父级是 `A` 的 `B` |
| `.type("A").adjacent().type("B")` | `A + B` | 紧跟 `A` 的 `B` |
| `.type("A").sibling().type("B")` | `A ~ B` | `A` 之后的 `B` |
| `.attr("kind")` | `[kind]` | 属性存在 |
| `.attr("kind", "text")` | `[kind=text]` | 属性等于 |
| `.attr("tags", "hot", Word)` | `[tags~=hot]` | 空格分隔列表中的一项 |
| `.attr("lang", "en", PrefixDash)` | `[lang\|=en]` | `en` 或 `en-...` |
| `.attr("src", "https", StartsWith)` | `[src^=https]` | 以它开头 |
| `.attr("src", ".pdf", EndsWith)` | `[src$=.pdf]` | 以它结尾 |
| `.attr("src", "bul", Contains)` | `[src*=bul]` | 包含 |

属性运算符： `Attribute::Match`：`Exists`、`Equals`、`Word`、`PrefixDash`、`StartsWith`、`EndsWith`、`Contains`。

## 伪类

伪类匹配方式为 `pseudo(name, argument)`：

```cpp
sheet.type("Input").pseudo("disabled").set({...});           // :disabled
sheet.type("Label").pseudo("nth-child", "2").set({...});     // :nth-child(2)
sheet.type("Label").pseudo("not", ".quiet").set({...});      // :not(.quiet)
sheet.type("Stack").pseudo("has", "Label").set({...});       // :has(Label)
```

| 分组 | 名字 |
| --- | --- |
| 状态 | `hover`、`focus`、`active`、`disabled`、`enabled`、`checked`、`invalid`、`valid`、`required`、`optional`、`read-only`、`read-write` |
| 位置 | `first-child`、`last-child`、`only-child`、`nth-child(n)`、`nth-last-child(n)`、`first-of-type`、`last-of-type`、`only-of-type`、`nth-of-type(n)`、`nth-last-of-type(n)` |
| 结构 | `root`、`empty` |
| 逻辑 | `not(selector)`、`is(selector)`、`where(selector)`、`has(selector)` |

`nth-child` 接受 `2`、`odd`、`even` 或 `an+b`。

## 多选择器

`rule(...)` 接受逗号分隔的列表，每一段各成一条规则：

```cpp
sheet.rule("Button, Label.note", {.color = ...});
```

## 长度单位

数值属性（`fontSize`、`gap`、`borderWidth`）接受 `Length`：一个数字加一个单位。裸数字就是 GUI 单位，所以 `4.f` 和 `4_px` 等价。`padding` 接受 `Padding`，四边各一个长度：`Padding::all(4_px)` 或 `Padding::symmetric(1_em, 4_px)`；普通 `Insets` 可以隐式转换。

| 单位 | 写法 | 解析为 |
| --- | --- | --- |
| px | `14_px` | GUI 单位，1:1 |
| em | `1.5_em` | 主题基础字号高度的 1.5 倍 |
| ex | `2_ex` | 每次半个字号高度 |
| % | `50_pct` | 基础字号高度的百分之几 |
| in | `0.5_in` | 1 英寸 = 96 GUI 单位 |
| cm / mm | `2.54_cm`、`25.4_mm` | 按 96/英寸换算 |
| pt / pc | `12_pt`、`1_pc` | 1pt = 96/72，1pc = 16 |

后缀带下划线是 clang 的要求（`14px` 无法调用字面量运算符）；`%` 是运算符，所以百分数写成 `_pct`。`using namespace bul::literals;` 就能全部引入。

绝对单位固定；`em`、`ex`、`%` 以主题基础字号（`Theme::fontSize`，默认 9 GUI 单位）乘 `fontScale` 为基准解析，因为解析样式时还没有可供参照的包含块。

## 解析顺序

控件解析顺序：继承的主题 > 匹配的规则 > 内联 `style()`。

# 网格

等宽等高的格子，行优先：从左到右排，排满换行。格子由 `layout::grid` 求解，行高均分，网格正好填满给它的矩形。

```cpp
auto grid = std::make_unique<bul::Grid>(3); // 三列
grid->gap(6.f);
grid->add(std::make_unique<bul::Button>("One"));
grid->add(std::make_unique<bul::Button>("Two"));
grid->add(std::make_unique<bul::Button>("Three"));
```

| 成员 | 含义 |
| --- | --- |
| `Grid(columns)` / `columns(n)` | 每行的格子数；最后一行可以不满。 |
| `add(child)` | 追加一个格子，行优先。 |
| `gap(value)` | 格子间距；`< 0`（默认）用 `Theme::gap`。 |
| `measureContent` | 最宽的子控件 × 列数、最高的 × 行数，加上间距。 |

实现上：测量给出两个方向上最大的那个格子；布局时把区域均分成 `columns` 个等宽格，再把每个子控件拉伸到它的格子。

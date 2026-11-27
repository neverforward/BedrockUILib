# 表格

格子放在固定和弹性轨道上。轨道就是 `layout::Track`：`{ .value = n }` 是固定尺寸，`{ .weight = w }` 是分摊固定轨道用剩的空间。布局来自 `layout::table`。

```cpp
auto table = std::make_unique<bul::Table>();
table->column({.value = 90.f});
table->column({.weight = 1.f});
table->row({.value = 9.f});
table->row({.weight = 1.f});
table->cell(0, 0, std::make_unique<bul::Label>("Name"));
table->cell(0, 1, std::make_unique<bul::Label>("Value"));
table->cell(1, 0, std::make_unique<bul::Label>("Gold"));
table->cell(1, 1, std::make_unique<bul::Label>("12"));
```

或者使用 Spec 树

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.table([](bul::Table& table) {
        table.column({.value = 90.f});
        table.column({.weight = 1.f});
        table.row({.value = 9.f});
        table.row({.weight = 1.f});
        table.cell(0, 0, std::make_unique<bul::Label>("Name"));
        table.cell(0, 1, std::make_unique<bul::Label>("Value"));
        table.cell(1, 0, std::make_unique<bul::Label>("Gold"));
        table.cell(1, 1, std::make_unique<bul::Label>("12"));
    });
});
```


| 成员 | 含义 |
| --- | --- |
| `column(track)` / `row(track)` | 追加一条轨道：`{.value}` 固定，`{.weight}` 弹性。 |
| `cell(row, column, child)` | 放一个控件；索引超过已声明的轨道时会补一条弹性轨道，所以只写 cell 也能建表。 |
| `gap(columnGap, rowGap)` | 轨道间距。 |
| `padding(all)` / `padding(horizontal, vertical)` | 整个网格相对边框的内缩，默认 0；非 0 会让首尾行比中间行高，所以让内容离开边框通常用 `cellPadding`。 |
| `cellPadding(all)` / `cellPadding(horizontal, vertical)` | 单元格内容相对格子的内边距，默认 4，避免文字压在分隔线上。 |
| `border(bool)` / `borderColor(color)` | 画外框和行列分隔线；默认开启，未指定颜色时用 `Theme::panelBorder`。 |
| `measureContent` | 固定轨道之和，加上每条弹性轨道里最大的那个格子。 |

每个格子都会拉伸到它的行、列求解出的矩形，格子里的控件就像普通子控件一样在该矩形里布局。固定的 `Track::value` 是**内容**尺寸，格子内边距会加在它外面：`{.value = 9.f}` 的行就是 9 单位文字加上内边距。
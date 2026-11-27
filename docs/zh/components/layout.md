# 布局

| 组件     | 用途                                                                                                                             |
| -------- | -------------------------------------------------------------------------------------------------------------------------------- |
| `Widget` | 基类节点：`measure`/`measureContent`、`layout`、`paint`、`hitTest`、尺寸约束，以及 `onClick`/`onFocus`/`onBlur`/`onHover` 回调。 |
| `Stack`  | 行或列。`add(child)` 保持测量尺寸；`add(child, 1.f)` 让子控件拿走其他子控件分完后剩下的空间。                                    |
| `Grid`   | 每行 N 个等宽格子，行优先（[网格](/zh/components/grid)）。                                                                       |
| `Table`  | 固定/弹性轨道上的单元格（[表格](/zh/components/table)）。                                                                        |
| `Label`  | 文本块：字号、对齐、自动换行、行距、`maxLines`、`maxWidth`。                                                                     |
| `Button` | `Type`（default/secondary/destructive/outline/link）和 `Size`（xs/sml/default/lg）；`disabled(true)` 变暗并忽略点击。            |
| `Badge`  | 小胶囊，用来标注另一个控件（`Default`、`Secondary`、`Outline`、`Destructive`）。                                                 |

`Stack` 用拿到的宽度测量子控件；列方向把子控件拉伸到自己的宽度，行方向保留各子控件的测量宽度，把剩余空间分给被拉伸的子控件。

```cpp
auto const rects = bul::layout::box(
    {0.f, 0.f, 200.f, 20.f},
    {bul::layout::Item{{60.f, 18.f}}, bul::layout::Item{{0.f, 18.f}, 1.f}},
    {.axis = bul::layout::Axis::Horizontal, .gap = 8.f}
); // 第一个保持 60，第二个拿走这一行剩下的
```

或者使用 Spec 树

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.row([&] {
        ui.button("Browse", [] {});
        ui.add(std::make_unique<bul::Input>(), 1.f); // 拉伸系数
    });
});
```

## 求解器

`bul/Layout.h` 把测量结果换算成矩形。里面的每条规则都是纯几何，不含控件、不含引擎类型，所以有单元测试；`Stack` 只是它的一个使用方，你自己的容器也可以直接用这些原语。

| 函数                                                              | 求解什么                                                                                                              |
| ----------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------- |
| `layout::box`                                                     | 一行或一列：主轴尺寸、`grow` 子项分剩余空间、`gap`、`padding`，以及交叉轴 `Align`（start / center / end / stretch）。 |
| `layout::grid`                                                    | 每行 `columns` 个等宽格子，行优先；行高均分，所以网格正好填满区域。                                                   |
| `layout::table`                                                   | 固定（`Track::value`）和弹性（`Track::weight`）的列与行；返回行优先的单元格矩形。                                     |
| `layout::clampScroll`、`layout::isVisible`、`layout::scrollThumb` | 可滚动范围：偏移上限、某项是否可见、滚动条滑块几何。                                                                  |
| `layout::mainExtent`、`layout::crossExtent`                       | 一组子控件需要多大，容器在 `measureContent` 里直接返回它。                                                            |

# 布局

| 组件 | 用途 |
| --- | --- |
| `Widget` | 基类节点：`measure`/`measureContent`、`layout`、`paint`、`hitTest`、尺寸约束，以及 `onClick`/`onFocus`/`onBlur`/`onHover` 回调。 |
| `Stack` | 行或列。`add(child)` 保持测量尺寸；`add(child, 1.f)` 让子控件拿走其他子控件分完后剩下的空间。 |
| `Label` | 文本块：字号、对齐、自动换行、行距、`maxLines`、`maxWidth`。 |
| `Button` | `Type`（default/secondary/destructive/outline/link）和 `Size`（xs/sml/default/lg）；`disabled(true)` 变暗并忽略点击。 |
| `Badge` | 小胶囊，用来标注另一个控件（`Default`、`Secondary`、`Outline`、`Destructive`）。 |

`Stack` 用拿到的宽度测量子控件；列方向把子控件拉伸到自己的宽度，行方向保留各子控件的测量宽度，把剩余空间分给被拉伸的子控件。

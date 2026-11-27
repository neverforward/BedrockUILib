# 表单字段

`Field` 把控件和它的标签、说明文字组合在一起，并承载那套 web 组件里挂在 field 元素上的状态：

| 成员 | 含义 |
| --- | --- |
| `label(...)` | `FieldLabel`，可以直接给控件，也可以只给文本让它自己建。 |
| `control(...)` | 控件本体，一般是 `Input`；也允许复合控件（输入框加按钮的一行）。 |
| `description(...)` | `FieldDescription`，控件下方灰掉的提示。 |
| `orientation(...)` | `Vertical`（默认）三段竖排；`Horizontal` 把标签放左边，控件拿剩余宽度。 |
| `disabled(true)` | 字段的 `data-disabled`：标签、说明变灰，控件禁用。 |
| `invalid(true)` | 字段的 `data-invalid`：控件用 `Theme::invalid` 绘制。 |
| `required(true)` | 标签上加一个 `Theme::invalid` 颜色的 `*`。 |

`FieldLabel` 支持在文字后接一个装饰，"Recommended" 徽标就是这么进标签的；`FieldDescription` 颜色更灰，字号略小。

状态通过 `Widget::setControlState` 自上而下施加：控件把容器施加的状态和自己直接得到的状态合并，所以

```cpp
field.disabled(true);          // 标签、说明、控件一起变灰
input.disabled(true);          // 不用 field，单个控件同样效果
```

或者使用 Spec 树

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.field(
        "Name",
        [&] { ui.input("Ada Lovelace"); },
        "Shown under the input.",
        [](bul::Field& field) { field.orientation(bul::Field::Orientation::Horizontal); }
    );
});
```


两种写法都成立，互不覆盖。
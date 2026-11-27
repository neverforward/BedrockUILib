# Checkbox

带内联标签的复选框，三个状态：未选、选中、半选。

```cpp
auto box = std::make_unique<bul::Checkbox>();
box->text("Accept terms");
box->onCheckedChange([](bool checked) { /* 新状态 */ });
field->control(std::move(box));
```

或者使用 Spec 树

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.checkbox("Accept terms", false, [](bul::Checkbox& box) {
        box.onCheckedChange([](bool checked) { /* 新状态 */ });
    });
});
```


| 成员 | 含义 |
| --- | --- |
| `text(text)` | 画在方框右侧的内联标签（Base UI 里 `Checkbox` 的外包 label）。字段自己的标签则是分组标题。 |
| `checked(bool)` / `checked()` | 状态。代码设置是程序行为，不触发 `onCheckedChange`。 |
| `indeterminate(bool)` / `indeterminate()` | 半选态（`data-indeterminate`）：画成短横线而不是对勾。它不算一个独立取值，点一下半选框变成选中，同时清掉标志。 |
| `onCheckedChange(fn)` | 点击或空格改变状态时触发。 |
| `readOnly(true)` | 可以聚焦，但点击和空格不改状态。 |
| `disabled(true)` | 不可聚焦；field 也能施加。 |
| `invalid(true)` | 方框变红：选中时整个 `Theme::invalid`，未选中时泛红色调；field 也能施加。 |

方框是 11 单位的正方形（一行高加一条边），控件高度和方框齐平。没有边框：未选时是凸起的表面色方块（指针或键盘焦点下更亮），选中或半选时是 accent 色。有焦点时空格切换状态；点在控件任意位置（方框或标签）都切换。对勾按像素绘制，任何 GUI 缩放下都清晰。

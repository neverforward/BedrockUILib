# Switch

开关，轨道颜色体现状态，和复选框一个思路。

```cpp
auto switchWidget = std::make_unique<bul::Switch>();
switchWidget->text("Wi-Fi");
switchWidget->onCheckedChange([](bool value) { /* 新状态 */ });
field->control(std::move(switchWidget));
```

或者使用 Spec 树

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.toggle("Wi-Fi", false, [](bul::Switch& toggle) {
        toggle.onCheckedChange([](bool value) { /* 新状态 */ });
    });
});
```


| 成员 | 含义 |
| --- | --- |
| `text(t)` | 画在轨道右侧的内联标签。 |
| `checked(b)` / `checked()` | 状态。代码设置是程序行为，不触发 `onCheckedChange`。 |
| `onCheckedChange(fn)` | 点击或空格翻转状态时触发。 |
| `readOnly(true)` | 可以聚焦，但状态不能变。 |
| `disabled(true)` / `required(true)` / `invalid(true)` | 常规控件状态；field 也能施加。 |

轨道 22 x 12 单位，滑块是 10 单位的像素圆：关的时候停在左侧表面色（`Theme::button`）上，开的时候停在右侧 `Theme::accent` 上。有焦点时空格切换；点在任意位置，轨道或标签，都翻转。invalid 给关态轨道加红色调，开态轨道直接用 `Theme::invalid`。

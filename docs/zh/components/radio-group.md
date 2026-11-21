# 单选组

单选按钮，恰好选一个；每个选项是一个带标签的 `Radio.Root`，选中的那个画 `Radio.Indicator` 圆点而不是对勾。

```cpp
auto group = std::make_unique<bul::RadioGroup>();
group->add("starter", "Starter");
group->add("pro", "Pro");
group->add("team", "Team", /* disabled */ true);
group->onValueChange([](std::string const& value) { /* 新选择 */ });
field->control(std::move(group));
```

| 成员 | 含义 |
| --- | --- |
| `add(value, text, disabled)` | 一个选项。value 是 `value()` 返回、`onValueChange` 上报的值。 |
| `value(v)` / `value()` | 选中的值，未选时为空。程序行为，无回调。 |
| `onValueChange(fn)` | 用户选中不同选项时触发；重复点同一个不触发。 |
| `direction(...)` / `gap(...)` | `Vertical`（默认）或 `Horizontal` 排列，间距规则同 Stack。 |
| `disabled(true)` | 所有选项变灰并退出焦点顺序。 |
| `readOnly(true)` | 选项仍可聚焦，但选择不能变。 |
| `invalid(true)` / `required(true)` | field 都能施加。未选中的 invalid 组让每个圆盘都带红色调；选中后只有选中的那个变红。 |

圆盘是 11 单位的像素圆，按行 span 绘制——和复选框同一套只分颜色的规则（未选表面色、选中 accent、无边框）。点选项即选中；空格选中聚焦的选项；上下左右方向键移动选择（跳过禁用项）；Tab 在选项间走。

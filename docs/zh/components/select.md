# Select

下拉框：触发器显示当前值，弹出列表画在树的其余部分之上。

```cpp
auto field = std::make_unique<bul::Field>();
field->label("Fruit");

auto select = std::make_unique<bul::Select>();
select->placeholder("Choose a fruit");
select->item("apple", "Apple");
select->item("durian", "Durian", /* disabled */ true);
select->onValueChange([](std::string const& value) { /* 新值 */ });
field->control(std::move(select));
```

或者使用 Spec 树

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.field("Fruit", [&] {
        ui.select(
            "Choose a fruit",
            {
                {.value = "apple", .label = "Apple"},
                {.value = "durian", .label = "Durian", .disabled = true},
            },
            [](bul::Select& select) {
                select.onValueChange([](std::string const& value) { /* 新值 */ });
            }
        );
    });
});
```


| 成员 | 含义 |
| --- | --- |
| `item(value, label, disabled)` | 一个选项。`Select.Item` + `ItemText`；label 默认等于 value；禁用的选项画成灰的，不能选。 |
| `value(v)` / `value()` | 选中的值。代码设置是程序行为，不触发 `onValueChange`。 |
| `placeholder(text)` | 未选中时 `Select.Value` 显示的提示（Base UI 的 `::placeholder`）。 |
| `open(true)` / `open()` | 开合状态；`onOpenChange(fn)` 上报每次变化（触发器、键盘、Esc、点击外部）。 |
| `onValueChange(fn)` | 只在选中不同选项时触发，重复点同一个不触发。 |
| `disabled(true)` | field 也能施加；禁用的 select 不能聚焦也不能打开。 |

选中行带一个 accent 色的 `ItemIndicator` 方块；指针或键盘高亮所在的行用 `Theme::buttonHover` 铺底。

**弹出层是 overlay，不是子控件。** 当子控件会被表单裁剪、按树序绘制，下面的字段会盖在它上面。所以控件通过 `collectOverlays` 上报打开的弹出层（容器原样转发），宿主先画树——flush 一遍让面板盖住底下的文字——再画弹出层，并把指针事件先给它。模态效果（和 Base UI 默认行为一致）也由此而来：`overlayHitTest` 认领整个屏幕，点外部关掉弹出层而不是触发后面的控件。Esc 和 Tab 先关弹出层，然后 Esc 才能关窗口。

键盘：上/下方向键打开弹出层并移动高亮（跳过禁用项），Enter 或空格提交高亮项，Esc 关闭。
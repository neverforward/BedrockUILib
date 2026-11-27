# Spec 树

除了链式 `make_unique`，也可以一次调用描述整棵树。**推荐用 builder** —— 也是计划中的 `Screen` API 的形态：

```cpp
#include "bul/SpecBuilder.h"

auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.label("Settings", 1.4f);
    ui.field("Name", [&] { ui.input("Ada Lovelace"); }, "Field + input.");
    ui.toggle("Notifications", true);
    ui.checkbox("Accept terms");
    ui.row([&] {
        ui.button("Apply", [] {});
        ui.button("Cancel", [] {});
    });
});
bul::showModal(std::move(tree));
```

| 成员 | 追加什么 |
| --- | --- |
| `row(body, configure)` / `column(body, configure)` | 一个 `Stack`；body 里的调用追加到它里面。 |
| `grid(columns, body, configure)` | 一个 `Grid`。 |
| `scroll(viewport, body, configure)` | 一个 `ScrollView`，内容是 body 里那列。 |
| `field(label, body, description, configure)` | 一个 `Field`；body 构建控件。 |
| `table(configure)` | 一个 `Table`；`configure` 填轨道和单元格。 |
| `label` / `button` / `badge` / `input` / `checkbox` / `toggle` / `select` / `radio` | 对应的控件。 |
| `fieldLabel` / `fieldDescription` | 手工拼字段时的标题与说明。 |
| `add(widget, stretch = 0)` | 任意控件；`stretch > 0` 分摊固定兄弟用剩的空间。 |
| `with(common)` | 把这些基类属性（`margin`、`style`、`className`、尺寸约束、事件）应用到下一个加入的控件。 |
| `id(value)` | 给下一个控件登记一个 id，之后用 `find` 取回。 |

每次调用都返回 builder，所以可以链式追加；基类属性跟在"最后加入的那个控件"后面，build() 收尾：

`cpp
auto tree = bul::spec::Builder{}
                .label("Settings", 1.4f)
                .margin(4.f)
                .button("Apply", [] {})
                .className("primary")
                .build();
`
每个容器和叶子还接受一个 *configure* 钩子 —— 一个拿到真实控件的 lambda —— 所以类型参数没覆盖到的部分也能设：

```cpp
ui.id("name").input("Ada", "", [](bul::Input& input) {
    input.validator([](std::string const& value) { return !value.empty(); });
    input.onChange([](std::string const& value) { /* ... */ });
});
```

## 运行时访问

`id(...)` 配合 `find<T>(...)` 能取回构建出的控件，所以树建好之后还能改状态：

```cpp
bul::spec::Builder ui;
ui.id("fruit").select("Pick a fruit", {{.value = "a", .label = "Apple"}});
auto tree = ui.take();

auto* fruit = ui.find<bul::Select>("fruit");
fruit->value("a");
```

套件不认识的控件也用同样的方式加入：

```cpp
ui.add(std::make_unique<my_mod::Divider>()); // 不需要 spec 节点
```

## 声明式结构体

同一棵树也可以用带指定初始化器的结构体写。builder 内部产出的就是这些结构体，`spec::node(...)` 包起来的也是它们，所以两种写法可以混用：

```cpp
auto tree = bul::spec::build(bul::spec::Stack{
    .gap      = 8.f,
    .children = {
        bul::spec::node(bul::spec::Label{.text = "Hello", .font = {.size = 1.4f}}),
        bul::spec::node(bul::spec::Field{
            .label   = "Name",
            .control = bul::spec::node(bul::spec::Input{.placeholder = "Ada"}),
        }),
    },
});
```

`Stack` 的子节点可以带拉伸系数，所以一行里可以混用固定子项和"拿走剩余空间"的子项：

```cpp
bul::spec::Stack{
    .children = {
        bul::spec::node(bul::spec::Button{.text = "Browse"}),
        bul::spec::weighted(bul::spec::node(bul::spec::Input{.placeholder = "Path"}), 1.f),
    },
}
```

`Common` 也带 `className` 和 `id`，所以节点可以像普通控件一样被[样式表选择器](/zh/components/stylesheets)选中。

每个节点都带 `common`，即 `Widget` 基类属性：`margin`、`fixedWidth` / `fixedHeight`、`minWidth` / `maxWidth`、`onClick` / `onFocus` / `onBlur` / `onHover` 回调、`style`、`className` 和 `id`。节点结构体 —— `Label`、`Button`、`Badge`、`Input`、`Field`、`Stack`、`Select`、`Checkbox`、`Switch`、`RadioGroup`、`Grid`、`Table`、`ScrollView`、`FieldLabel`、`FieldDescription` —— 都是聚合体，指定初始化写起来就像它描述的对象本身。子节点用 `spec::node(...)` 包一层，`spec::build(...)` 返回构建好的 `Widget` 树。

每个节点由控件自己应用：每个组件都有 `apply(spec::X const& node)` 设置自己的参数，`spec::buildContent()` 只是"构造控件 + 调用 apply"的桥。字段到 setter 的映射因此跟着控件走，而不是集中在某一个文件里。

套件不认识的控件在结构体写法里用 `spec::factory([] { return std::make_unique<my_mod::Divider>(); })` 加入。
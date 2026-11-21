# Spec 树

除了链式 `make_unique`，也可以直接用结构体描述一棵树，一次调用构建：

```cpp
auto tree = bul::spec::build(bul::spec::Stack{
    .gap      = 8.f,
    .children = {
        bul::spec::node(bul::spec::Label{.text = "Hello", .font = {.size = 1.4f}}),
        bul::spec::node(bul::spec::Field{
            .label   = "Name",
            .control = bul::spec::node(bul::spec::Input{.placeholder = "Ada"}),
        }),
        bul::spec::node(bul::spec::Switch{.text = "Notifications", .checked = true}),
    },
});
bul::showModal(std::move(tree));
```

每个节点都带 `common`，即 `Widget` 基类属性：`margin`、`fixedWidth` / `fixedHeight`、`minWidth` / `maxWidth`，以及 `onClick` / `onFocus` / `onBlur` / `onHover` 回调。节点结构体——`Label`、`Button`、`Badge`、`Input`、`Field`、`Stack`、`Select`、`Checkbox`、`Switch`、`RadioGroup`——都是聚合体，指定初始化写起来就像它描述的对象本身。子节点用 `spec::node(...)` 包一层，`spec::build(...)` 返回构建好的 `Widget` 树。

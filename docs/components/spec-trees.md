# Spec trees

A tree can also be described with structs and built in one call, instead of a
chain of `make_unique` calls:

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

Every node carries `common`, the `Widget` base properties: `margin`,
`fixedWidth` / `fixedHeight`, `minWidth` / `maxWidth`, and the `onClick` /
`onFocus` / `onBlur` / `onHover` handlers. The node structs — `Label`, `Button`,
`Badge`, `Input`, `Field`, `Stack`, `Select`, `Checkbox`, `Switch`,
`RadioGroup` — are aggregates, so a designated initializer reads like the
object it describes. Children are wrapped with `spec::node(...)`, and
`spec::build(...)` returns the finished `Widget` tree.

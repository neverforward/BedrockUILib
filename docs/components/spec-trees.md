# Spec trees

A tree can be described in one call instead of a chain of `make_unique` calls.
The recommended form is a builder body, the shape the planned `Screen` API uses:

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

| Member | Appends |
| --- | --- |
| `row(body, configure)` / `column(body, configure)` | A `Stack`; the body appends into it. |
| `grid(columns, body, configure)` | A `Grid`. |
| `scroll(viewport, body, configure)` | A `ScrollView` whose content is the body's column. |
| `field(label, body, description, configure)` | A `Field`; the body builds the control. |
| `table(configure)` | A `Table`; `configure` fills in the tracks and cells. |
| `label` / `button` / `badge` / `input` / `checkbox` / `toggle` / `select` / `radio` | The matching widget. |
| `fieldLabel` / `fieldDescription` | The field's parts, when a field is composed by hand. |
| `add(widget, stretch = 0)` | Any widget; `stretch > 0` shares the room the fixed siblings leave. |
| `with(common)` | Base properties (`margin`, `style`, `className`, size constraints, handlers) on the next widget. |
| `id(value)` | Registers the next widget so `find` can return it. |

Every call returns the builder, so additions chain; the common properties chain
after the widget added last, and build() closes the chain:

```cpp
auto tree = bul::spec::Builder{}
                .label("Settings", 1.4f)
                .margin(4.f)
                .button("Apply", [] {})
                .className("primary")
                .build();
```
Every container and leaf also takes a *configure* hook, a lambda over the real
widget, so anything the typed parameters do not cover is still reachable:

```cpp
ui.id("name").input("Ada", "", [](bul::Input& input) {
    input.validator([](std::string const& value) { return !value.empty(); });
    input.onChange([](std::string const& value) { /* ... */ });
});
```

## Runtime access

`id(...)` plus `find<T>(...)` return the built widget, so state can be changed
after the tree is built:

```cpp
bul::spec::Builder ui;
ui.id("fruit").select("Pick a fruit", {{.value = "a", .label = "Apple"}});
auto tree = ui.take();

auto* fruit = ui.find<bul::Select>("fruit");
fruit->value("a");
```

Anything the kit does not know joins the same way:

```cpp
ui.add(std::make_unique<my_mod::Divider>()); // no spec node needed
```

## Declarative structs

The same tree can be written as structs with designated initializers. This is what
the builder emits, and what `spec::node(...)` wraps, so the two forms mix:

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

A `Stack` child can carry a stretch factor, so a row can mix fixed children with
one that takes the rest:

```cpp
bul::spec::Stack{
    .children = {
        bul::spec::node(bul::spec::Button{.text = "Browse"}),
        bul::spec::weighted(bul::spec::node(bul::spec::Input{.placeholder = "Path"}), 1.f),
    },
}
```

`Common` also carries `className` and `id`, so a node can be targeted by a
[stylesheet selector](/components/stylesheets) like any widget.

Every node carries `common`, the `Widget` base properties: `margin`,
`fixedWidth` / `fixedHeight`, `minWidth` / `maxWidth`, the `onClick` / `onFocus` /
`onBlur` / `onHover` handlers, a `style`, a `className` and an `id`. The node
structs (`Label`, `Button`, `Badge`, `Input`, `Field`, `Stack`, `Select`,
`Checkbox`, `Switch`, `RadioGroup`, `Grid`, `Table`, `ScrollView`, `FieldLabel`,
`FieldDescription`) are aggregates, so a designated initializer reads like the
object it describes. Children are wrapped with `spec::node(...)`, and
`spec::build(...)` returns the finished `Widget` tree.

Each node is applied by the widget itself: every component has an
`apply(spec::X const& node)` that sets its parameters, and `spec::buildContent()`
is only the bridge that constructs the widget and calls it. The field-to-setter
mapping therefore lives next to the widget, not in one central file.

A widget the kit does not know joins a struct tree through
`spec::factory([] { return std::make_unique<my_mod::Divider>(); })`.

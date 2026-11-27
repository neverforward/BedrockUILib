# Form field

`Field` composes one control with its label and description, and carries the
state the web component set puts on the field element:

| Member | Meaning |
| --- | --- |
| `label(...)` | A `FieldLabel`, either given as a widget or created from text. |
| `control(...)` | The control itself, usually an `Input`; a composite control (a row of input and button) is allowed. |
| `description(...)` | A `FieldDescription`, the muted hint under the control. |
| `orientation(...)` | `Vertical` (default) stacks the three parts; `Horizontal` puts the label on the left and gives the control the remaining width. |
| `disabled(true)` | The field's `data-disabled`: dims the label and the description and disables the control. |
| `invalid(true)` | The field's `data-invalid`: the control is drawn with `Theme::invalid`. |
| `required(true)` | The label grows a `*` marker in `Theme::invalid`. |

`FieldLabel` takes an optional decoration after the text, which is how a
"Recommended" badge ends up inside the label; `FieldDescription` is muted and
slightly smaller.

State is imposed top-down through `Widget::setControlState`: a control combines
what its container imposed with the flags it was given directly, so

```cpp
field.disabled(true);          // dims label, description and control
input.disabled(true);          // the same effect for one control, without a field
```

Or with a spec tree

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


are both valid and neither overwrites the other.
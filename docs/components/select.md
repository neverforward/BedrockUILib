# Select

A dropdown: a trigger that shows the current value, and a popup list of options
drawn above the rest of the tree.

```cpp
auto field = std::make_unique<bul::Field>();
field->label("Fruit");

auto select = std::make_unique<bul::Select>();
select->placeholder("Choose a fruit");
select->item("apple", "Apple");
select->item("durian", "Durian", /* disabled */ true);
select->onValueChange([](std::string const& value) { /* the new value */ });
field->control(std::move(select));
```

Or with a spec tree

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
                select.onValueChange([](std::string const& value) { /* the new value */ });
            }
        );
    });
});
```


| Member | Meaning |
| --- | --- |
| `item(value, label, disabled)` | One option. `Select.Item` + `ItemText`; the label defaults to the value, and a disabled option is drawn dimmed and cannot be chosen. |
| `value(v)` / `value()` | The chosen value. Setting it is programmatic and does not fire `onValueChange`. |
| `placeholder(text)` | What `Select.Value` shows while nothing is selected (`::placeholder` in Base UI). |
| `open(true)` / `open()` | The open state; `onOpenChange(fn)` reports every change (trigger, keyboard, escape, outside click). |
| `onValueChange(fn)` | Fires only when a different option is chosen, not when the same one is picked again. |
| `disabled(true)` | `Field` can impose it too; a disabled select cannot be focused or opened. |

The selected row is marked with an accent `ItemIndicator` square; the row under
the pointer or keyboard highlight is filled with `Theme::buttonHover`.

**The popup is an overlay, not a child.** A child would be clipped to the form
and painted in tree order, so a field below it would draw over it. The widget
therefore reports the open popup through `collectOverlays` (containers forward
it), the host paints it after the tree — flushing the tree first, so its panel
covers the text under it — and gives it the pointer before the tree. This is
also what makes it modal like Base UI's default: `overlayHitTest` claims the
whole screen, so an outside click closes the popup instead of activating the
control behind it. Escape and Tab close it first; only then does Escape close
the window.

Keyboard: Up/Down open the popup and move the highlight (disabled options are
skipped), Enter or Space commits the highlighted option, Escape closes.
# Checkbox

A box with an inline label and three states: unticked, ticked and mixed.

```cpp
auto box = std::make_unique<bul::Checkbox>();
box->text("Accept terms");
box->onCheckedChange([](bool checked) { /* the new state */ });
field->control(std::move(box));
```

| Member | Meaning |
| --- | --- |
| `text(text)` | The inline label drawn right of the box (`Checkbox`'s enclosing label in Base UI). The field's own label, when there is one, is the group heading. |
| `checked(bool)` / `checked()` | The state. Setting it is programmatic and does not fire `onCheckedChange`. |
| `indeterminate(bool)` / `indeterminate()` | The mixed state (`data-indeterminate`): drawn as a dash instead of a tick. It is not a value of its own — clicking a mixed box ticks it and clears the flag. |
| `onCheckedChange(fn)` | Fires when a click or Space changes the state. |
| `readOnly(true)` | Focusable, but clicks and Space do not change the state. |
| `disabled(true)` | Not focusable; the `Field` can impose it too. |
| `invalid(true)` | Turns the box red: fully `Theme::invalid` when ticked, a red cast when not; the `Field` can impose it too. |

The box is 11 units square (one line plus a border) and the widget is as tall as
the box. It has no border: unticked it is a raised surface square (lighter under the
pointer or keyboard focus), ticked or mixed it is the accent colour. Space
toggles while the checkbox has focus; a click anywhere in the widget — box or
label — toggles it. The tick is drawn as pixels, so it stays crisp at any GUI
scale.

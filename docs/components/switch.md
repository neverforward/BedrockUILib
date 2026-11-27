# Switch

An on/off switch, The track colour carries the state, like the checkbox.

```cpp
auto switchWidget = std::make_unique<bul::Switch>();
switchWidget->text("Wi-Fi");
switchWidget->onCheckedChange([](bool value) { /* the new state */ });
field->control(std::move(switchWidget));
```

Or with a spec tree

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.toggle("Wi-Fi", false, [](bul::Switch& toggle) {
        toggle.onCheckedChange([](bool value) { /* the new state */ });
    });
});
```


| Member | Meaning |
| --- | --- |
| `text(t)` | The inline label drawn right of the track. |
| `checked(b)` / `checked()` | The state. Setting it is programmatic and does not fire `onCheckedChange`. |
| `onCheckedChange(fn)` | Fires when a click or Space flips the state. |
| `readOnly(true)` | Focusable, but the state cannot change. |
| `disabled(true)` / `required(true)` / `invalid(true)` | The usual control states; the field can impose them too. |

The track is 22 x 12 units and the thumb is a 10-unit pixel circle: it sits on
the left on the raised surface (`Theme::button`) and on the right on
`Theme::accent` once the switch is on. Space toggles while the switch has focus;
a click anywhere — track or label — flips it. Invalid tints the off track red
and paints the on track in `Theme::invalid` itself.
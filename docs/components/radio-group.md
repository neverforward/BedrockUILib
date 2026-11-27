# Radio group

Radio buttons with exactly one choice; every option is `Radio.Root` with its
label, and the chosen one paints a `Radio.Indicator` dot instead of a tick.

```cpp
auto group = std::make_unique<bul::RadioGroup>();
group->add("starter", "Starter");
group->add("pro", "Pro");
group->add("team", "Team", /* disabled */ true);
group->onValueChange([](std::string const& value) { /* the new choice */ });
field->control(std::move(group));
```

Or with a spec tree

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.radio(
        {
            {.value = "starter", .text = "Starter"},
            {.value = "pro", .text = "Pro"},
            {.value = "team", .text = "Team", .disabled = true},
        },
        "",
        [](bul::RadioGroup& group) {
            group.onValueChange([](std::string const& value) { /* the new choice */ });
        }
    );
});
```


| Member | Meaning |
| --- | --- |
| `add(value, text, disabled)` | One option. The value is what `value()` returns and `onValueChange` reports. |
| `value(v)` / `value()` | The chosen value, empty while nothing is chosen. Programmatic, no callback. |
| `onValueChange(fn)` | Fires when the user chooses a different option; choosing the same one again is silent. |
| `direction(...)` / `gap(...)` | `Vertical` (default) or `Horizontal` layout, with the stack's gap rule. |
| `disabled(true)` | Every option dims and leaves the focus order. |
| `readOnly(true)` | Options stay focusable, but the choice cannot change. |
| `invalid(true)` / `required(true)` | The field can impose both. An invalid group with no choice tints every disc; once something is chosen only that disc is red. |

The disc is an 11-unit pixel circle drawn as row spans — the same colour-only
rule as the checkbox (surface while unchosen, accent when chosen, no border).
Clicking an option chooses it; Space chooses the focused option; Up/Down and
Left/Right move the choice, skipping disabled options; Tab walks the options.
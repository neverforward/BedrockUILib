# Components

The widget kit lives in `src/bul/widget/`, one file pair per component. A widget
only measures, lays out and paints through `Canvas`; it never touches the engine.
That is what lets the same tree run in the game, in a test with a recording
canvas, or in an offline previewer.

```cpp
#include "bul/Widget.h"   // the umbrella header: every component below

auto root = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
root->gap(8.f);
root->add(std::make_unique<bul::Label>("Settings"));
root->add(std::make_unique<bul::Button>("Apply", [] { /* ... */ }));

bul::showModal(std::move(root)); // bul/Modal.h
```

## Layout

| Component | Purpose |
| --- | --- |
| `Widget` | Base node: `measure`/`measureContent`, `layout`, `paint`, `hitTest`, size constraints and the `onClick`/`onFocus`/`onBlur`/`onHover` handlers. |
| `Stack` | Row or column. `add(child)` keeps the measured size; `add(child, 1.f)` lets the child take the room the other children leave. |
| `Label` | A text block: size, alignment, word wrap, line spacing, `maxLines` and `maxWidth`. |
| `Button` | `Type` (default/secondary/destructive/outline/link) and `Size` (xs/sml/default/lg); `disabled(true)` dims it and ignores clicks. |
| `Badge` | Small pill that annotates another control (`Default`, `Secondary`, `Outline`, `Destructive`). |

A `Stack` measures its children with the width it was offered; a column stretches
its children to its own width, a row keeps each child's measured width and shares
the leftover space with the stretched children.


## Widget base

Every component derives from `Widget`, so the properties below exist everywhere.

| Member | Meaning |
| --- | --- |
| `margin(all)` / `margin(h, v)` | Space kept outside the widget when a parent measures and lays it out. |
| `size()` / `width()` / `height()` | The laid-out size, in GUI units. |
| `fixedSize(w, h)` / `fixedWidth(w)` / `fixedHeight(h)` | Overrides the measured size; the widget's bounds are clamped to it even when the parent offers more. |
| `minSize(w, h)` / `maxSize(w, h)` | Bounds the measured content size (0 = no constraint). |
| `onClick(fn)` | Makes any widget clickable: a `Label` with a click handler becomes interactive, even though its kind has no action of its own. |
| `onFocus(fn)` / `onBlur(fn)` | Run when the host gives or takes keyboard focus. |
| `onHover(fn)` | `fn(bool entered)` when the pointer enters or leaves the widget. |
| `notifyClick` / `notifyFocus` / `notifyHover` | Host entry points: they run the widget's own reaction first, then the callback. |

Components override `measureContent(...)`; `measure(...)` applies the size
constraints and the margin, so every widget is sized through one path. The click,
focus and hover handlers are the widget-level spelling of the HTML element
attributes of the same names.

## Text

`Label` is the text block, in the spirit of Qt's `QLabel`: it can wrap, scale,
align and cap itself, and the same layout is used for measuring and for painting.

```cpp
auto label = std::make_unique<bul::Label>("Long enough to need a second line.");
label->wrap(true);          // break at spaces, and between CJK characters
label->maxWidth(160.f);     // keep a readable column width
label->lineSpacing(2.f);
label->maxLines(2);         // the second line ends with "..."
label->align(bul::Align::Center);
label->verticalAlign(bul::VAlign::Center);
```

| Member | Meaning |
| --- | --- |
| `text(t)` / `text()` | The string. Explicit `\n` always starts a new line. |
| `size(s)` / `size()` | Font scale around the theme's base size (1 = body text). |
| `align(a)` / `align()` | Horizontal alignment of each line: `Left`, `Center`, `Right`. |
| `verticalAlign(v)` / `verticalAlign()` | Where the block sits inside the bounds: `Top`, `Center`, `Bottom`. |
| `wrap(true)` / `wrap()` | Word wrap at the available width. Spaces are break opportunities; CJK breaks between any two characters because it has no spaces. |
| `maxWidth(w)` / `maxWidth()` | Caps the room the text uses even when the parent offers more; the widget reports `w` as its width, so the column stays stable. |
| `lineSpacing(v)` / `lineSpacing()` | Extra space between lines, in units at theme scale 1. |
| `maxLines(n)` / `maxLines()` | Truncates the block to `n` lines; the last one is elided. |
| `color(c)` / `color()` | Explicit text colour (`Rgba`); when set it wins over `muted()`. |
| `elide(b)` / `elide()` | Cuts a single line that does not fit with a trailing `...` (on by default). |
| `muted()` | Draws with `Theme::textMuted`, the caption colour. |

Wrapping is greedy: each line takes code points until the next one would not fit,
then backs up to the last break opportunity. A single glyph wider than the column
is allowed to overflow rather than disappearing. Text is measured with the same
per-code-point advance table the input uses, so a CJK line breaks where it is
actually drawn.

## Form field

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

are both valid and neither overwrites the other.

## Input

```cpp
auto field = std::make_unique<bul::Field>();
field->label("Email");
field->description("Shown under the input.");

auto input = std::make_unique<bul::Input>();
input->placeholder("you@example.com");
input->onChange([](std::string const& value) { /* called on every edit */ });
field->control(std::move(input));
```

| Member | Meaning |
| --- | --- |
| `value(text)` / `value()` | The text. Setting it does not fire `onChange`; user edits do. |
| `placeholder(text)` | Drawn in `Theme::textMuted` while the value is empty. |
| `kind(...)` | `Text`, `Password` (one `*` per code point), `Search`, `File`. |
| `disabled(true)` / `invalid(true)` | The two states; the field can impose them too. |
| `required(true)` | Marks the input as required (the marker itself is drawn by the label). |
| `maxLength(n)` | Limit in code points, not bytes: a Chinese character counts as one. |
| `onSubmit(fn)` | Enter pressed while the input has focus. |
| `validator(fn)` | `fn(const std::string&) -> bool` runs after every user edit. While it returns false the input is drawn with `Theme::invalid` (`invalid()` is true); the first passing edit clears it. Initial and programmatic values are not judged. |
| `kind(File)` + host picker | Clicking a `Kind::File` input asks the modal host for a path (`wantsFilePicker`); the host opens the native file dialog and calls `chooseFile(path)`. |
| `onChoose(fn)` | A file was chosen: the callback receives the full path. |
| `filePath()` / `filePath(p)` | The chosen path (the full path; the field itself shows the file name). |
| `chooseText(text)` / `emptyFileText(text)` | The two strings of a file input, for localization. |

Editing rules live in `bul/TextEdit` and are unit-tested without a game: the
caret is a byte offset that always sits on a code point boundary, Shift extends a
selection from an anchor, typing or pasting replaces the selection, and pasted
line breaks are dropped because the field is single line.

Text events are a **delta stream**, not plain text: an IME rewriting its
composition sends a backspace (`0x08`) for each character it takes back and then
the new composition; `TextEdit::insert` applies those edits, so the pinyin never
piles up in front of the committed characters.

The box does not resize with its value: the value is scrolled while the field is
focused and cut with an ellipsis otherwise, so a long string cannot push the
window or the surrounding form around.

**Wide glyphs get their own advance.** The engine measures CJK / Hangul /
fullwidth code points narrower than the unicode fallback font it actually draws
them with (the ASCII resource-pack font and the built-in unifont disagree). A
field that trusted that measurement would draw text wider than the box and leave
the caret inside the text. `Input` therefore lays the line out on its own
per-code-point advance table -- a wide code point advances at least one line
height -- and the caret, the selection plate, the visible window and the glyph
positions all come from that one table. Runs of narrow glyphs are still drawn as
one string; each wide glyph is placed on its cell.

Focus, the caret and the clipboard belong to the host, not to the widget:

- clicking a focusable widget gives it focus (`setFocused`), clicking anywhere
  else takes it away;
- Tab and Shift+Tab move between inputs in visual order;
- Backspace, Delete, the arrow keys, Home, End and Ctrl+A are routed to the
  focused widget as `KeyStroke`s;
- **the host claims the platform keyboard while a text field has focus**
  (`KeyboardManager::tryClaimKeyboardOwnership` + `tryEnableKeyboard`). Without
  that claim the engine sends no typed text at all -- this is the piece that
  makes an input editable, and it is also what lets the platform drive the IME;
- typed characters then arrive through the scene's own text entry point
  (`UIScene::handleTextChar`) and are routed to the focused widget;
- the keyboard is released when focus leaves the field and when the window
  closes, so the game's own text fields (chat) keep working;
- Ctrl+C / Ctrl+X / Ctrl+V use the OS clipboard, because typing CJK still
  depends on the engine's text services.

A file input is different: the widget never opens a dialog. It reports
`wantsFilePicker()`, the host opens `bul::platform::openFileDialog`, and the
chosen path is handed back through `chooseFile`. The field itself shows the file
name; `filePath()` keeps the full path.

## Select

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
it), the host paints it after the tree -- flushing the tree first, so its panel
covers the text under it -- and gives it the pointer before the tree. This is
also what makes it modal like Base UI's default: `overlayHitTest` claims the
whole screen, so an outside click closes the popup instead of activating the
control behind it. Escape and Tab close it first; only then does Escape close
the window.

Keyboard: Up/Down open the popup and move the highlight (disabled options are
skipped), Enter or Space commits the highlighted option, Escape closes.

## Checkbox

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
| `indeterminate(bool)` / `indeterminate()` | The mixed state (`data-indeterminate`): drawn as a dash instead of a tick. It is not a value of its own -- clicking a mixed box ticks it and clears the flag. |
| `onCheckedChange(fn)` | Fires when a click or Space changes the state. |
| `readOnly(true)` | Focusable, but clicks and Space do not change the state. |
| `disabled(true)` | Not focusable; the `Field` can impose it too. |
| `invalid(true)` | Turns the box red: fully `Theme::invalid` when ticked, a red cast when not; the `Field` can impose it too. |

The box is 11 units square (one line plus a border) and the widget is as tall as
the box. It has no border: unticked it is a raised surface square (lighter under the
pointer or keyboard focus), ticked or mixed it is the accent colour. Space
toggles while the checkbox has focus; a click anywhere in the widget -- box or
label -- toggles it. The tick is drawn as pixels, so it stays crisp at any GUI
scale.

## Switch

An on/off switch: HTML's `input[type=checkbox][role=switch]` and Qt Quick's
`Switch` in one widget. The track colour carries the state, like the checkbox.

```cpp
auto switchWidget = std::make_unique<bul::Switch>();
switchWidget->text("Wi-Fi");
switchWidget->onCheckedChange([](bool value) { /* the new state */ });
field->control(std::move(switchWidget));
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
a click anywhere -- track or label -- flips it. Invalid tints the off track red
and paints the on track in `Theme::invalid` itself.

## Radio group

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

| Member | Meaning |
| --- | --- |
| `add(value, text, disabled)` | One option. The value is what `value()` returns and `onValueChange` reports. |
| `value(v)` / `value()` | The chosen value, empty while nothing is chosen. Programmatic, no callback. |
| `onValueChange(fn)` | Fires when the user chooses a different option; choosing the same one again is silent. |
| `direction(...)` / `gap(...)` | `Vertical` (default) or `Horizontal` layout, with the stack's gap rule. |
| `disabled(true)` | Every option dims and leaves the focus order. |
| `readOnly(true)` | Options stay focusable, but the choice cannot change. |
| `invalid(true)` / `required(true)` | The field can impose both. An invalid group with no choice tints every disc; once something is chosen only that disc is red. |

The disc is an 11-unit pixel circle drawn as row spans -- the same colour-only
rule as the checkbox (surface while unchosen, accent when chosen, no border).
Clicking an option chooses it; Space chooses the focused option; Up/Down and
Left/Right move the choice, skipping disabled options; Tab walks the options.


## Theming

Every colour and metric is a `Theme` token with a neutral placeholder default
(`bul::Theme::neutral()`). The input-specific tokens are `input`, `inputHover`,
`inputBorder` and `invalid`; the rest are shared with the buttons and the panel.

```cpp
bul::Theme theme = bul::Theme::neutral();
theme.accent = bul::Rgba::fromRgb(0x59A9FF);
theme.input  = bul::Rgba::fromRgb(0x101319);
bul::setTheme(theme);
```

## Consuming from another mod

The library is a DLL with an exported C++ API: every public class and function
carries `BUL_API` (`bul/Export.h`), and the build generates an import library
next to the dll. A consumer mod

- adds `src/` and `src-client/` of this repository to its include path,
- links `BedrockUILib.lib`, and
- is built with the same compiler toolchain and LeviLamina SDK version, because
  the C++ ABI crosses the dll boundary.



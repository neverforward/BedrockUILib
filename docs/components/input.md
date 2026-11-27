# Input

```cpp
auto field = std::make_unique<bul::Field>();
field->label("Email");
field->description("Shown under the input.");

auto input = std::make_unique<bul::Input>();
input->placeholder("you@example.com");
input->onChange([](std::string const& value) { /* called on every edit */ });
field->control(std::move(input));
```

Or with a spec tree

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.field(
        "Email",
        [&] {
            ui.input("you@example.com", "", [](bul::Input& input) {
                input.onChange([](std::string const& value) { /* called on every edit */ });
            });
        },
        "Shown under the input."
    );
});
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


Focus, the caret and the clipboard belong to the host, not to the widget:

- clicking a focusable widget gives it focus (`setFocused`), clicking anywhere
  else takes it away;
- Tab and Shift+Tab move between inputs in visual order;
- Backspace, Delete, the arrow keys, Home, End and Ctrl+A are routed to the
  focused widget as `KeyStroke`s;
- **the host claims the platform keyboard while a text field has focus**
  (`KeyboardManager::tryClaimKeyboardOwnership` + `tryEnableKeyboard`). Without
  that claim the engine sends no typed text at all — this is what makes an input
  editable, and it is also what lets the platform drive the IME;
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
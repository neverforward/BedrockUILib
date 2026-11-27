# Text

`Label` is the text block, it can wrap, scale, align
and cap itself, and the same layout is used for measuring and for painting.

```cpp
auto label = std::make_unique<bul::Label>("Long enough to need a second line.");
label->wrap(true);          // break at spaces, and between CJK characters
label->maxWidth(160.f);     // keep a readable column width
label->lineSpacing(2.f);
label->maxLines(2);         // the second line ends with "..."
label->align(bul::Align::Center);
label->verticalAlign(bul::VAlign::Center);
```

Or with a spec tree

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.label("Long enough to need a second line.", 1.f, false, [](bul::Label& label) {
        label.wrap(true);        // break at spaces, and between CJK characters
        label.maxWidth(160.f);   // keep a readable column width
        label.lineSpacing(2.f);
        label.maxLines(2);       // the second line ends with "..."
        label.align(bul::Align::Center);
        label.verticalAlign(bul::VAlign::Center);
    });
});
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
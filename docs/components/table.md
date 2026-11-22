# Table

Cells on fixed and flexible tracks. A track is a `layout::Track`: `{ .value = n }`
for a fixed size, or `{ .weight = w }` for a share of the space the fixed tracks
leave over. The layout comes from `layout::table`.

```cpp
auto table = std::make_unique<bul::Table>();
table->column({.value = 90.f});
table->column({.weight = 1.f});
table->row({.value = 9.f});
table->row({.weight = 1.f});
table->cell(0, 0, std::make_unique<bul::Label>("Name"));
table->cell(0, 1, std::make_unique<bul::Label>("Value"));
table->cell(1, 0, std::make_unique<bul::Label>("Gold"));
table->cell(1, 1, std::make_unique<bul::Label>("12"));
```

| Member | Meaning |
| --- | --- |
| `column(track)` / `row(track)` | Appends a track: `{.value}` fixed, `{.weight}` flexible. |
| `cell(row, column, child)` | Places a widget; an index past the declared tracks appends a flexible one, so a table can be built by cells alone. |
| `gap(columnGap, rowGap)` | Space between tracks. |
| `padding(all)` / `padding(horizontal, vertical)` | Inset of the whole grid from the border, 0 by default. A non-zero value makes the first and last rows taller than the inner ones, so `cellPadding` is the usual way to keep text off the border. |
| `cellPadding(all)` / `cellPadding(horizontal, vertical)` | Inset of the content inside each cell, 4 units by default, so text does not sit on the separator lines. |
| `border(bool)` / `borderColor(color)` | Draws the outer border and the row/column separators; on by default, `Theme::panelBorder` when no colour is set. |
| `measureContent` | The sum of the fixed tracks plus the largest cell in each flexible track. |

Every cell stretches to the rectangle its row and column solve to, so a cell
widget lays itself out inside that rectangle like any other child. A fixed
Track::value is the *content* size: the cell padding is added around it, so a
{.value = 9.f} row is nine units of text plus the padding.

# Grid

A grid of equal cells, row-major: items go left to right, then wrap. The cells are
solved by `layout::grid`, so the rows split the height evenly and the grid fills
the rectangle it is given.

```cpp
auto grid = std::make_unique<bul::Grid>(3); // three columns
grid->gap(6.f);
grid->add(std::make_unique<bul::Button>("One"));
grid->add(std::make_unique<bul::Button>("Two"));
grid->add(std::make_unique<bul::Button>("Three"));
```

Or with a spec tree
```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.grid(
        3,
        [&] {
            ui.button("One", [] {});
            ui.button("Two", [] {});
            ui.button("Three", [] {});
        },
        [](bul::Grid& grid) { grid.gap(6.f); }
    );
});
```


| Member | Meaning |
| --- | --- |
| `Grid(columns)` / `columns(n)` | Cells per row; the last row may be short. |
| `add(child)` | Appends a cell, row-major. |
| `gap(value)` | Space between cells; `< 0` (the default) uses `Theme::gap`. |
| `measureContent` | The widest child times the columns and the tallest times the rows, plus the gaps. |

Under the hood: the measured size gives the largest cell in each direction; the
layout then splits the bounds into `columns` equal cells and stretches each child
to its cell.
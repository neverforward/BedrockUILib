# Layout

| Component | Purpose                                                                                                                                        |
| --------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| `Widget`  | Base node: `measure`/`measureContent`, `layout`, `paint`, `hitTest`, size constraints and the `onClick`/`onFocus`/`onBlur`/`onHover` handlers. |
| `Stack`   | Row or column. `add(child)` keeps the measured size; `add(child, 1.f)` lets the child take the room the other children leave.                  |
| `Grid`    | Equal cells in N columns, row-major ([Grid](/components/grid)).                                                                                |
| `Table`   | Cells on fixed and flexible tracks ([Table](/components/table)).                                                                               |
| `Label`   | A text block: size, alignment, word wrap, line spacing, `maxLines` and `maxWidth`.                                                             |
| `Button`  | `Type` (default/secondary/destructive/outline/link) and `Size` (xs/sml/default/lg); `disabled(true)` dims it and ignores clicks.               |
| `Badge`   | Small pill that annotates another control (`Default`, `Secondary`, `Outline`, `Destructive`).                                                  |

A `Stack` measures its children with the width it was offered; a column stretches
its children to its own width, a row keeps each child's measured width and shares
the leftover space with the stretched children.

```cpp
auto const rects = bul::layout::box(
    {0.f, 0.f, 200.f, 20.f},
    {bul::layout::Item{{60.f, 18.f}}, bul::layout::Item{{0.f, 18.f}, 1.f}},
    {.axis = bul::layout::Axis::Horizontal, .gap = 8.f}
); // the first child keeps 60, the second takes the rest of the row
```

Or with a spec tree

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.row([&] {
        ui.button("Browse", [] {});
        ui.add(std::make_unique<bul::Input>(), 1.f); // the stretch factor
    });
});

```

## The solver

`bul/Layout.h` turns measurements into rectangles. Every rule there is pure
geometry with no widgets and no engine types, so it is unit tested; `Stack` is
one consumer, and a container of your own can use the same primitives.

| Function                                                          | Solves                                                                                                                                         |
| ----------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| `layout::box`                                                     | A row or column: main-axis sizes, `grow` items sharing the leftover, `gap`, `padding` and cross-axis `Align` (start / center / end / stretch). |
| `layout::grid`                                                    | Equal cells in `columns` columns, row-major; the rows split the height, so the grid fills its area.                                            |
| `layout::table`                                                   | Fixed (`Track::value`) and flexible (`Track::weight`) columns and rows; the cell rectangles come back row-major.                               |
| `layout::clampScroll`, `layout::isVisible`, `layout::scrollThumb` | A scrollable range: the offset limit, whether an item is in view, and the scrollbar thumb geometry.                                            |
| `layout::mainExtent`, `layout::crossExtent`                       | The size a list of children needs, which a container returns from `measureContent`.                                                            |

# Layout

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

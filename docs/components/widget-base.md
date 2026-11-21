# Widget base

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
constraints and the margin, so every widget is sized through one path.

# Scroll

A viewport over a taller content widget. The content is laid out at its measured
height, shifted by the offset, clipped to the viewport, and a bar on the right
edge shows where the view is.

```cpp
auto list = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
for (int i = 1; i <= 20; ++i) {
    list->add(std::make_unique<bul::Label>("Row " + std::to_string(i)));
}

auto view = std::make_unique<bul::ScrollView>();
view->viewport(120.f); // the visible height
view->content(std::move(list));
view->scrollBy(24.f);  // programmatic scroll
```

Or with a spec tree

```cpp
auto tree = bul::spec::build([](bul::spec::Builder& ui) {
    ui.scroll(
        120.f,
        [&] {
            for (int i = 1; i <= 20; ++i) {
                ui.label("Row " + std::to_string(i));
            }
        },
        [](bul::ScrollView& view) { view.scrollBy(24.f); }
    );
});
```

| Member | Meaning |
| --- | --- |
| `content(child)` | The single content widget, usually a `Stack`. |
| `viewport(n)` | Height of the visible window; `0` (the default) means "as tall as the content", which never scrolls. |
| `scrollbar(bool)` | Draws the bar on the right edge; on by default. |
| `scrollTo(value)` / `scrollBy(delta)` / `offset()` | Programmatic scrolling; the offset is clamped to `content - viewport`. |
| `contentHeight()` / `scrollable()` | What was measured and whether anything is hidden. |
| `wheelStep(n)` | Distance one wheel notch scrolls; 24 units by default. |

When the bar is shown the content stops short of it: the lane is 4 units wide and
the content keeps another 4 units of gutter, so text and controls never sit on the
bar. Without scrolling there is no lane and the content uses the full width.

The bar is solved by `layout::scrollThumb`, so it shrinks with the visible
fraction and slides with the offset. The content is painted inside
`Canvas::pushClip`, so rows at the edge are cut by the viewport instead of
bleeding over it; the game canvas maps that to the engine's clipping rectangle.

The bar is interactive: pressing the thumb drags the view, pressing the lane
jumps to that position and keeps dragging. The press captures the pointer, so the
content under the bar does not receive the drag.

The wheel is routed by the host: it is offered to the widget under the pointer,
bubbles through the containers above it, and a `ScrollView` scrolls one
`wheelStep` per notch. A consumer can still call `scrollTo` or `scrollBy` from its
own input.

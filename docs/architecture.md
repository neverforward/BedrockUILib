# Architecture

Status: the model layer and the modal host are implemented and unit tested; a
HUD element registry and a named-screen service (`ui.registerScreen`, `ui.push`)
are still planned.

## Layers

```
src/                     pure C++, no mc/ or ll/ includes, unit tested
  bul/Geometry.h         Rect, Point, half-open containment, insets
  bul/Color.h            Rgba with fromRgb / fromArgb / withAlpha
  bul/TextMetrics.h      measurement interface + UTF-8 safe truncation
  bul/Canvas.h           drawing surface the model paints into (fill/outline/text)
  bul/Theme.*            colour tokens and the neutral defaults
  bul/Style.*            per-widget colour overrides
  bul/StyleSheet.*       CSS-like rules with type, className and id selectors
  bul/Layout.*           row/column, grid, table and scroll solving
  bul/TextEdit.*         the edit model: caret, selection, insert/erase
  bul/Widget.h           umbrella header for the widget kit
  bul/widget/*           Widget base + one file pair per component, TextFlow.h
  bul/Spec.*             declarative spec nodes and the tree builder
  bul/SpecBuilder.*      the imperative builder

src-client/              engine adapters, verified in game
  bul/gfx/Painter.*      immediate-mode drawing over MinecraftUIRenderContext
  bul/gfx/GameCanvas.*   Canvas implementation: drawing plus clipping
  bul/Modal.*            public API: showModal, closeModal, setTheme, theme
  bul/modal/*            host internals: state, scene hooks, render, input, text
  bul/platform/*         OS clipboard and file dialog
  mod/*                  LeviLamina entry and the client service

src-demo/                --tests=y only, never shipped
  bul/demo/DemoPanel.*   the gallery: one page per component, F6 toggles it
```

The model half has no `mc/` or `ll/` dependency, so a test compiles it without
the game. `src-client` is the only place that touches the SDK.

## Frame model

1. `showModal(tree)` creates a placeholder screen
   (`SceneFactory::createCommonDialogInfoScreen("", "")`) and pushes it on the
   client scene stack. The placeholder supplies what the library does not own:
   the engine's input capture, its cursor and Esc handling.
2. A hook on `UIScene::$render` recognises the placeholder and replaces its
   drawing with the widget tree. `ModalRender` measures the tree against the
   window, lays it out and paints it through `GameCanvas`. Popups are painted
   after the tree.
3. Closing waits until the placeholder is the top of the stack and then calls
   `schedulePopScreen(1)`. The exit hook resets focus, pointer capture, hover and
   pending text edits, so a closed window leaves nothing behind.
4. `ModalInput` listens to `ll::event::input::KeyInputEvent` and
   `MouseInputEvent`. Mouse buttons are cancelled while the modal is open; moves
   pass through, because the engine has to keep its pointer position alive for
   hit testing. A click moves focus to a focusable control and is delivered
   through `hitTest`; the left press can capture the pointer
   (`press`/`drag`/`release`), which is how the scrollbar drags. Keys go to the
   focused widget; Tab and Shift+Tab move focus; Esc closes.
5. The wheel arrives as a Windows wheel delta (120 per notch). It is offered to
   the widget under the pointer and bubbles through the containers; `ScrollView`
   multiplies it by `wheelStep`.
6. The engine has no event for typed text. `ModalText` hooks the client input
   callback and routes characters to the focused field, and the host claims the
   platform keyboard (`KeyboardManager`) while a text field has focus. That claim
   is what makes typing and the IME work.

## Rendering rules

- `Painter` keeps the engine's batching order: rectangles, then images, then
  text. `flush()` closes a z band, and everything drawn after it is on top.
- Text is deferred: `flushText` draws with the clipping rectangle that is
  current at flush time. `ScrollView` flushes before narrowing the clip (so text
  already queued by earlier widgets is not cut) and again inside it (so its own
  text is clipped), then pops the clip.
- Clipping nests by intersection, and the painter keeps the rectangle stack and
  applies the intersection to the engine.
- Coordinates are GUI units. `Painter` converts them to the engine's
  `RectangleArea` for both fills and the clip.

## Public API

What a consumer calls today:

```cpp
#include "bul/Modal.h"
#include "bul/SpecBuilder.h"

bul::Theme theme = bul::Theme::neutral();
theme.text = bul::Rgba::fromRgb(0xE0E0E0);
bul::setTheme(theme);

bul::showModal(bul::spec::build([](bul::spec::Builder& ui) {
    ui.label("Settings", 1.4f);
    ui.row([&] {
        ui.button("Apply", [] {});
        ui.button("Cancel", [] {});
    });
}));
```

The planned service layer adds a name per screen and a HUD registry:

```cpp
ui.registerScreen("my_mod.settings", [](bul::Screen& screen) { /* builder */ });
ui.push("my_mod.settings");
ui.hudElement("my_mod.hud", [](bul::Hud& hud) { hud.text("..."); });
```

`Screen` would wrap the builder that exists now, and `Hud` would reuse the same
tree with a per-frame update hook. Shapes may change when those pieces exist.

## Test plan

`src-test/common` runs 209 tests without the game:

- geometry and layout solving: rows, columns, grid, table, scroll ranges;
- text layout, CJK advances, wrapping and truncation;
- the edit model: caret, selection, insert, erase, IME delta streams;
- every component: measure, layout, paint, hit test, state and callbacks;
- theme, style and stylesheet resolution, including specificity;
- spec nodes and the builder: parameters, chaining, ids, custom widgets;
- the modal-free protocol pieces: wheel routing, clipping calls, pointer
  capture.

Verified in game, by hand:

- the window draws inside the game's UI pass and follows the GUI scale;
- mouse, wheel and keyboard behave; the scrollbar drags; text and CJK carets
  work through the engine font;
- closing returns input to the game with no leftovers.

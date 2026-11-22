# Architecture

Status: **skeleton + feasibility demo**. The module map and public API below are the plan; `Geometry`, `Color`, `TextMetrics` and `gfx::Painter` exist, and a `--tests=y` demo panel draws through the engine UI pass (F6 toggles, Esc closes). Everything else is still planned.

## Module map

```
src/                         pure logic, no mc/ or ll/ includes, unit-tested
  bul/Geometry.h             Rect, Point, half-open containment, insets, centring
  bul/Color.h                Rgba with fromRgb / fromArgb / withAlpha
  bul/TextMetrics.h          measurement interface + UTF-8 safe truncateToWidth
  bul/Version.h              version and ABI macros
  bul/Layout.*               row/column/scroll/grid/table solving, unit tested
  bul/HitTest.*              PLANNED: point -> widget, z-order aware
  bul/Focus.*                PLANNED: focus order, tab/arrow transitions, modal scope
  bul/Theme.*                PLANNED: theme tokens + neutral defaults + overrides
  bul/Binding.*              PLANNED: value providers and dirty-refresh bookkeeping

src-client/                  engine adapters, no unit tests, verified in game
  mod/                       LeviLamina entry, i18n load, service lifecycle
  bul/gfx/Painter.*          immediate-mode UI pass drawing (fill/outline/text)
  bul/gfx/TextRenderer.*     PLANNED: engine TextMetrics implementation
  bul/UiRoot.*               PLANNED: layer stack, per-frame update and draw
  bul/ScreenHost.*           PLANNED: event wiring and layer hosting
  bul/InputRouter.*          PLANNED: pointer/key routing, swallow policy
  bul/Cursor.*               PLANNED: software cursor policy and bookkeeping
  bul/Screens.*              PLANNED: registration API and single-install guard

src-demo/                    --tests=y only, never shipped
  bul/demo/DemoPanel.*       feasibility panel (AfterUIRenderEvent listener)
```

## Frame model

1. Opening a screen pushes a **native scene** on the client scene stack (`SceneFactory::createCommonDialogInfoScreen` -> `ISceneStack::pushScreen`). The engine then owns input, the cursor and Esc/back. Closing is `schedulePopScreen(1)`.
2. Render pass callback (engine) -> `ScreenHost` -> `UiRoot::draw(Painter&)`, only while our scene is on top.
3. `InputRouter` receives pointer/key events. It decides synchronously — from the previous frame's hit tests — whether to swallow, and queues the real handling. Only our own controls consume events; the modal scene already keeps the world and the screen below from seeing input.
4. On the client thread, once per frame, `UiRoot::update()` runs the queued events, focus changes and per-frame callbacks.
5. The same update runs the watchdog. It must not live in a closable screen's draw path: a watchdog that stops when the UI closes cannot clean up a cursor, a hook or a captured input, and the symptom looks like "the mod broke the mouse".

## Public API (planned, not implemented yet)

```cpp
#include "bul/BedrockUILib.h"

auto& ui = bul::service();                 // idempotent, safe to call twice

bul::Theme theme = bul::Theme::neutral();  // placeholder defaults, fully overridable
theme.color.text = 0xE0E0E0;
ui.setTheme(theme);

ui.registerScreen("my_mod.settings", [] (bul::Screen& screen) {
    screen.row([&] {
        screen.label("Title");
        screen.button("Apply", [] { /* ... */ });
    });
});

ui.push("my_mod.settings");                // Esc pops it, Tab moves focus
ui.hudElement("my_mod.hud", [] (bul::Hud& hud) { hud.text("..."); });
```

Shapes may change once the engine-side pieces exist.

## Test plan

Unit tests live in `src-test/common` (no game):

- layout solving: sizes, alignment, wrapping, scrolling, nested rows and columns;
- hit tests with z-order, clipping and disabled widgets;
- focus order and keyboard routing (Tab, Shift+Tab, arrows, Enter, Esc), modal scopes, focus restoration on close;
- text measurement and truncation through `TextMetrics` (fake implementation with fixed advances);
- theme token resolution and fallback to neutral defaults;
- binding refresh: dirty propagation, no redraw when nothing changed;
- version and ABI macros.

Hand-verified in game, documented per step:

- the panel draws inside the game's UI pass and follows the GUI-scale setting;
- pointer and keyboard behave; the cursor appears and disappears exactly once;
- Chinese text renders through the engine font;
- closing the UI returns input to the game, no leftovers.
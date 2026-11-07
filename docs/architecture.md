# Architecture

Status: **skeleton + feasibility demo**. The module map and the public API below
are the plan; `Geometry`, `Color`, `TextMetrics` and `gfx::Painter` exist, and a
`--tests=y` demo panel draws through the engine UI pass (F6 toggles, Esc closes).
Everything else is still planned.

## Decision

On 2026-11-03 the implementation route was decided: **the library draws its own
widgets inside the game's UI pass** through `MinecraftUIRenderContext`, and owns
layout, hit-testing, focus and input routing. The alternative — JSON UI
definitions in a resource pack plus a `ScreenController` — was fully investigated
and rejected by the maintainer, although it is the only route that gives
engine-managed controls.

Consequences accepted with the decision (see
[investigation-native-ui.md](investigation-native-ui.md) for the evidence):

- The library, not the engine, owns pointer/keyboard routing, focus order, the
  cursor and text input. The engine still owns the render pass and the font.
- No resource pack, no JSON UI schema, no pack-load timing: everything is C++ and
  ships in the dll.
- Look and feel are ours: the vanilla-ish appearance has to be reproduced from
  textures and metrics, not inherited from engine controls.

## Module map

```
src/                         pure logic, no mc/ or ll/ includes, unit-tested
  bul/Geometry.h             Rect, Point, half-open containment, insets, centring
  bul/Color.h                Rgba with fromRgb / fromArgb / withAlpha
  bul/TextMetrics.h          measurement interface + UTF-8 safe truncateToWidth
  bul/Version.h              version and ABI macros
  bul/Layout.*               PLANNED: row/column/scroll/grid/table solving
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

`src/` must stay free of `mc/` and `ll/` includes so the test target can compile
and run it without a game.

## Frame model

1. A screen is opened by pushing a **native scene** on the client scene stack
   (`SceneFactory::createCommonDialogInfoScreen` -> `ISceneStack::pushScreen`).
   The engine then owns input, the cursor and Esc/back for us; this is what
   Lamium does (`src/ui/SettingsScreen.cpp`: "This native information screen
   supplies focus/cursor ownership"). Closing is `schedulePopScreen(1)`.
2. Render pass callback (engine) -> `ScreenHost` -> `UiRoot::draw(Painter&)`,
   only while our scene is the one on top.
3. `InputRouter` receives pointer/key events; it decides synchronously whether to
   swallow the event from the previous frame's hit tests and queues the real
   handling. Only our own controls consume events — the modal scene already
   keeps the world and the screen below from seeing input.
4. On the client thread, once per frame, `UiRoot::update()` runs the queued
   events, focus changes and per-frame callbacks.
5. The same update runs the watchdog. It must not live in a closable screen's
   draw path: a watchdog that stops running when the UI closes cannot clean up a
   cursor, a hook or a captured input, and the symptom then looks like "the mod
   broke the mouse".

## Hard constraints (verified on this client, do not re-learn)

- **Exclusive fullscreen does not composite the hardware cursor.** `GetCursorInfo`
  reports `CURSOR_SHOWING` and a valid handle while the player sees nothing, so a
  UI that needs a pointer must draw a software cursor itself.
- **The GDK client never produces IME composition.** `GCS_COMPSTR` stays empty and
  `WM_IME_*` never arrives, so a self-made IME cannot work. Text entry must use
  the game's own text field, or clipboard paste.
- **`ShowCursor` is a global counter** and the game pushes it negative every
  frame; showing a cursor means calling it until the count is non-negative and
  accounting for exactly the calls made.
- **Never steal foreground focus** (`SetForegroundWindow` / `AttachThreadInput`):
  this client fights back, and the workaround chain is what broke input before.

## Planned public API (to be confirmed when implemented)

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

Exact shapes are open until the engine-side pieces exist; nothing here is
implemented yet.

## Test plan

Unit-tested in `src-test/common` (no game):

- layout solving (sizes, alignment, wrapping, scrolling), including nested rows
  and columns;
- hit tests with z-order, clipping and disabled widgets;
- focus order and keyboard routing (Tab, Shift+Tab, arrows, Enter, Esc), modal
  scopes, focus restoration on close;
- text measurement and truncation through the `TextMetrics` interface (fake
  implementation with fixed advances);
- theme token resolution and fallback to neutral defaults;
- binding refresh: dirty propagation, no redraw when nothing changed;
- version and ABI macros.

Verified by hand in game (documented per step):

- the panel is drawn inside the game's UI pass, follows the GUI-scale setting;
- pointer and keyboard behave, the cursor appears and disappears exactly once;
- Chinese text renders through the engine font;
- closing the UI returns input to the game, with no leftovers.

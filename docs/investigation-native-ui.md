# Native UI investigation

Status: **audit record + decision, plus outcome**. The maintainer chose the
self-draw route on 2026-11-03, and the route is now implemented as a modal host
over a placeholder screen. [architecture.md](architecture.md) describes the
implementation. The evidence below keeps both routes on record so the decision
can be re-examined instead of re-investigated, and the tables carry an outcome
note where the implementation diverged from the plan.

Scope: how a client-only LeviLamina mod can create a *native* screen / panel / HUD element on
this GDK client (Minecraft Bedrock 1.26.51.01 + LeviLamina client), i.e. a UI drawn inside the
game's UI pass, not by an external renderer window.

## Verdict

The engine offers exactly one way to get its own controls, and this project does not use it.

* **Engine-native controls require JSON UI definitions.** A control tree can only be instantiated
  by the engine from a *resolved UI definition* (`UIControlFactory` + `UIResolvedDef`); a mod
  cannot `new UIControl`, so "build the engine's control tree in C++" does not exist in this SDK.
  Screens are then created by name (`SceneFactory::createUIScene`), wrapped into a scene
  (`SceneFactory::_createScreen`) and pushed on the client scene stack
  (`ISceneStack::pushScreen`), with a `ScreenController` subclass for data and events. The
  definitions ship in a resource pack inside the mod folder, which LeviLamina injects by itself.
* **That route was investigated and then rejected by the maintainer (2026-11-03).** The library
  draws its own widgets inside the game's UI pass through `MinecraftUIRenderContext` and owns
  layout, hit-testing, focus, cursor and text input. This is the route every rich client UI in
  this ecosystem uses (see "Reference projects").
* The accepted costs are listed in [architecture.md](architecture.md). The JSON UI findings below
  remain in this document as the record of the alternative, and of what the engine can do that we
  are deliberately not using.

## Evidence

### 1. Path A (programmatic control tree) is closed on this SDK

| Fact | Evidence |
| --- | --- |
| `UIControl` cannot be constructed: the default constructor is declared under "prevent constructor by default" with no `MCAPI` body | `...\levilamina-client-sdk\26.51.5\70daec...\include\mc\client\gui\controls\UIControl.h:114-117` |
| `UIControlFactory` only creates trees from `UIResolvedDef` / `UIControl const& template` | same package `...\controls\UIControlFactory.h:54-137` |
| `UIScene`'s public constructor needs a `VisualTree`, a `UIControlFactory`, a `UIMeasureStrategy`, a `LayoutManager` | `...\screens\UIScene.h:222-232` |
| `ScreenView` (the thing that renders/measures a tree) has no usable constructor | `...\screens\ScreenView.h:188-192` |

`UIResolvedDef` comes from the JSON UI repository (`IUIDefRepository`, loaded from a
`ResourcePackStack`), and that interface is read-only (`loadDefsList`, `findDef`,
`forEachControl`, ...). So every control tree in the game originates in JSON.

### 2. Path B is the supported route, and the scene layer is public API

| Fact | Evidence |
| --- | --- |
| Create a screen from a definition name + a controller | `SceneFactory::createUIScene(std::string const& screenName, std::shared_ptr<ScreenController>)`, `...\screens\SceneFactory.h:719-720` |
| Wrap a `BaseScreen` into a scene | `SceneFactory::_createScreen(std::shared_ptr<BaseScreen>)`, `SceneFactory.h:209` |
| Push / pop scenes | `ISceneStack::pushScreen(std::shared_ptr<AbstractScene>, bool flush)`, `schedulePopScreen`, `isOnSceneStack(std::string const&)`, `...\screens\interfaces\ISceneStack.h:183-261` |
| Reach the factory / stack from a mod | `ClientInstance::getSceneFactory()`, `getClientSceneStack()`, `getMainSceneStack()`, `...\mc\client\game\ClientInstance.h:882,888,896`; `ll::service::getClientInstance()`, `docs\ll\src-client\ll\api\service\TargetedBedrock.h:13` |
| Controller seam (data + input + tick) | `ScreenController::{bind, bindString, bindBool, bindFloat, bind*ForCollection, bind*ForAnyCollection, registerButtonEventHandler, tick, preFrameTick, tryExit, onOpen, onLeave}`, `...\screens\ScreenController.h:391-482`; `MinecraftScreenController(std::shared_ptr<MinecraftScreenModel>, ScreenExitBehavior, bool)`, `...\screens\controllers\MinecraftScreenController.h`; `ScreenExitBehavior`, `...\controllers\ScreenExitBehavior.h` |
| Write a value into a bound property | `UIPropertyBag::set(std::string_view, mce::Color / glm::vec2 / glm::ivec2)`, `...\controls\UIPropertyBag.h:125-129` |
| Engine-side input/focus/scroll: keyboard, mouse, gamepad, Esc, Tab order, focus containers, scroll views all exist as components | `...\controls\{InputComponent,FocusComponent,FocusContainerComponent,ScrollViewComponent,GridComponent,ButtonComponent,ToggleComponent,SliderComponent,TextEditComponent,DropdownComponent,TooltipTriggerComponent}.h` |

### 3. Mod resource packs are injected by LeviLamina itself

`docs\ll\src-client\ll\core\main_win.cpp`

* `InitPacksHook` (lines 131-164) adds the running mod's own `resource_packs` directory
  (`Mod::getResourceDir()`, defined in `docs\ll\src\ll\api\mod\Mod.cpp:48`) **and every other
  mod's directory** as `PackType::Resources` / `PackOrigin::User` directory pack sources.
* `InitPacksHook2` (lines 166-195) walks every loaded pack whose path contains the mods path
  and pushes it into the resource stack that the UI definitions are built from.

So a client mod ships `mods/<Mod>/resource_packs/<Pack>/`, and the definitions inside become
available to `IUIDefRepository` before the UI loads. Nothing else is required from the mod.

### 4. Working precedent on exactly this client (installed in the local instance)

| Mod | Pack | What it proves |
| --- | --- | --- |
| `mods/Advancements` | `resource_packs/Advancements/{ui/advancements_screen.json (25 KB), ui/advancements_toast.json, ui/hud_screen.json, ui/pause_screen.json, texts/zh_CN.lang}` | A full custom screen (`advancements.advancements_screen@common.base_screen`), a HUD "info box" (toast inserted into `hud_screen`), patch-based buttons in `pause_screen`, and Chinese label text via `texts/zh_CN.lang`. Its DLL imports `SceneFactory::createUIScene`, `SceneFactory::_createScreen`, `MinecraftScreenController::ctor`, `ScreenController::bindString` / `bindStringForAnyCollection` / `bindItemDataForAnyCollection` / `registerButtonEventHandler`, `UIPropertyBag::set`. |
| `mods/LL-ModMenu` | `resource_packs/ModMenu/ui/start_screen.json` | Minimal injection: `main_buttons_panel/stacked_rows` + `modifications`/`insert_after` adds one native button to the start screen, handled by `button.mods_button_clicked`. |
| `mods/playback` | `resource_packs/playback-ui/ui/start_screen.json` | Same pattern, third independent implementation. |

The Advancements pack is also the best available field notes on JSON UI pitfalls (Chinese
comments in `advancements_screen.json` / `advancements_toast.json`): `layer` ordering vs. item
icons, `keep_ratio`, `#size_binding_*` units, collection virtualisation.

### 5. Reference projects: what the ecosystem actually does

The five listed projects use one of two mechanisms. None of them builds engine control trees
programmatically (confirming section 1 from a second direction).

| Project | UI mechanism | Evidence |
| --- | --- | --- |
| `MiracleForest/BarelyEnoughItems` | custom draw through `MinecraftUIRenderContext` | `src/ui/gfx/Painter.h`, "thin batching wrapper over `MinecraftUIRenderContext`", strict order rects -> `flushImages` -> item icons -> text -> `flushText`; own widgets in `src/ui/widgets/*` |
| `MiracleForest/Bedstones` | same framework, plus injection into vanilla screens | `src/client/ui/{gfx/Painter, UiRoot, screen/Screens.h, widgets/{Style,Widgets,TextBox,Tooltip}}`, `src/client/input/InputRouter.h`, `src/client/inventorybutton/PanelLocator.h` (finds the vanilla 176x166 `player_inventory` panel rect and puts a button on it) |
| `MiracleForest/AlleviatedBackpacks` | same framework | `src/client/ui/{gfx/Painter, screen/BackpackScreen, screen/InventoryParts, screen/Tabs, widgets/*}`, `src/client/input/InputRouter.cpp`; client companion receives screen state over packets from its own server side |
| `amatouhake/Lamium` | same framework + world-space overlay | `src/ui/{Widgets,SettingsScreen,HudEditor,Toast,TextFit,TranslationsZhCN}`, `src/overlay/*`; `tests/` pure + `tests-native/` SDK types (documented in `AGENTS.md`) |
| `honeymirror/RenderDragonShaderLoader` | neither, RenderDragon shader/material pipeline | only relevant if we ever need to render outside the UI system; not needed for screens/HUD elements |

The shared client UI framework across the first four (`Painter` + `UiRoot` + `UiLayer` + widgets
+ `InputRouter`) is the de-facto standard here, and it is *all-or-nothing*: because the mod draws
its own pixels, it also owns

* hit-testing (`InputRouter` decides on the window thread whether to swallow an event, based on
  the previous frame's hit tests, then replays it on the client thread from
  `ClientInstance::update`),
* focus / tab order / text-focus key capture,
* cursor and GUI-scale handling,
* text input (their own `TextInput`/`TextBox` + clipboard helpers, since IME does not work here; see the project notes on paste-only input).

Self-drawing was initially excluded by the task (engine
should own input/focus/cursor), then accepted by the maintainer's decision on 2026-11-03, and it
is now the library's central engineering problem, see "Risks of the chosen route". Their code
is still the best available reference for **draw order inside the UI pass**, widget look
(`Style.h` = vanilla look), layout math and test structure.

| Borrow (mechanism-independent) | Not adopted |
| --- | --- |
| `Painter`'s strict layer order (background -> items -> foreground -> text) and the "flush once per z band" rule | their `InputRouter` and widget tree wholesale (ours is a library with a public API, theirs is per-mod) |
| `PanelLocator` idea: look up a vanilla control's rect from the last frame to place something next to it | server-authoritative companion protocol (this library is client-only) |
| repo layout: pure core (no `mc/`/`ll/` includes, unit-tested) + gtest target + in-game selftests + post-build that merges `lang/` | per-mod texture/asset pipelines |
| i18n split: mod strings in `lang/<locale>.json`, pack strings in `texts.d/<locale>/*.lang`, duplicate keys fail the build | |

### 6. DDUI (the new data-driven forms) was considered and rejected as the substrate

* `GameplayUI::DataDrivenScreenAPI::showDataDrivenScreen(screenId, formId, dataInstanceId)`
  exists client-side (`...\mc\client\gui\gameplay\api\gameplayui\DataDrivenScreenAPI.h:60`),
  and its schema is high-level (`Panel`, `FormButton`, `FormSwitch`, `FormSlider`,
  `FormDropdown`, `FormTextField`, `FormScrollView`, `ScrollableGridLayout`, ...), attractive.
* But its data path is a datastore instance id normally created by the server
  (`ll::ui::ScreenSession` is server-only: `ll\api\ui\base\ScreenSession.h:20,48-50`), and it
  is a *form* surface (modal dialog), not a general panel/HUD surface. Revisit only as a
  convenience layer for forms, if ever.

## Hooks and engine entry points of the chosen route

The JSON UI route needed none. The library owns the engine entry points, so
consumers never hook anything (two dependents hooking the same function is the
failure mode this rule prevents). What the implementation uses:

| Need | Entry point | State |
| --- | --- | --- |
| Draw inside the UI pass | a hook on `UIScene::$render`: our callback replaces the drawing of a placeholder screen (`SceneFactory::createCommonDialogInfoScreen`) | implemented |
| One pass per frame | the same render hook fires every frame while the placeholder is on top | implemented |
| Pointer, wheel and buttons | `ll::event::input::MouseInputEvent` listener: buttons are cancelled, moves pass through, the wheel arrives as a Windows delta (120 per notch) | implemented |
| Keys | `ll::event::input::KeyInputEvent` listener; the host routes Tab, Shift+Tab and Esc | implemented |
| Typed text | the client input callback behind `UIScene::handleTextChar`, plus a claim on the platform keyboard through `KeyboardManager` | implemented |
| Cursor | not hooked: the placeholder screen keeps the engine cursor | not needed |
| Close | `schedulePopScreen(1)` once the placeholder is the top of the stack | implemented |

Rules: the listeners are installed at enable and removed when the window closes.
There is no process-wide input hook of our own, so a failed teardown cannot leave
the mouse broken.

## API consequences (chosen route)

The control tree is ours, so the API is a C++ widget tree, not a JSON schema and
not a resource pack. A consumer builds a tree and hands it to `showModal`; the
library owns the placeholder screen, the render hook, the backdrop, input capture
and the queue-aware close. The tree can be written by hand, described with
`bul::spec` nodes, or built with `bul::spec::Builder`, which is the recommended
form.

Layout solving, hit testing, focus order, scrolling, text measurement and the
edit model are pure logic in `src/` and unit-tested. Drawing, input and the font
live in `src-client/`. A name per screen and a HUD registry are still planned;
see [architecture.md](architecture.md).

## Risks of the JSON UI option (recorded; not the route taken)

1. **JSON UI is undocumented and version-tied.** Names such as `common.base_screen`,
   `common.item_renderer`, `pause.pause_button_template`, `start.main_button` are engine
   internals; a game update can rename them. Mitigation: keep the library pack minimal, copy
   only patterns proven on this client, verify after each Bedrock update, and never rely on
   patching vanilla screens for the core feature set.
2. **Pack load timing.** Packs are registered before UI load; a new/changed definition needs a
   game restart. `createUIScene` for an unknown name must be treated as a runtime failure:
   validate + log + i18n error, never crash.
3. **CJK text in native labels is unproven in this exact build.** The Advancements mod ships
   `texts/zh_CN.lang` and binds those strings into labels, which is strong evidence, but it has
   to be seen on screen once. This is acceptance point #1 of the feasibility demo, because a
   failure here changes the whole product (the task says so explicitly).
4. **HUD z-order.** Advancements found that a panel with `layer` 100 hides item icons, while
   panel 1 + icon `layer` 60 works. Our HUD defaults must stay inside the known-good band, and
   the HUD data-push path is still open (see "Hooks needed").
5. **Collections are virtualised:** only the items inside the viewport are instantiated. Do not
   assume a control exists for every model row.
6. **JSON UI unit traps:** `#size_binding_x/y` are multiples of the parent size, not pixels;
   `keep_ratio` defaults to true (a stretched 1 px image collapses); `#texture` needs a default
   texture.
7. **Screen name collisions** between mods: one namespace prefix (`bul.*`) for the library,
   per-consumer prefixes for consumer screens, uniqueness validated at registration.
8. **Packaging:** `@levibuildscript/modpacker` copies only the DLL, PDB and `manifest.json`
   (`...\levibuildscript\0.6.2\...\rules\modpacker.lua:25-53`). `resource_packs/` and
   `lang/` must be copied by an `after_build` step and by the release workflow, otherwise the
   shipped DLL has no UI definitions.
9. **`tooth.json` must ship the pack** (`files`/placements) or Lip installs a DLL without its
   definitions.
10. **Custom drawing inside a JSON UI control is unproven.** `MinecraftUICustomRenderer` exists
    (`...\controls\renderers\MinecraftUICustomRenderer.h`) and vanilla controls use it
    (`inventory_item_renderer`, `hud_*_renderer`), but `UIControlFactory` exposes no public
    registration hook, so a third-party renderer name cannot be assumed. Keep Tier 1 strictly
    declarative; if a consumer needs a custom visual (graph, preview), the fallback is a HUD
    layer drawn through `MinecraftUIRenderContext`, not a control inside the panel.
11. **Two visual dialects.** Ecosystem mods look like Java/JEI (custom draw); engine JSON UI
    looks like Bedrock. A consumer expecting the former will need Tier 2 (their own JSON UI),
    which is a different skill than C++.

## Risks of the chosen route (self-draw)

1. **The cursor is ours.** Exclusive fullscreen does not composite the hardware cursor
   (`GetCursorInfo` says showing while nothing is on screen), and the game pushes the
   `ShowCursor` counter negative every frame. A pointer UI must draw a software cursor and
   account for exactly the `ShowCursor` calls it makes. Insight burned several rounds here. The implementation does not hit this: the window replaces a placeholder screen, so the engine keeps the cursor.
2. **Text input cannot be self-made.** This GDK client never produces IME composition
   (`GCS_COMPSTR` empty, `WM_IME_*` absent), so text entry must go through the game's own text
   field or clipboard paste. A text input widget is therefore the most expensive primitive in
   the set.
3. **Focus stealing is a trap.** `SetForegroundWindow` / `AttachThreadInput` chains fight the
   client and end up breaking input for the whole game.
4. **We own the swallow policy.** Every event we swallow must be released again, and a release is
   only swallowed if its press was; otherwise the world stops receiving clicks or the UI closes
   mid-drag.
5. **CJK through the engine font is still unverified for our own drawing.** The engine renders
   Chinese in vanilla text; our measurement and drawing must use the same font path
   (`MinecraftUIRenderContext` text), and the first demo checks it on screen.
6. **Vanilla fidelity is on us.** Without engine controls the "looks like the game" claim is
   reproduction work: metrics, nine-patches, hover/pressed states, focus rings.
7. **Performance is on us.** Hit tests run on the window thread against the last frame's layout;
   long lists need virtualisation, and layout must be cached, not recomputed per event.
8. **Hook lifetime.** The input/pointer hook of the library is process-wide: it must be removed
   at disable and guarded by a watchdog on an always-running path, or a failed teardown looks
   like "the mod broke the mouse" (the exact symptom recorded in the project notes). The implementation installs LL event listeners only and removes them on close.
9. **GUI scale and DPI.** All geometry is in GUI units; conversions to and from screen pixels
   must be centralised, or hit tests drift from what is drawn.

### How these risks turned out

- The cursor (1) is not ours after all: the window replaces a placeholder
  screen, so the engine keeps the cursor and its `ShowCursor` accounting.
- Focus stealing (3) was never needed.
- CJK renders through the engine font in game (5).
- Vanilla fidelity (6) is a theme plus pixel-art drawing. It is on us.
- Virtualisation (7) is still open: `ScrollView` paints every child and clips
  the result.
- The input hook (8) is a pair of LL event listeners, removed on close.
- GUI scale and DPI (9) are handled in `Painter`, in one place.

## Minimal runnable verification (the feasibility gate for step 3)

This list was written before the implementation. The demo now covers it.

1. The panel draws inside the UI pass through `MinecraftUIRenderContext`: a
   header, a caption, buttons and rows of text, one of them Chinese.
2. Checked in game on this client:
   * the window follows the GUI scale;
   * the engine cursor shows while the window is open, and input returns to the
     game when it closes;
   * hover and click work, a click outside the window does not reach the world,
     and the wheel scrolls a list;
   * Tab moves focus, Esc closes the window, typed text lands in the focused
     field, and the Chinese string renders as glyphs;
   * the scrollbar drags.
3. Not measured: frame time with the window open, and the state after disabling
   the mod while the window is open. The host installs a few LL event listeners
   and removes them on close, so there is no process-wide input hook to leak.

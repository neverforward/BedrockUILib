# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

During 0.x only source compatibility is promised. Breaking changes are listed
under `### Changed`.

## [Unreleased]

### Added

- xmake project skeleton (clang-cl, `--tests=y` gtest target), i18n files,
  client-only `tooth.json`, version header generated from `BUL_VERSION`.
- Pure model: `Geometry`, `Rgba`, `TextMetrics`, `Theme`, `TextEdit` (UTF-8 caret
  and selection).
- Widget tree: `Widget`, `Stack`, `Label`, `Button`, `Badge`, `Field`,
  `FieldLabel`, `FieldDescription`, `Input`.
- Input editing: focus and Tab order, editing keys, clipboard, IME text; the host
  claims the platform keyboard while a field has focus.
- Native file dialog for file inputs, `docs/components.md` and the gallery demo
  (F6 in `--tests=y` builds).
- Public C++ API exported for consumer mods (`BUL_API`): another mod includes the
  public headers and links `BedrockUILib.lib`.
- `Select`: a dropdown whose popup is painted above the tree by the host
  (`item`, `value` / `onValueChange`, `open` / `onOpenChange`).
- `Checkbox`: a box with an inline label (`checked` / `onCheckedChange`,
  `indeterminate`, `readOnly`).
- `Grid`: equal cells in N columns (row-major) and `Table`: cells on fixed and flexible `layout::Track` columns and rows; both are containers over the layout solver, and `Table` draws its outer border and separators by default (`border(false)` turns them off), with 4 units of cell padding inside each cell (`cellPadding(...)`, so text stays off the lines) and the separators running border to border; every row band is the same height.
- Spec nodes for every widget: `Grid`, `Table`, `FieldLabel` and `FieldDescription` join the existing `Label` / `Button` / `Badge` / `Input` / `Field` / `Stack` / `Select` / `Checkbox` / `Switch` / `RadioGroup`.
- `ScrollView`: a viewport over a taller content widget -- a `viewport(n)` height, programmatic `scrollTo` / `scrollBy` and wheel scrolling through the widget tree, and a draggable scrollbar (a press captures the pointer), a scrollbar solved by `layout::scrollThumb` and a `Canvas::pushClip` around the content (the game canvas maps it to the engine's clipping rectangle).
- `bul/Layout.h`: the layout solver -- `box` (row/column with grow, gap, padding and cross-axis align), `grid`, `table` (fixed and flexible tracks), `clampScroll` / `isVisible` / `scrollThumb` and the extent helpers; `Stack` now lays out through it.
- `StyleSheet`: CSS-like rules (`type` / `className` / `id` selectors, specificity) that colour whole components over the theme; an inline `Style` still wins.
- `Style`: per-widget colour overrides merged over the inherited `Theme` (inline style over the theme, inherited down the tree); the spec `Common` carries one.
- `bul::spec`: declarative trees (`spec::Stack`, `spec::Field`, ... built with `spec::build` / `spec::node`), sharing the `Widget` base properties.
- `Switch`: an on/off track with a thumb (`text`, `checked` / `onCheckedChange`, `readOnly`).
- `RadioGroup`: radio buttons with one choice (`add`, `value` /
  `onValueChange`, `readOnly`); arrow keys move the selection.
- `Label`: sizes, alignment, word wrap (spaces and CJK), line spacing, `maxLines` with an ellipsis, `maxWidth` and an explicit text colour.
- `Widget` base gained `fixedSize` / `minSize` / `maxSize`, the `size()` getters
  and the `onClick` / `onFocus` / `onBlur` / `onHover` handlers.
- Engine adapter (`Painter`, `GameCanvas`) and the modal host (`showModal`,
  `closeModal`, `modalVisible`, `setTheme`, `theme`).

### Changed

- `Button` gained `type`, `size` and `disabled`; `Stack` gained a stretch factor.
- The demo is a gallery: a home page routes to one page per component.
- Modal code split into widget / gfx / modal modules; `Modal.cpp` is the facade.

### Fixed

- Engine calls moved outside the host mutex; close watchdog; retired scene
  released a few frames later.
- Focus and the platform keyboard are released when the window closes.
- IME composition is applied as a delta stream (`0x08` backspace), so pinyin no
  longer piles up in front of the committed text.
- A field's width no longer depends on its value: it scrolls while focused and is
  cut with `...` otherwise.
- Weighted `Stack` children take their share of the leftover room, so columns
  cannot overflow the window.
- CJK text uses the field's own advance table: the caret stays on the glyph edge
  and the text inside the box.

### Removed

- Server variant assets.

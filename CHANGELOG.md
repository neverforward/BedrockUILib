# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

During 0.x only source compatibility is promised. Breaking changes are listed
under `### Changed`.

## [Unreleased]

### Changed

- `Style` now holds style properties only. The palette tokens (`primary`,
  `button`, `input`, `accent`, `invalid`, `textMuted`, ...) left it and live on
  the `Theme` again; `color`, `backgroundColor`, `borderColor` and
  `accentColor` are the way to reach them from a widget or a rule.
- `backgroundHover` and `mutedColor` are gone: a hover colour is written as a
  `:hover` rule (`{"Button:hover", {.backgroundColor = ...}}`), and muted text
  is the theme's `textMuted` or a `color` rule on the component that draws it.
- `Style::text` is now `Style::color`; `Style::panel` and `Style::panelBorder`
  are now `Style::backgroundColor` and `Style::borderColor` (source-breaking).
- `Theme::padding` is now `std::optional<Insets>` and means container padding,
  none by default; the modal window frame padding moved to
  `Theme::windowPadding`.
- `Widget::measure` resolves the widget's style before measuring, so a styled
  padding, font size, margin or size takes part in layout.

### Added

- A style layer on `Style`: `color`, `backgroundColor`, `borderColor`,
  `borderWidth`, `padding`, `gap`, `fontSize`, plus the box and text set:
  `margin`, `width` / `height`, `min-*` / `max-*`, `text-align` and
  `line-height`. `Stack` and `Grid` honour `padding` and `gap`; `Button`,
  `Input`, `Badge` and `Select` honour the surface, border and text properties;
  `Label` honours `text-align` and `line-height`; every widget honours `margin`
  and the size properties, and size-affecting properties change the measured
  size too.
- A fluent selector API for rules: `.type("Label")`, `.type<bul::Label>()`,
  `.className(...)`, `.id(...)`, `.any()`, the combinators `.child()`,
  `.adjacent()` and `.sibling()`, attribute selectors with every operator,
  pseudo classes through one `pseudo(name, argument)` call, and `set(Style)` to
  finish. `Widget::attr(...)` adds attributes (with `type`, `class` and `id`
  answered automatically); widgets now know their parent and children, so
  descendant, child and sibling selectors and the structural pseudo classes can
  match.
- `Selector::parse` / `parseList` read the same selectors as style text, including
  comma-separated lists.
- `Length` with the units `px`, `em`, `ex`, `%`, `cm`, `mm`, `in`, `pt` and
  `pc`, and the matching literals (`14_px`, `1.5_em`, `2_ex`, `50_pct`, `1_in`,
  `12_pt`, ...). `Style::fontSize`, `gap` and `borderWidth` take a `Length`,
  `Padding` gives `padding` one length per side, and the relative units resolve
  against the new `Theme::fontSize` (9 GUI units).
- Text and box properties: `visibility` (hidden keeps the space),
  `text-transform` (uppercase / lowercase / capitalize), `white-space` (wrap or
  not), `vertical-align` and `text-align` / `line-height` on text blocks.
- `display: none` drops a widget from the layout (`Stack` and `Grid` skip it
  entirely), `opacity` paints the subtree through an alpha-multiplying canvas,
  `border-radius` rounds surfaces and borders by drawing the corners row by row,
  and `letter-spacing` widens and spreads the glyph positions.
- The demo gallery's Style page holds it all: theme presets, inline styles and a
  stylesheet written with the fluent selectors, showing the box, text and state
  properties on real components, with Apply / Clear rules.
- Text styling through the game's own format codes: `font-weight: bold` draws the
  run as `§l...§r`, `font-style: italic` as `§o...§r`, and the new `obfuscated`
  property as `§k...§r`. A `FormatCanvas` wraps every string a styled widget or
  container paints, so components do not carry the codes.
- 31 tests for selectors, attributes, pseudo classes, lengths and the style
  properties (245 total).

### Fixed

- A rounded border no longer sticks half a pixel outside the straight sides: the
  corner arcs are sampled on a centre line of `radius - borderWidth / 2`.
- A bordered surface insets its fill by the border width, so the background no
  longer shows around the stroke at rounded corners.
- `Stack` and `Grid` paint their own `background-color`, border and radius when a
  style sets them, so a styled container looks like a box instead of being
  invisible.

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
- Spec trees reach parity with the widgets: `Stack` children take a `stretch` (`spec::weighted`, `Builder::add(w, stretch)`), `Common` carries `className` / `id`, and the nodes gained `Label::lineSpacing` / `elide`, `Input` file bits and `validator`, `Field` custom label/description nodes, `Select::open`, `ScrollView::wheelStep`; the builder gained `with(common)`, `table(configure)`, `fieldLabel` and `fieldDescription`.
- The builder chains: every call returns it, common properties (`margin`, `fixedSize`, `className`, `style`, `onClick`) apply to the widget added last, and `build()` closes a chain.
- `spec::Builder` can stand in for the widget API: every leaf and container takes a configure hook (a lambda over the real widget), `with(common)` applies the base properties, and `id(...)` + `find<T>()` return the built widget for runtime state changes.
- Widget classes own their spec mapping: every component has `apply(spec::X const&)`, and `spec::buildContent()` only constructs the widget and calls it.
- `spec::Builder`: the imperative spec-tree form (`build([](spec::Builder& ui) { ui.label(...); ui.row([&] { ui.button(...); }); })`) recommended for consumers; the struct nodes stay as the declarative alternative; Builder::add(...) and spec::factory(...) let any widget, custom included, join either form without a dedicated node.
- Spec nodes for every widget: `Grid`, `Table`, `FieldLabel` and `FieldDescription` join the existing `Label` / `Button` / `Badge` / `Input` / `Field` / `Stack` / `Select` / `Checkbox` / `Switch` / `RadioGroup`.
- `ScrollView`: a viewport over a taller content widget -- a `viewport(n)` height, programmatic `scrollTo` / `scrollBy` and wheel scrolling through the widget tree, and a draggable scrollbar (a press captures the pointer), a scrollbar solved by `layout::scrollThumb` and a `Canvas::pushClip` around the content (the game canvas maps it to the engine's clipping rectangle).
- `bul/Layout.h`: the layout solver -- `box` (row/column with grow, gap, padding and cross-axis align), `grid`, `table` (fixed and flexible tracks), `clampScroll` / `isVisible` / `scrollThumb` and the extent helpers; `Stack` now lays out through it.
- `StyleSheet`: style rules (`type` / `className` / `id` selectors, specificity) that colour whole components over the theme; an inline `Style` still wins.
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

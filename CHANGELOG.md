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

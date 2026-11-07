# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

During 0.x only source compatibility is promised. Breaking changes are listed
under `### Changed`.

## [Unreleased]

### Added

- xmake project skeleton with clang-cl warning set, `--tests=y` gtest target,
  i18n files and client-only `tooth.json`.
- Pure model in `src/`: geometry, `Rgba`, `TextMetrics`, `Theme`.
- Widget tree in `src/bul/widget/`: `Widget`, `Label`, `Button`, `Stack`.
- `bul::gfx::Painter` and `bul::gfx::GameCanvas` for engine drawing.
- Modal host in `src-client/bul/modal/`: placeholder scene, hooks, render, input.
- Public API: `showModal`, `closeModal`, `modalVisible`, `setTheme`, `theme`.
- Feasibility demo (`--tests=y` only): F6 toggles a modal window, Esc closes.
- Version generated at build time from `BUL_VERSION`; `0.0.0` otherwise.

### Changed

- Modal code split into widget / gfx / modal modules; `Modal.cpp` is the facade.

### Fixed

- Engine calls moved outside the host mutex (re-entrancy deadlock risk).
- Watchdog for a pop that never lands, so the modal cannot stay stuck closing.
- Retired placeholder scene released a few frames after the pop.
- Modality split: touches, keys and the pointer all go through the tree.
- `theme()` returns a copy instead of a reference to mutable state.

### Removed

- Server variant assets.

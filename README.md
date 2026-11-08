# BedrockUILib

Client-only library for LeviLamina that lets **other client mods** build screens,
panels and HUD elements with the game's own UI stack — the way the game draws its
own info boxes and settings panels. It is not an external overlay window and it
does not render outside the game's UI pass.

> Status: **repository skeleton**. There is no public API yet. The design
> decision behind it is in [docs/investigation-native-ui.md](docs/investigation-native-ui.md)
> and [docs/architecture.md](docs/architecture.md).

## What it is not

- Not a player-facing mod: it has no screens of its own outside a `--tests=y` demo.
- Not a server mod: `--target_type=server` fails at configure time, by design.
- Not a UI kit with a look: colours, radii, fonts and spacing are theme tokens
  supplied by the consumer, with neutral defaults that exist only as placeholders.

## Requirements

- Windows x64, Minecraft Bedrock GDK client (1.26.60.x) through LeviLauncher.
- [xmake](https://xmake.io) with clang-cl, C++20.
- LeviLamina client 26.60.x (the package is pinned in `xmake.lua`).

## Build

```bash
xmake f -y -p windows -a x64 -m release --target_type=client
xmake build BedrockUILib
```

The artifact is `bin/BedrockUILib/` (dll, pdb, manifest.json, `lang/`).
`lang/` is a runtime folder and is never inside the dll.

## Test

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILibTests
xmake run BedrockUILibTests
```

Reconfigure back to release with `--tests=n` before building the mod itself.
See [CONTRIBUTING.md](CONTRIBUTING.md) for the rules.

## Demo (`--tests=y` only)

The demo panel is never part of a release build: `src-demo/` is added to the mod
target only when tests are enabled. To try it in game:

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILib
```

Copy `bin/BedrockUILib/` into `<instance>/mods/BedrockUILib/`, launch, then:

- **F6** toggles the panel;
- **Esc** closes it.

The panel opens on a gallery home page that routes to one page per component
(Buttons, Input); each page is the reference for that component, `Back` returns
to the home page.

[docs/investigation-native-ui.md](docs/investigation-native-ui.md) under
"Minimal runnable verification".

## Versioning

0.x: source compatibility only. The version lives in `src/bul/Version.h`,
`CHANGELOG.md` and the git tag, and those three must agree.

## License

MIT (see [LICENSE](LICENSE)).

# BedrockUILib

Client-only library for LeviLamina. Other client mods use it to build screens, panels and HUD elements on the game's own UI stack — the same stack the game uses for its info boxes and settings panels. It is not an overlay window; everything renders inside the game's UI pass.

## Requirements

- Windows x64, Minecraft Bedrock GDK client (1.26.60.x) via LeviLauncher.
- [xmake](https://xmake.io) with clang-cl, C++20.
- LeviLamina client 26.60.x.

## Build

```bash
xmake f -y -p windows -a x64 -m release --target_type=client
xmake build BedrockUILib
```

Output goes to `bin/BedrockUILib/`

## Test

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILibTests
xmake run BedrockUILibTests
```

Reconfigure back to release (`--tests=n`) before building the mod itself. Rules in [CONTRIBUTING.md](CONTRIBUTING.md).

## Demo (`--tests=y` only)

The demo panel is excluded from release builds: `src-demo/` is only added to the mod target when tests are enabled. To try it in game:

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILib
```

Copy `bin/BedrockUILib/` into `<instance>/mods/BedrockUILib/`, launch, then:

- **F6** toggles the panel;
- **Esc** closes it.

## Documentation

The docs are a VitePress site in `docs/` (English at the root, Chinese under
`docs/zh/`):

```bash
cd docs
npm install
npm run dev      # local preview
npm run build    # static site in docs/.vitepress/dist/
```

## Versioning

0.x: source compatibility only.

## License

MIT (see [LICENSE](LICENSE)).
# Getting started

BedrockUILib is a client-only library for [LeviLamina](https://github.com/LiteLDev/LeviLamina).
Other client mods use it to build screens, panels and HUD elements on the game's
own UI stack — the same stack the game uses for its info boxes and settings
panels. It is not an overlay window: everything renders inside the game's UI
pass, so it follows the interface scale and sorts with the rest of the UI.

## Requirements

- Windows x64, Minecraft Bedrock GDK client (1.26.60.x) via LeviLauncher.
- [xmake](https://xmake.io) with clang-cl, C++20.
- LeviLamina client 26.60.x.

## Build

```bash
xmake f -y -p windows -a x64 -m release --target_type=client
xmake build BedrockUILib
```

The mod is written to `bin/BedrockUILib/`.

## Test

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILibTests
xmake run BedrockUILibTests
```

Reconfigure back to release (`--tests=n`) before building the mod itself.
The project rules are in
[CONTRIBUTING.md](https://github.com/neverforward/BedrockUILib/blob/main/CONTRIBUTING.md).

## Demo

The demo panel is excluded from release builds: `src-demo/` is only added to the
mod target when tests are enabled.

```bash
xmake f -y -p windows -a x64 -m debug --tests=y --target_type=client
xmake build BedrockUILib
```

Copy `bin/BedrockUILib/` into `<instance>/mods/BedrockUILib/`, launch, then:

- **F6** toggles the gallery; **Esc** closes it;
- the gallery is the documentation: the home page routes to one page per
  component, and every page is built from the public API only.

::: tip
When the game is running it locks `BedrockUILib.dll`. Close the game before
copying a new build into the instance.
:::

## Use it from another mod

The library is a DLL with an exported C++ API (`bul/Export.h`): a consumer mod
includes `src/` and `src-client/`, links `BedrockUILib.lib` next to the built
dll, and is built with the same compiler toolchain and LeviLamina SDK version,
because the C++ ABI crosses the dll boundary. See
[API export](/components/api-export) for the details.

## Documentation map

| Document | What it answers |
| --- | --- |
| [Components](/components/) | The widget kit: layout, fields, every control, the style layers and the declarative spec tree. |
| [Architecture](/architecture) | The module map, the input/cursor/text constraints and what is unit-tested versus verified in game. |
| [Native UI investigation](/investigation-native-ui) | How native UI can be built on this client, what the engine offers, and why the implementation route is the one it is. |

## License

MIT (see [LICENSE](https://github.com/neverforward/BedrockUILib/blob/main/LICENSE)).

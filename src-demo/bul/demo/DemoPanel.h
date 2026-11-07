#pragma once

namespace bul::demo {

/// Compiles only in a `--tests=y` build (the whole src-demo/ tree is added to
/// the mod target when tests are enabled). Installs the feasibility panel:
/// an AfterUIRenderEvent listener plus the F6 / Esc keys.
void install();

/// Removes the listener and the keys. Safe to call when not installed.
void uninstall();

} // namespace bul::demo

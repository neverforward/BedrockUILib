#pragma once

#include <string>
#include <string_view>

namespace bul::platform {

/// The OS clipboard as UTF-8; empty when the clipboard is unavailable, empty or
/// does not contain text.
[[nodiscard]] std::string clipboardText();

/// Replaces the OS clipboard with `utf8`. Unavailable or locked clipboards are
/// ignored: a failed copy must never take the UI down.
void setClipboardText(std::string_view utf8);

} // namespace bul::platform

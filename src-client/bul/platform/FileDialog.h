#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace bul::platform {

/// Opens the host's native file picker and returns the chosen path as UTF-8.
///
/// Returns nullopt when the user cancels or the dialog cannot be shown. The call
/// blocks the calling thread until the dialog closes; the dialog runs its own
/// message loop, so the window itself stays responsive while it is open.
[[nodiscard]] std::optional<std::string> openFileDialog(std::string_view title);

} // namespace bul::platform

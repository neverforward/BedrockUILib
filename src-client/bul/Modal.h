#pragma once

#include <memory>

#include "bul/Theme.h"
#include "bul/Widget.h"

namespace bul {

/// Shows `root` as a modal window and returns false when a window is already
/// open or the client is not ready. The library owns everything below this call:
/// the placeholder scene that provides focus/cursor ownership, the rendering
/// hook that replaces the scene's own drawing, the backdrop, input capture, and
/// the queue-aware close. The caller only builds a widget tree.
bool showModal(std::unique_ptr<Widget> root);

/// Closes the current modal window, if any.
void closeModal();

[[nodiscard]] bool modalVisible();

/// Replaces the visual tokens used by the modal host. Values are placeholders
/// until the consumer sets its own.
void                setTheme(Theme theme);
[[nodiscard]] Theme theme();

} // namespace bul

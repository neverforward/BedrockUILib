// Public entry points of the modal window. The work is split by responsibility:
// the widget tree lives in src/bul/widget/, the drawing in gfx/GameCanvas.cpp
// and modal/ModalRender.cpp, and the placeholder scene lifecycle plus engine
// hooks in modal/ModalHost.cpp. This file is only the facade.

#include "bul/Modal.h"

#include <utility>

#include "bul/modal/ModalInternal.h"

namespace bul {

bool showModal(std::unique_ptr<Widget> windowRoot) {
    {
        std::lock_guard lock{detail::mutex};
        if (detail::state != detail::Modality::Closed || detail::scene) {
            return false;
        }
        detail::hovered = nullptr; // never keep a pointer into the previous tree
        detail::root    = std::move(windowRoot);
    }
    if (!detail::openHost()) {
        std::lock_guard lock{detail::mutex};
        detail::root.reset();
        return false;
    }
    return true;
}

void closeModal() { detail::closeHost(); }

bool modalVisible() { return detail::state != detail::Modality::Closed; }

void setTheme(Theme newTheme) {
    std::lock_guard lock{detail::mutex};
    detail::theme = newTheme;
}

Theme theme() {
    std::lock_guard lock{detail::mutex};
    return detail::theme;
}

} // namespace bul

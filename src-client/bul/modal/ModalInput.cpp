#include "bul/modal/ModalInternal.h"

#include <algorithm>
#include <string>
#include <vector>

#include <windows.h>

#include "ll/api/event/EventBus.h"
#include "ll/api/event/input/KeyInputEvent.h"
#include "ll/api/event/input/MouseInputEvent.h"

#include "mc/client/game/ClientInstance.h"
#include "mc/client/gui/GuiData.h"
#include "mc/client/input/KeyboardManager.h"
#include "mc/deps/core/math/Vec2.h"
#include "mc/deps/input/MouseAction.h"

#include "bul/platform/Clipboard.h"
#include "bul/platform/FileDialog.h"

namespace bul::detail {

namespace {

// mc/deps/input/Keyboard.h
constexpr int kBackspaceKey = 8;
constexpr int kTabKey       = 9;
constexpr int kReturnKey    = 13;
constexpr int kEscapeKey    = 27;
constexpr int kEndKey       = 35;
constexpr int kHomeKey      = 36;
constexpr int kLeftKey      = 37;
constexpr int kRightKey     = 39;
constexpr int kDeleteKey    = 46;
constexpr int kKeyA         = 65;
constexpr int kKeyC         = 67;
constexpr int kKeyV         = 86;
constexpr int kKeyX         = 88;

bool isDown(int virtualKey) { return (GetKeyState(virtualKey) & 0x8000) != 0; }

/// Engine key code -> the logical key a widget understands.
[[nodiscard]] Key keyOf(int code) noexcept {
    switch (code) {
    case kBackspaceKey:
        return Key::Backspace;
    case kDeleteKey:
        return Key::Delete;
    case kLeftKey:
        return Key::Left;
    case kRightKey:
        return Key::Right;
    case kHomeKey:
        return Key::Home;
    case kEndKey:
        return Key::End;
    case kReturnKey:
        return Key::Enter;
    case kTabKey:
        return Key::Tab;
    case kEscapeKey:
        return Key::Escape;
    case kKeyA:
        return Key::A;
    default:
        return Key::None;
    }
}

/// Tab / Shift+Tab: the focus order is the tree's `collectFocusables` order.
void cycleFocus(int direction) {
    std::lock_guard lock{mutex};
    if (state != Modality::Open || !root) {
        return;
    }
    std::vector<Widget*> focusables;
    root->collectFocusables(focusables);
    if (focusables.empty()) {
        return;
    }
    auto const count = static_cast<int>(focusables.size());
    auto const it    = std::find(focusables.begin(), focusables.end(), focused);
    int const  current =
        it == focusables.end() ? (direction > 0 ? -1 : count) : static_cast<int>(it - focusables.begin());
    int const next = ((current + direction) % count + count) % count;
    setFocus(focusables[static_cast<std::size_t>(next)]);
}

/// Ctrl+C and Ctrl+X. The clipboard is platform glue, so it lives here and not in
/// the widget: the tree only exposes selection text and text insertion.
void copySelection(Widget* target, bool cut) {
    queueEdit([target, cut] {
        std::string const selected = target->selectedText();
        if (selected.empty()) {
            return;
        }
        platform::setClipboardText(selected);
        if (cut) {
            target->textInput(std::string_view{});
        }
    });
}

void routeKey(int code) {
    bool const ctrl  = isDown(VK_CONTROL);
    bool const shift = isDown(VK_SHIFT);

    if (code == kTabKey && !ctrl) {
        cycleFocus(shift ? -1 : 1);
        return;
    }

    Widget* target = nullptr;
    {
        std::lock_guard lock{mutex};
        if (state != Modality::Open) {
            return;
        }
        target = focused;
    }
    if (!target) {
        return;
    }

    if (ctrl) {
        switch (code) {
        case kKeyA:
            queueEdit([target, shift] { target->key(KeyStroke{Key::A, true, shift}); });
            return;
        case kKeyC:
            copySelection(target, false);
            return;
        case kKeyX:
            copySelection(target, true);
            return;
        case kKeyV:
            queueEdit([target] { target->textInput(platform::clipboardText()); });
            return;
        default:
            break;
        }
    }

    Key const key = keyOf(code);
    if (key == Key::None) {
        return;
    }
    queueEdit([target, stroke = KeyStroke{key, ctrl, shift}] { target->key(stroke); });
}

} // namespace

// ---------------------------------------------------------------- platform keyboard
//
// Drawing a caret is not enough: the engine only delivers typed text (and drives
// the IME) while a screen owns the platform keyboard, so the host claims it for
// the focused field and releases it again when focus leaves or the window closes.

void releaseTextKeyboard() {
    if (client == nullptr || !keyboardOwned) {
        return;
    }
    keyboardOwned  = false;
    auto& keyboard = client->getKeyboardManager();
    keyboard.disableKeyboard();
    keyboard.releaseKeyboardOwnership();
    log().debug("bul: text keyboard released");
}

void syncTextKeyboard() {
    if (client == nullptr) {
        return;
    }
    Widget* const target = focused;
    bool const    wanted = target != nullptr && target->acceptsTextInput() && state == Modality::Open;

    if (keyboardOwned && !wanted) {
        releaseTextKeyboard();
    }
    if (!wanted || keyboardOwned) {
        return;
    }

    auto& keyboard = client->getKeyboardManager();
    if (!keyboard.tryClaimKeyboardOwnership()) {
        static bool loggedClaimFailure = false;
        if (!loggedClaimFailure) {
            loggedClaimFailure = true;
            log().debug("bul: keyboard ownership is not available yet");
        }
        return;
    }
    Rect const bounds = target->bounds();
    bool const enabled =
        keyboard.tryEnableKeyboard(std::string{}, 128, true, false, false, ::Vec2{bounds.x, bounds.y}, bounds.height);
    if (!enabled) {
        keyboard.releaseKeyboardOwnership();
        log().debug("bul: the engine refused to enable the text keyboard");
        return;
    }
    keyboardOwned = true;
    log().debug("bul: text keyboard claimed at ({:.0f}, {:.0f})", bounds.x, bounds.y);
}

void installInput() {
    static ll::event::ListenerPtr keyListener;
    static ll::event::ListenerPtr mouseListener;
    if (keyListener) {
        return;
    }
    auto& bus = ll::event::EventBus::getInstance();

    keyListener = bus.emplaceListener<ll::event::input::KeyInputEvent>([](ll::event::input::KeyInputEvent& event) {
        bool const closing = state != Modality::Closed;
        if (!closing) {
            return;
        }
        event.cancel();
        if (!event.isDown()) {
            return;
        }
        if (event.keyCode() == kEscapeKey) {
            closeHost();
            return;
        }
        routeKey(event.keyCode());
    });

    mouseListener =
        bus.emplaceListener<ll::event::input::MouseInputEvent>([](ll::event::input::MouseInputEvent& event) {
            if (state == Modality::Closed) {
                return;
            }
            // Swallow buttons, but let moves through: the engine has to process them
            // to keep its pointer state (GuiData::mPointerX/Y) alive, which is what
            // hit testing reads. Cancelling moves froze the pointer at the position
            // it had before the modal opened.
            bool const isMove = event.actionButtonId() == MouseAction::ActionMove;
            if (!isMove) {
                event.cancel();
            }
            if (!client) {
                return;
            }
            Point const point = pointerInGuiUnits(*client);
            Widget*     hit   = nullptr;
            bool        click = false;
            {
                std::lock_guard lock{mutex};
                if (!root) {
                    return;
                }
                hit = root->hitTest(point);
                click =
                    event.actionButtonId() == MouseAction::ActionLeft && event.buttonData() == MouseAction::DataDown;
                if (click) {
                    // A click moves keyboard focus to a focusable control and takes
                    // it away from anything else, like a browser form does.
                    if (hit && hit->focusable() && hit->interactive()) {
                        setFocus(hit);
                    } else {
                        setFocus(nullptr);
                    }
                    log().debug(
                        "bul: click at ({:.0f}, {:.0f}) raw ({}, {}) scale {:.2f} hit={} focused={}",
                        point.x,
                        point.y,
                        client->getGuiData()->mPointerX,
                        client->getGuiData()->mPointerY,
                        client->getGuiData()->mInvGuiScale,
                        (hit && hit->interactive()) ? "widget" : "none",
                        focused != nullptr
                    );
                }
            }
            // Outside the lock: a click handler may close the modal, and closeHost()
            // takes the same mutex.
            if (!click || hit == nullptr) {
                return;
            }
            if (hit->wantsFilePicker()) {
                // Blocks this thread until the dialog closes; the dialog runs its
                // own message loop, so its own window stays responsive. The
                // widget only receives the result.
                log().debug("bul: opening the file dialog");
                if (auto const chosen = platform::openFileDialog("Choose file")) {
                    log().debug("bul: file dialog chose '{}'", *chosen);
                    hit->chooseFile(*chosen);
                } else {
                    log().debug("bul: file dialog cancelled");
                }
                return;
            }
            if (hit->interactive()) {
                hit->activateAt(point);
            }
        });
}

} // namespace bul::detail

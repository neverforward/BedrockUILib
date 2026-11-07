#include "bul/modal/ModalInternal.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/input/KeyInputEvent.h"
#include "ll/api/event/input/MouseInputEvent.h"

#include "mc/client/game/ClientInstance.h"
#include "mc/client/gui/GuiData.h"
#include "mc/deps/input/MouseAction.h"

namespace bul::detail {

namespace {

// mc/deps/input/Keyboard.h
constexpr int kEscapeKey = 27;

} // namespace

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
        if (event.isDown() && event.keyCode() == kEscapeKey) {
            closeHost();
        }
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
                    log().info(
                        "bul: click at ({:.0f}, {:.0f}) raw ({}, {}) scale {:.2f} hit={}",
                        point.x,
                        point.y,
                        client->getGuiData()->mPointerX,
                        client->getGuiData()->mPointerY,
                        client->getGuiData()->mInvGuiScale,
                        (hit && hit->interactive()) ? "widget" : "none"
                    );
                }
            }
            // Outside the lock: a click handler may close the modal, and closeHost()
            // takes the same mutex.
            if (click && hit && hit->interactive()) {
                hit->activate();
            }
        });
}

} // namespace bul::detail

// Demo window for the self-drawn route (step 3).
//
// This file shows the intended consumer experience: build a widget tree and ask
// the library for a modal window. Everything below that — the placeholder scene
// that owns focus/cursor, replacing its drawing, the backdrop, input capture and
// the queue-aware close — lives in `bul/Modal.cpp`.

#include "bul/demo/DemoPanel.h"

#include <functional>
#include <memory>
#include <string>

#include "ll/api/event/EventBus.h"
#include "ll/api/event/client/ClientJoinLevelEvent.h"
#include "ll/api/event/input/KeyInputEvent.h"
#include "ll/api/i18n/I18n.h"

#include "bul/Modal.h"
#include "bul/Theme.h"
#include "bul/Widget.h"
#include "mod/Mod.h"

using namespace ll::literals;

namespace bul::demo {

namespace {

// mc/deps/input/Keyboard.h: F6 = 117.
constexpr int kToggleKeyCode = 117;

ll::event::ListenerPtr gKeyListener;
ll::event::ListenerPtr gJoinListener;

[[nodiscard]] ll::io::Logger& log() { return BedrockUILib::getInstance().getSelf().getLogger(); }

[[nodiscard]] std::unique_ptr<Widget> makeDemoWindow() {
    auto column = std::make_unique<Stack>(Stack::Direction::Vertical);
    column->gap(8.f);
    column->add(std::make_unique<Label>(std::string{"BedrockUILib demo panel"}, 1.2f));
    auto body = std::make_unique<Label>(
        std::string{"The library draws the whole tree: backdrop, input and modality are its job."}
    );
    body->muted();
    column->add(std::move(body));

    // Button types: the surface, border and text colour are theme tokens.
    auto types = std::make_unique<Stack>(Stack::Direction::Horizontal);
    types->gap(6.f);
    auto addTyped = [&types](char const* text, Button::Type type, std::function<void()> onClick = {}) {
        auto button = std::make_unique<Button>(std::string{text}, std::move(onClick));
        button->type(type);
        types->add(std::move(button));
    };
    addTyped("default", Button::Type::Default);
    addTyped("secondary", Button::Type::Secondary);
    addTyped("destructive", Button::Type::Destructive);
    addTyped("outline", Button::Type::Outline);
    addTyped("link", Button::Type::Link, [] { closeModal(); });
    column->add(std::move(types));

    // Button sizes: only scale and padding change.
    auto sizes = std::make_unique<Stack>(Stack::Direction::Horizontal);
    sizes->gap(6.f);
    auto addSized = [&sizes](char const* text, Button::Size size) {
        auto button = std::make_unique<Button>(std::string{text});
        button->size(size);
        sizes->add(std::move(button));
    };
    addSized("xs", Button::Size::XSmall);
    addSized("sml", Button::Size::Small);
    addSized("default", Button::Size::Default);
    addSized("lg", Button::Size::Large);
    column->add(std::move(sizes));

    auto actions = std::make_unique<Stack>(Stack::Direction::Horizontal);
    actions->gap(8.f);
    actions->add(std::make_unique<Button>(std::string{"Close"}, [] { closeModal(); }));
    column->add(std::move(actions));
    return column;
}

void toggle() {
    if (modalVisible()) {
        closeModal();
        log().info("demo: modal closing");
        return;
    }
    bool const shown = showModal(makeDemoWindow());
    log().info("demo: modal shown={}", shown);
}

} // namespace

void install() {
    if (gKeyListener) {
        return;
    }
    auto& bus    = ll::event::EventBus::getInstance();
    auto& logger = BedrockUILib::getInstance().getSelf().getLogger();
    gKeyListener = bus.emplaceListener<ll::event::input::KeyInputEvent>([](ll::event::input::KeyInputEvent& event) {
        if (!event.isDown() || event.keyCode() != kToggleKeyCode) {
            return;
        }
        event.cancel();
        toggle();
    });
    // gJoinListener = bus.emplaceListener<ll::event::ClientJoinLevelEvent>(
    //     [](ll::event::ClientJoinLevelEvent&) { toggle(); }
    // );
    logger.info("demo installed: F6 toggles a modal window built from a widget tree");
}

void uninstall() {
    auto& bus = ll::event::EventBus::getInstance();
    for (auto* listener : {&gKeyListener, &gJoinListener}) {
        if (*listener) {
            bus.removeListener(*listener);
            listener->reset();
        }
    }
    closeModal();
}

} // namespace bul::demo

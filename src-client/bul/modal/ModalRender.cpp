#include "bul/modal/ModalInternal.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "ll/api/event/EventBus.h"
#include "ll/api/event/render/UIRenderEvent.h"

#include "mc/client/game/ClientInstance.h"
#include "mc/client/gui/GuiData.h"
#include "mc/client/gui/screens/ScreenView.h"
#include "mc/client/gui/screens/UIScene.h"
#include "mc/client/renderer/screen/MinecraftUIRenderContext.h"

#include "bul/gfx/GameCanvas.h"

namespace bul::detail {

namespace {

void drawModal(MinecraftUIRenderContext& context) {
    gfx::GameCanvas canvas{context};
    Rect const      screen = canvas.screen();
    canvas.fill(screen, Rgba{0.f, 0.f, 0.f, 1.f}, theme.scrim);

    // The window is the content size plus its own padding: measuring the tree
    // gives the children only, so shrinking the frame by the padding used to clip
    // the last child (the button) outside it.
    float const padding  = theme.padding;
    float const maxWidth = std::min(screen.width * 0.7f, 360.f);
    Size const  wanted   = root->measure(std::max(maxWidth - 2.f * padding, 40.f), theme, canvas);
    float const width    = std::min(wanted.width + 2.f * padding, maxWidth);
    float const height   = std::min(wanted.height + 2.f * padding, screen.height * 0.85f);
    Rect const  window   = Rect::fromCenterSize({screen.width / 2.f, screen.height / 2.f}, width, height);

    canvas.fill(window, theme.panel, 1.f);
    canvas.outline(window, theme.panelBorder, 1.f);
    root->layout(window.inset(padding, padding), theme, canvas);

    // Hover comes from the engine pointer state once per frame: that state only
    // updates while the engine still processes mouse input, so it cannot be read
    // from the event that triggered the frame.
    Point const pointer = pointerInGuiUnits(context.mClient);

    // An open popup owns the pointer: it is hovered and hit-tested before the
    // tree, so the row under the pointer highlights and the control behind it
    // cannot react.
    std::vector<Widget*> overlays;
    root->collectOverlays(overlays);
    Widget* hoverTarget = nullptr;
    for (auto it = overlays.rbegin(); it != overlays.rend(); ++it) {
        if (Widget* const claimed = (*it)->overlayHitTest(pointer)) {
            hoverTarget = claimed;
            break;
        }
    }
    if (hoverTarget == nullptr) {
        Widget* const hit = root->hitTest(pointer);
        hoverTarget       = (hit != nullptr && hit->interactive()) ? hit : nullptr;
    }
    if (hovered && hovered != hoverTarget) {
        hovered->setHovered(false);
    }
    hovered = hoverTarget;
    if (hovered) {
        hovered->setHovered(true);
        hovered->setPointer(pointer);
    }

    root->paint(canvas, theme);
    if (!overlays.empty()) {
        // Flush the tree first: the popup must cover the text drawn under it,
        // and one batch draws all fills before all text.
        canvas.flush();
        for (Widget* const overlayWidget : overlays) {
            overlayWidget->paintOverlay(canvas, theme);
        }
    }
    canvas.flush();

    if (!loggedWindow) {
        loggedWindow = true;
        log().debug(
            "bul: window {:.0f}x{:.0f} of screen {:.0f}x{:.0f} at ({:.0f}, {:.0f})",
            width,
            height,
            screen.width,
            screen.height,
            window.x,
            window.y
        );
        auto const sizes = context.mClient.getGuiData()->mScreenSizeData;
        log().debug(
            "bul: sizes total=({:.0f},{:.0f}) client=({:.0f},{:.0f}) ui=({:.0f},{:.0f}) scale={:.3f}",
            sizes->totalScreenSize->x,
            sizes->totalScreenSize->y,
            sizes->clientScreenSize->x,
            sizes->clientScreenSize->y,
            sizes->clientUIScreenSize->x,
            sizes->clientUIScreenSize->y,
            context.mClient.getGuiData()->mGuiScale
        );
    }
}

} // namespace

void renderModalFrame(MinecraftUIRenderContext& context) {
    std::vector<EditAction> edits;
    {
        std::lock_guard lock{mutex};
        if (!visible || !root) {
            return;
        }
        // The stack may still reference the scene we just popped: release our copy a
        // few frames later instead of immediately.
        if (retiredScene && ++retiredFrames > 10) {
            retiredScene.reset();
            retiredFrames = 0;
        }
        edits.swap(pendingEdits);
    }

    // Applied here, on the thread that paints, so an edit that reallocates the
    // value string can never run while the tree is being drawn. The callbacks
    // run outside the lock: one of them may close the window.
    for (auto& edit : edits) {
        edit();
    }

    std::lock_guard lock{mutex};
    if (!visible || !root) {
        return;
    }
    // The platform keyboard follows the focused text field; without it the
    // engine sends no typed text at all.
    syncTextKeyboard();
    drawModal(context);
}

void installRender() {
    static ll::event::ListenerPtr listener;
    if (listener) {
        return;
    }
    auto& bus = ll::event::EventBus::getInstance();
    listener  = bus.emplaceListener<ll::event::BeforeUIRenderEvent>([](ll::event::BeforeUIRenderEvent& event) {
        tryApplyClose();
        if (&event.screenView() != ourView()) {
            return; // only our placeholder's pass is replaced
        }
        event.cancel();
        renderModalFrame(event.uiRenderContext());
    });
}

} // namespace bul::detail

#include "bul/modal/ModalInternal.h"

#include <chrono>
#include <string>
#include <utility>

#include "ll/api/memory/Hook.h"
#include "ll/api/service/TargetedBedrock.h"

#include "mc/client/game/ClientInstance.h"
#include "mc/client/gui/screens/AbstractScene.h"
#include "mc/client/gui/screens/SceneFactory.h"
#include "mc/client/gui/screens/ScreenView.h"
#include "mc/client/gui/screens/UIScene.h"

namespace bul::detail {

namespace {

bool hooksInstalled{};

void finishCloseLocked(char const* reason) {
    if (scene) {
        retiredScene  = std::move(scene);
        retiredFrames = 0;
    }
    visible       = false;
    state         = Modality::Closed;
    popIssued     = false;
    popRetries    = 0;
    leaveAsked    = false;
    closingFrames = 0;
    hovered       = nullptr;
    log().info("bul: modal closed ({})", reason);
}

} // namespace

void finishClose(char const* reason) { finishCloseLocked(reason); }

// ---------------------------------------------------------------- engine hooks

// Everything below lives at namespace scope: the hook macro declares a static
// counter, and an unused one in an anonymous namespace is a warning.
LL_TYPE_INSTANCE_HOOK(
    BulSceneRender,
    ll::memory::HookPriority::Normal,
    ::UIScene,
    &::UIScene::$render,
    void,
    ::ScreenContext&           screenContext,
    ::FrameRenderObject const& renderObject
) {
    bool const          ours   = isOurScene(this);
    ::ScreenView* const before = ourView();
    ourView()                  = ours ? mScreenView.get() : nullptr;
    origin(screenContext, renderObject);
    ourView() = before;
}

LL_TYPE_INSTANCE_HOOK(
    BulSceneEntrance,
    ll::memory::HookPriority::Normal,
    ::UIScene,
    &::UIScene::$onScreenEntrance,
    void,
    bool isRevisiting,
    bool doScreenTransitions
) {
    origin(isRevisiting, isOurScene(this) ? false : doScreenTransitions);
}

LL_TYPE_INSTANCE_HOOK(
    BulSceneExit,
    ll::memory::HookPriority::Normal,
    ::UIScene,
    &::UIScene::$onScreenExit,
    void,
    bool                               isPopping,
    bool                               doScreenTransitions,
    ::std::shared_ptr<::AbstractScene> pushedScene
) {
    bool const ours = isOurScene(this);
    origin(isPopping, ours ? false : doScreenTransitions, std::move(pushedScene));
    if (ours && isPopping) {
        std::lock_guard lock{mutex};
        finishCloseLocked("placeholder gone");
    }
}

LL_TYPE_INSTANCE_HOOK(
    BulSceneWorldBehind,
    ll::memory::HookPriority::Normal,
    ::UIScene,
    &::UIScene::$renderGameBehind,
    bool
) {
    if (isOurScene(this)) {
        return true; // the translucent backdrop must show the world
    }
    return origin();
}

// ---------------------------------------------------------------- lifecycle

bool openHost() {
    auto instance = ll::service::getClientInstance();
    if (!instance) {
        log().warn("bul: no client instance, modal not opened");
        return false;
    }
    auto placeholder = instance->getSceneFactory().createCommonDialogInfoScreen(std::string{}, std::string{});
    if (!placeholder) {
        log().error("bul: createCommonDialogInfoScreen returned null");
        return false;
    }
    {
        std::lock_guard lock{mutex};
        client        = std::addressof(*instance);
        scene         = placeholder;
        visible       = true;
        state         = Modality::Open;
        popIssued     = false;
        popRetries    = 0;
        closingFrames = 0;
    }
    installHostHooks();
    installRender();
    installInput();
    sceneStack(*instance)->pushScreen(placeholder, false);
    log().info("bul: modal opened (active stack)");
    return true;
}

void closeHost() {
    std::lock_guard lock{mutex};
    if (state != Modality::Open) {
        return; // already closing: a second pop would take the screen underneath
    }
    state         = Modality::Closing;
    visible       = false;
    popIssued     = false;
    popRetries    = 0;
    leaveAsked    = false;
    closingFrames = 0;
    log().info("bul: close requested");
}

void tryApplyClose() {
    enum class Action { None, Leave, Pop } action = Action::None;

    std::shared_ptr<::AbstractScene> target;
    ::IClientInstance*               owner = nullptr;
    {
        std::lock_guard lock{mutex};
        if (state != Modality::Closing || !scene || !client) {
            return;
        }
        owner  = client;
        target = scene;
        ++closingFrames;

        auto const stack = sceneStack(*client);
        if (!stack->isOnSceneStack(target->getScreenName())) {
            finishCloseLocked("already off the stack");
            return;
        }
        if (popIssued) {
            // Watchdog: a pop that never lands would keep the modal in Closing
            // forever, and the hotkey could then only ever "close" it again.
            if (closingFrames > 240) {
                finishCloseLocked("pop never landed");
            }
            return;
        }
        if (stack->getTopScene() != target.get()) {
            // pushScreen/pop are queued: never pop before our scene is the top
            // one, or the screen underneath is taken down instead.
            if (leaveAsked) {
                if (closingFrames > 360) {
                    finishCloseLocked("leaveScreen did not take");
                }
                return;
            }
            if (++popRetries > 120) {
                log().warn("bul: placeholder never reached the top, asking it to leave");
                leaveAsked = true;
                action     = Action::Leave;
            } else if (popRetries % 30 == 0) {
                log().info("bul: pop waiting, top='{}' retry={}", stack->getScreenName(), popRetries);
            }
        } else {
            log().info("bul: popping placeholder (top confirmed)");
            popIssued = true;
            action    = Action::Pop;
        }
    }

    // Engine calls that can re-enter us are made outside the lock: the exit hook
    // takes the same mutex.
    if (action == Action::Leave && target) {
        target->leaveScreen();
    } else if (action == Action::Pop && owner != nullptr) {
        sceneStack(*owner)->schedulePopScreen(1);
    }
}

void installHostHooks() {
    if (hooksInstalled) {
        return;
    }
    BulSceneRender::hook();
    BulSceneEntrance::hook();
    BulSceneExit::hook();
    BulSceneWorldBehind::hook();
    hooksInstalled = true;
}

} // namespace bul::detail

#include "bul/modal/ModalInternal.h"

#include "ll/api/mod/NativeMod.h"

#include "mc/client/game/ClientInstance.h"
#include "mc/client/gui/GuiData.h"
#include "mc/client/gui/screens/AbstractScene.h"
#include "mc/client/gui/screens/SceneFactory.h"

namespace bul::detail {

std::mutex                       mutex;
std::unique_ptr<Widget>          root;
std::shared_ptr<::AbstractScene> scene;
std::shared_ptr<::AbstractScene> retiredScene;
::IClientInstance*               client{};
bool                             visible{};
Modality                         state{Modality::Closed};
bool                             popIssued{};
int                              popRetries{};
int                              closingFrames{};
bool                             leaveAsked{};
int                              retiredFrames{};
bool                             loggedWindow{};
bool                             loggedPushResult{};
Theme                            theme = Theme::neutral();
Widget*                          hovered{};

namespace {
thread_local ::ScreenView* gOurView = nullptr;
}

::ScreenView*& ourView() { return gOurView; }

ll::io::Logger& log() { return ll::mod::NativeMod::current()->getLogger(); }

Bedrock::NotNullNonOwnerPtr<::ISceneStack> sceneStack(::IClientInstance& instance) {
    // The active stack: main stack on the title screen, client stack in a world.
    // `IClientInstance::getCurrentSceneStack()` still exists in 26.60 (only the
    // SceneFactory copy left the SDK), so it covers both cases.
    return instance.getCurrentSceneStack();
}

bool isOurScene(::AbstractScene const* candidate) {
    return candidate != nullptr && (scene.get() == candidate || retiredScene.get() == candidate);
}

Point pointerInGuiUnits(::IClientInstance& instance) {
    // Engine pointer state: the value the game itself uses to place the cursor,
    // so it works for mouse, touch and gamepad-driven cursors and needs no
    // platform header.
    auto        gui   = instance.getGuiData();
    float const scale = gui->mInvGuiScale;
    return {static_cast<float>(gui->mPointerX) * scale, static_cast<float>(gui->mPointerY) * scale};
}

} // namespace bul::detail

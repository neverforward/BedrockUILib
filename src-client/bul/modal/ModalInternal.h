#pragma once

// Shared state and entry points of the modal host. Split by responsibility:
//   ModalState.cpp  - state, logging, small helpers
//   ModalHost.cpp   - placeholder scene lifecycle, engine hooks, queue-aware close
//   ModalRender.cpp - drawing the widget tree in place of the placeholder pass
//   ModalInput.cpp  - key/mouse capture and routing into the tree
//   Modal.cpp       - public API (showModal/closeModal/...), nothing else

#include <memory>
#include <mutex>

#include "bul/Theme.h"
#include "bul/Widget.h"

#include "ll/api/io/Logger.h"
#include "ll/api/service/TargetedBedrock.h"

#include "mc/client/gui/screens/interfaces/ISceneStack.h"

class IClientInstance;
class AbstractScene;
class ScreenView;
class MinecraftUIRenderContext;

namespace bul::detail {

/// Closed -> Open -> Closing -> (exit hook) Closed. Single flight: a second close
/// must not schedule a second pop, or the screen underneath is taken down too.
enum class Modality { Closed, Open, Closing };

extern std::mutex                       mutex;
extern std::unique_ptr<Widget>          root;
extern std::shared_ptr<::AbstractScene> scene;
extern std::shared_ptr<::AbstractScene> retiredScene;
extern ::IClientInstance*               client;
extern bool                             visible;
extern Modality                         state;
extern bool                             popIssued;
extern int                              popRetries;
extern int                              closingFrames;
extern bool                             leaveAsked;
extern int                              retiredFrames;
extern bool                             loggedWindow;
extern bool                             loggedPushResult;
extern Theme                            theme;
extern Widget*                          hovered;

/// The ScreenView of the pass currently rendering our placeholder, set by the
/// `UIScene::$render` hook. Thread-local because UI rendering is per thread.
[[nodiscard]] ::ScreenView*& ourView();

[[nodiscard]] ll::io::Logger&                            log();
[[nodiscard]] Bedrock::NotNullNonOwnerPtr<::ISceneStack> sceneStack(::IClientInstance& client);
[[nodiscard]] bool                                       isOurScene(::AbstractScene const* scene);
[[nodiscard]] Point                                      pointerInGuiUnits(::IClientInstance& client);

// Host lifecycle (ModalHost.cpp).
void installHostHooks();
bool openHost();
void closeHost();
void finishClose(char const* reason);
void tryApplyClose();

// Rendering (ModalRender.cpp).
void installRender();
void renderModalFrame(MinecraftUIRenderContext& context);

// Input (ModalInput.cpp).
void installInput();

} // namespace bul::detail

#pragma once

// Shared state and entry points of the modal host. Split by responsibility:
//   ModalState.cpp  - state, logging, small helpers
//   ModalHost.cpp   - placeholder scene lifecycle, engine hooks, queue-aware close
//   ModalRender.cpp - drawing the widget tree in place of the placeholder pass
//   ModalInput.cpp  - key/mouse capture and routing into the tree
//   ModalText.cpp   - text character routing (the engine's own text entry point)
//   Modal.cpp       - public API (showModal/closeModal/...), nothing else

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

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
/// Widget that owns the keyboard, or nullptr. Guarded by `mutex`.
extern Widget* focused;

/// Whether this host currently owns the platform keyboard. Guarded by `mutex`.
/// The engine only delivers typed text (and drives the IME) to a screen that
/// claimed the keyboard through `KeyboardManager`, so this is what actually
/// makes an input editable.
extern bool keyboardOwned;

/// The ScreenView of the pass currently rendering our placeholder, set by the
/// `UIScene::$render` hook. Thread-local because UI rendering is per thread.
[[nodiscard]] ::ScreenView*& ourView();

[[nodiscard]] ll::io::Logger&                            log();
[[nodiscard]] Bedrock::NotNullNonOwnerPtr<::ISceneStack> sceneStack(::IClientInstance& client);
[[nodiscard]] bool                                       isOurScene(::AbstractScene const* scene);
[[nodiscard]] Point                                      pointerInGuiUnits(::IClientInstance& client);

/// Moves keyboard focus. The caller must hold `mutex`; the previous owner is
/// told it lost focus before the new one is told it gained it.
void setFocus(Widget* target);

/// Edits produced by the input thread and applied by the render thread before
/// the next paint. Input events and UI rendering can run on different threads,
/// and the edit itself reallocates the value string, so it must never run while
/// the tree is being drawn.
using EditAction = std::function<void()>;

/// Queues an edit (`mutex` is taken here). Dropped when no window is open.
void queueEdit(EditAction action);

/// Edits waiting for the next frame. Guarded by `mutex`; drained by
/// ModalRender.cpp, cleared when the window closes.
extern std::vector<EditAction> pendingEdits;

/// Claims or releases the platform keyboard for the focused text field. Called
/// once per frame under `mutex` while our scene renders, and on close.
void syncTextKeyboard();
void releaseTextKeyboard();

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

// Text characters (ModalText.cpp). The engine has no event for typed text: the
// only interceptable entry point is the client input callback itself.
void installTextRouting();

} // namespace bul::detail

// Text character routing.
//
// LeviLamina has events for keys and mouse buttons but none for typed text. The
// engine still delivers every typed character to the scene that owns the screen
// (`AbstractScene::handleTextChar`, i.e. `UIScene::$handleTextChar`), and that is
// a virtual function of our placeholder scene, so it can be hooked reliably.
//
// The static `ClientInputCallbacks::handleTextChar` is the wrong target on this
// client: it lives in the game binary and the game calls it internally, so
// patching the import in our module never sees those calls.

#include "bul/modal/ModalInternal.h"

#include <cstddef>
#include <string>

#include "ll/api/memory/Hook.h"

#include "mc/client/gui/screens/UIScene.h"
#include "mc/deps/input/enums/FocusImpact.h"

namespace bul::detail {

namespace {

/// Hands the character to the focused widget.
///
/// Queued, not applied here: the character ends up in a std::string that may be
/// painted by another thread at this very moment.
void routeTextChar(std::string const& utf8Text) {
    Widget* target = nullptr;
    {
        std::lock_guard lock{mutex};
        if (state == Modality::Closed) {
            return;
        }
        target = focused;
    }
    if (target == nullptr || !target->acceptsTextInput()) {
        // One line is enough to tell "no focus" apart from "the hook never fired"
        // when a consumer reports that typing does nothing.
        static bool loggedUnfocused = false;
        if (!loggedUnfocused) {
            loggedUnfocused = true;
            log().debug("bul: text input arrived but no text field has focus");
        }
        return;
    }
    static int loggedChars = 0;
    if (loggedChars < 3) {
        ++loggedChars;
        log().debug("bul: text input reached the focused field: '{}' ({} bytes)", utf8Text, utf8Text.size());
    }
    std::string const character = utf8Text;
    queueEdit([target, character] { target->textInput(character); });
}

} // namespace

// Everything below lives at namespace scope: the hook macro declares a static
// counter, and an unused one in an anonymous namespace is a warning.
LL_TYPE_INSTANCE_HOOK(
    BulSceneText,
    ll::memory::HookPriority::Normal,
    ::UIScene,
    &::UIScene::$handleTextChar,
    void,
    ::std::string const& inputUtf8,
    ::FocusImpact        focusImpact
) {
    if (isOurScene(this)) {
        // The placeholder dialog has no text control: the text is ours. It is
        // swallowed even when no input has focus, so a character can never reach
        // the screen underneath.
        if (!inputUtf8.empty()) {
            routeTextChar(inputUtf8);
        }
        return;
    }
    origin(inputUtf8, focusImpact);
}

void installTextRouting() {
    static bool installed = false;
    if (installed) {
        return;
    }
    BulSceneText::hook();
    installed = true;
}

} // namespace bul::detail

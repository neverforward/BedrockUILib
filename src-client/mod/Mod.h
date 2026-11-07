#pragma once

#include "ll/api/mod/NativeMod.h"

namespace bul {

/// Mod entry point. Until the public API lands this only proves that the
/// skeleton loads, translates and unloads cleanly on the client target.
class BedrockUILib {

public:
    static BedrockUILib& getInstance();

    BedrockUILib() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace bul

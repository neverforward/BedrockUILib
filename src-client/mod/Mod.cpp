#include "mod/Mod.h"

#include "ll/api/i18n/I18n.h"
#include "ll/api/mod/RegisterHelper.h"

#include "bul/Version.h"

#ifdef BUL_DEMO
#include "bul/demo/DemoPanel.h"
#endif

// The _tr / _trl literals live in ll::literals (an inline namespace).
using namespace ll::literals;

namespace bul {

BedrockUILib& BedrockUILib::getInstance() {
    static BedrockUILib instance;
    return instance;
}

bool BedrockUILib::load() {
    getSelf().getLogger().info("BedrockUILib {} loading", versionString);
#ifdef BUL_DEMO
    // Keys must be registered before the game builds its input mappings, which
    // happens after mod load but before enable().
    demo::install();
#endif
    return true;
}

bool BedrockUILib::enable() {
    auto& logger = getSelf().getLogger();
    auto& i18n   = ll::i18n::getInstance();
    if (auto res = i18n.load(getSelf().getLangDir()); !res) {
        logger.error("i18n load failed");
        res.error().log(logger);
    }
    logger.info(
        R"(    ____           __                __   __  ________    _ __  
   / __ )___  ____/ /________  _____/ /__/ / / /  _/ /   (_) /_ 
  / __  / _ \/ __  / ___/ __ \/ ___/ //_/ / / // // /   / / __ \
 / /_/ /  __/ /_/ / /  / /_/ / /__/ ,< / /_/ // // /___/ / /_/ /
/_____/\___/\__,_/_/   \____/\___/_/|_|\____/___/_____/_/_.___/ 
                                                                )"
    );
    logger.info("{}", "bedrockuilib.enabled"_tr(BUL_VERSION_STRING));
    return true;
}

bool BedrockUILib::disable() {
#ifdef BUL_DEMO
    demo::uninstall();
#endif
    return true;
}

} // namespace bul

LL_REGISTER_MOD(bul::BedrockUILib, bul::BedrockUILib::getInstance());

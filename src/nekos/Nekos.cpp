#include "Nekos.h"
#include "NekosFS.h"
#include "NekosCoreApps.h"
#include "NekoShell.h"
namespace nekos {
    void init() {
        nekos::fs::init();
        nekos::registerCoreApps();
        nekos::Console::begin();
    }
}


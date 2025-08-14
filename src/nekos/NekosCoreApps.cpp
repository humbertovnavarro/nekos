#include "NekosCoreApps.h"
#include "NekosNetApps.h"
#include "NekosSysApps.h"
#include "NekoShell.h"
#include "NekosArgParse.h"
#include "NekosFSApps.h"

namespace nekos {
    void registerCoreApps() {
        registerSysApps();
        registerNetApps();
        registerFSApps();
    }
}

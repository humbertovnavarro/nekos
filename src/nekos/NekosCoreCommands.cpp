#include "NekosCoreCommands.h"
#include "NekosSysCommands.h"
#include "NekosConsole.h"
#include "NekosArgParse.h"
#include "NekosNetCommands.h"

namespace nekos {
    void registerCoreCommands() {
        registerSysCommands();
        #ifdef NEKOS_HAS_WIFI
        registerNetCommands();
        #endif
    }
}

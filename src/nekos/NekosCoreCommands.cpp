#include "NekosCoreCommands.h"
#include "NekosNetCommands.h"
#include "NekosSysCommands.h"
#include "NekosConsole.h"
#include "NekosArgParse.h"
#include "NekosFSCommands.h"

namespace nekos {
    void registerCoreCommands() {
        registerSysCommands();
        registerNetCommands();
        registerFSCommands();
    }
}

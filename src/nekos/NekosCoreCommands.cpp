#include "NekosCoreCommands.h"
#include "NekosNetCommands.h"
#include "NekosConsole.h"
#include "NekosArgParse.h"
#include "NekosFSCommands.h"
#include "NekosSysCommands.h"
namespace nekos {
    void registerCoreCommands() {
        registerNetCommands();
        registerFSCommands();
        registerSysCommands();
    }
}

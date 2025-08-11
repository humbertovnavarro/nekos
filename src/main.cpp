#include "Arduino.h"
#include "lib/Console.h"
#include "WiFi.h"

void setup(void) {
    Console::init(true);
    
    Console::registerCommand("help", [](const char* args){
        Console::log("Available commands:");
        int count = Console::getCommandCount();
        for (int i = 0; i < count; i++) {
            const char* cmdName = Console::getCommandName(i);
            if (cmdName) {
                Console::logf("  %s", cmdName);
            }
        }
    });

    Console::registerCommand("info", [](const char* args){
        Console::log("This is a FreeRTOS shell example.");
    });
    
    Console::registerCommand("ip", [](const char* args){
        IPAddress ip = WiFi.localIP();
        if (ip) {
            Console::logf("IP Address: %s", ip.toString().c_str());
        } else {
            Console::log("IP Address: Not connected");
        }
    });

}

void loop(void) {
    vTaskDelay(100);
    Console::init(true);
}
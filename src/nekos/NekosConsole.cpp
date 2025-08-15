#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "Arduino.h"
#include "NekosCommandRegistry.h"
#include "NekosConsole.h"
#include "FreeRTOS.h"
namespace nekos
{
    // Define static members
    CommandRegistry commands = CommandRegistry();
    char Console::_lineBuf[SHELL_INPUT_BUFFER_SIZE] = {};
    size_t Console::_lineIndex = 0;
    TaskHandle_t consoleLoopHandle;

    void consoleLoop(void *pvparams)
    {
        for (;;)
        {
            while (Serial.available() > 0)
            {
                nekos::Console::poll();
                vTaskDelay(portTICK_RATE_MS * 1);
            }
            vTaskDelay(portTICK_RATE_MS * 10);
        }
    }

    void Console::begin(unsigned long baud)
    {
        Serial.begin(baud);
        const char *banner[] = {
            "      |\\---/|",
            "      | ,_, |",
            "       \\_`_/-..----.",
            "    ___/ `   ' ,\"\"+ \\  ",
            "   (__...'   __\\    |`.___.';",
            "     (_,...'(_,.`__)/'.....+",
            "  Welcome to Nekos Console! 🐱\n'help' to list commands",
            nullptr};
        for (int i = 0; banner[i] != nullptr; ++i)
        {
            Serial.println(banner[i]);
        }
        Serial.println();
        printPrompt();
        xTaskCreate(consoleLoop, "consoleLoop", 8192, nullptr, 1, &consoleLoopHandle);
    }

    void Console::printPrompt()
    {
        Serial.printf("[%s]=> ", "🐈");
    }

    void Console::log(const char *message)
    {
        if (!message)
            return;
        Console::logf("%s\n", message);
    }

    void Console::logf(const char *fmt, ...)
    {
        if (!fmt)
            return;
        char buf[QUEUE_MSG_SIZE];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        Serial.printf(buf);
    }

    void Console::_dispatchLine(const char *line)
    {
        if (!line || line[0] == '\0')
            return;
        // Always start command output on a new line
        Serial.println();
        char buf[SHELL_INPUT_BUFFER_SIZE];
        strncpy(buf, line, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        char *saveptr = nullptr;
        char *cmd = strtok_r(buf, " ", &saveptr);
        char *args = strtok_r(nullptr, "", &saveptr);
        if (!cmd)
            return;
        Console::commands.executeCommand(cmd, args);
        printPrompt();
    }

    void Console::poll()
    {
        char c = (char)Serial.read();
        if (c == '\r')
            return;
        if (c == '\n')
        {
            if (_lineIndex > 0)
            {
                _lineBuf[_lineIndex] = '\0';
                _dispatchLine(_lineBuf);
                _lineIndex = 0;
                _lineBuf[0] = '\0';
            }
        }
        else if ((c == '\b' || c == 0x7F) && _lineIndex > 0)
        {
            _lineIndex--;
            _lineBuf[_lineIndex] = '\0';
            Serial.print("\b \b");
        }
        else if (_lineIndex < (SHELL_INPUT_BUFFER_SIZE - 1))
        {
            _lineBuf[_lineIndex++] = c;
            Serial.write(c);
        }
        else
        {
        }
    }
}

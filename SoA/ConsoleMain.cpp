#include "stdafx.h"
#include "ConsoleMain.h"

#include <Vorb/script/Environment.h>
#include <Vorb/script/REPL.h>

#include "ConsoleFuncs.h"

#define SOA_CONSOLE_COLOR_HEADER (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define SOA_CONSOLE_COLOR_PROMPT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define SOA_CONSOLE_COLOR_MAIN (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define SOA_CONSOLE_COLOR_OUTPUT (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define SOA_CONSOLE_COLOR_ERROR (FOREGROUND_RED | FOREGROUND_INTENSITY)

namespace {
    struct ConsolePrinter {
    public:
        void setColor(ui32 color) {
            if (m_lastColor != color) {
                fflush(stdout);
                fflush(stderr);
            }
            m_lastColor = color;
            SetConsoleTextAttribute(hndConsole, color);
        }
        void out(Sender, const cString msg) {
            setColor(SOA_CONSOLE_COLOR_OUTPUT);
            puts(msg);
        }
        void err(Sender, const cString msg) {
            setColor(SOA_CONSOLE_COLOR_ERROR);
            puts(msg);
        }

        HANDLE hndConsole;
    private:
        ui32 m_lastColor = 0;
    };
}

void consoleMain() {
    // Get console for manipulation
    HANDLE hndConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    ConsolePrinter printer = {};
    printer.hndConsole = hndConsole;

    // Write out that we are using the console version
    printer.setColor(SOA_CONSOLE_COLOR_HEADER);
    puts(R"(
     _________________
    /                /
   /                /
  /  SoA Console   /
 /                / ______
/                /  |v0.1|
==========================
)");

    vscript::Environment env = {};
    vscript::REPL repl(&env);
    registerFuncs(env);

    repl.onStream[VORB_REPL_STREAM_OUT] += makeDelegate(printer, &ConsolePrinter::out);
    repl.onStream[VORB_REPL_STREAM_ERR] += makeDelegate(printer, &ConsolePrinter::err);
    char buf[1024];
    
    while (true) {
        printer.setColor(SOA_CONSOLE_COLOR_PROMPT);
        printf(">>> ");
        printer.setColor(SOA_CONSOLE_COLOR_MAIN);
        std::cin.getline(buf, 1024);
        repl.invokeCommand(buf);
    }
}

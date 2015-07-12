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
        void out(Sender, const cString msg) {
            SetConsoleTextAttribute(hndConsole, SOA_CONSOLE_COLOR_OUTPUT);
            puts(msg);
        }
        void err(Sender, const cString msg) {
            SetConsoleTextAttribute(hndConsole, SOA_CONSOLE_COLOR_ERROR);
            puts(msg);
        }

        HANDLE hndConsole;
    };
}

void consoleMain() {
    // Get console for manipulation
    HANDLE hndConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Write out that we are using the console version
    SetConsoleTextAttribute(hndConsole, SOA_CONSOLE_COLOR_HEADER);
    puts(R"(
     _________________
    /                /
   /                /
  /  SoA Console   /
 /                / ______
/                /  |v0.1|
==========================
)");

    vscript::Environment env;
    vscript::REPL repl(&env);
    registerFuncs(env);

    ConsolePrinter printer;
    printer.hndConsole = hndConsole;
    repl.onStream[VORB_REPL_STREAM_OUT] += makeDelegate(printer, &ConsolePrinter::out);
    repl.onStream[VORB_REPL_STREAM_ERR] += makeDelegate(printer, &ConsolePrinter::err);
    char buf[1024];
    
    while (true) {
        SetConsoleTextAttribute(hndConsole, SOA_CONSOLE_COLOR_PROMPT);
        printf(">>> ");
        SetConsoleTextAttribute(hndConsole, SOA_CONSOLE_COLOR_MAIN);
        std::cin.getline(buf, 1024);
        repl.invokeCommand(buf);
    }
}

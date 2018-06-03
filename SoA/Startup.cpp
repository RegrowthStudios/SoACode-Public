#include "stdafx.h"
#include "Startup.h"

namespace {
    void printHelp() {
        printf(R"(

Command-line arguments:
"-a" to launch main application
"-c" to bring up the console
"-h" for this help text
"-q" to do nothing

... Press any key to exit ...
)");
    }
}

Startup startup(int argc, cString* argv) {
    // Application mode is the default
    Startup mode = Startup::HELP;
    bool shouldOutputHelp = false;

    // Check if another argument exists
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-a") == 0) {
                mode = Startup::APP;
            } else if (strcmp(argv[i], "-c") == 0) {
                mode = Startup::CONSOLE;
            } else if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-h") == 0) {
                shouldOutputHelp = true;
                break;
            } else if (strcmp(argv[i], "-q") == 0) {
                mode = Startup::EXIT;
            } else {
                printf("Unrecognized option:\n%s\n", argv[i]);
            }
        }
    }

    // Print help if they wanted it or didn't enter anything meaningful
    if (shouldOutputHelp || mode == Startup::HELP) {
        printHelp();
    }

    return mode;
}

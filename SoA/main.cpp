#define SOA_GAME
#include "stdafx.h"

#include "IOManager.h"
#include "utils.h"
#include "Errors.h"
#include <boost/filesystem/operations.hpp>

#if defined(WIN32) || defined(WIN64)
#include <SDL/SDL_syswm.h>
#endif

#include "App.h"

/// Creates the environment for IOManagers
/// @param argv: Process arguments
void initIOEnvironment(char** argv);

// Entry
int main(int argc, char **argv) {
    // Set up the IO environment
    initIOEnvironment(argv);

#if defined(WIN32) || defined(WIN64)
    // Tell windows that our priority class should be real time
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif
    // Run the game
    MainGame* mg = new App;
    mg->run();
    delete mg;
    mg = nullptr;

#if defined(WIN32) || defined(WIN64)
    // Need to free the console on windows
    FreeConsole();
#endif

    return 0;
}

void initIOEnvironment(char** argv) {
    // Make Sure The Initial Path Is Set Correctly
    boost::filesystem::path cwP =  boost::filesystem::initial_path();

    // Set The Executable Directory
    nString execDir;
    IOManager::getDirectory(argv[0], execDir);
    IOManager::setExecutableDirectory(execDir);

    // Set The Current Working Directory
    nString cwPath, cwDir;
    convertWToMBString((cwString)boost::filesystem::system_complete(cwP).c_str(), cwPath);
    IOManager::getDirectory(cwPath.c_str(), cwDir);
    IOManager::setCurrentWorkingDirectory(cwDir);

#ifdef DEBUG
    printf("Executable Directory:\n    %s\n", IOManager::getExecutableDirectory());
    printf("Current Working Directory:\n    %s\n\n\n", IOManager::getCurrentWorkingDirectory()
        ? IOManager::getCurrentWorkingDirectory()
        : "None Specified");
#endif // DEBUG
}

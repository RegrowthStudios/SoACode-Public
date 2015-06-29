#include "stdafx.h"

#ifdef OS_WINDOWS
#include <SDL/SDL_syswm.h>
#endif
#include <Vorb/Vorb.h>
#define VORB_IMPL_UI_SDL
#define VORB_IMPL_SOUND_FMOD
#define VORB_IMPL_FONT_SDL
#include <Vorb/VorbLibs.h>

#include "App.h"
#include "Startup.h"

// Entry
int main(int argc, char **argv) {
    // Initialize Vorb modules
    vorb::init(vorb::InitParam::ALL);

#ifdef OS_WINDOWS
    // Tell windows that our priority class should be above normal
    SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
#endif

    // Get the startup mode
    switch (startup(argc, argv)) {
    case Startup::APP:
        // Run the game
        { App().run(); }
        break;
    case Startup::CONSOLE:
        // Run the console
        break;
    case Startup::HELP:
        getchar();
        break;
    default:
        // Do nothing
        break;
    }

    // Dispose Vorb modules
    vorb::dispose(vorb::InitParam::ALL);

#ifdef OS_WINDOWS
    // Need to free the console on windows
    FreeConsole();
#endif

    return 0;
}

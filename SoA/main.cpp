#define SOA_GAME
#include "stdafx.h"

#include <condition_variable>
#include <mutex>
#include <thread>

#include <boost/filesystem/operations.hpp>
#include <glm/gtc/matrix_transform.hpp>
#if defined(WIN32) || defined(WIN64)
#include <SDL/SDL_syswm.h>
#endif

#include "BlockData.h"
#include "Camera.h"
#include "ChunkManager.h"
#include "Collision.h"
#include "Constants.h"
#include "FileSystem.h"
#include "Frustum.h"
#include "GameManager.h"
#include "IOManager.h"
#include "InputManager.h"
#include "Item.h"
#include "Options.h"
#include "Planet.h"
#include "Sound.h"
#include "TerrainGenerator.h"
#include "Texture2d.h"
#include "Threadpool.h"
#include "VoxelEditor.h"

#include "utils.h"

#include "App.h"

#ifndef GL_UNSIGNED_SHORT_5_6_5
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#define MAX(a,b) ((a)>(b)?(a):(b))
#include "LoadMonitor.h"

Uint32 rmask, gmask, bmask, amask;

bool graphFunctions = 0;
float speedMod = 0.0095f;
bool inGame = 0;
int counter = 0;

void CalculateFps(Uint32 frametimes[10], Uint32 &frametimelast, Uint32 &framecount, volatile float &framespersecond);

void checkTypes();

bool mtRender = 0;
thread *renderThread;
mutex rlock;
condition_variable rcond;

Planet *newPlanet;

SDL_Surface *screen = NULL;

bool hasFocus = 0;

// Creates The Environment For IO Managers
void initIOEnvironment(char** argv);

int main(int argc, char **argv) {
    checkTypes();

    initIOEnvironment(argv);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

    MainGame* mg = new App;
    mg->run();
    delete mg;
    mg = nullptr;

    //GameManager::chunkManager.FreeFrameBuffer();
    FreeConsole(); //WINDOWS ONLY
    //CleanupText2D();
    SDL_Quit();
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

bool HasSettingsChanged() {

    return 0;
}

void CalculateFps(Uint32 frametimes[10], Uint32 &frametimelast, Uint32 &framecount, volatile float &framespersecond) {

    Uint32 frametimesindex;
    Uint32 getticks;
    Uint32 count;
    Uint32 i;

    frametimesindex = framecount % 10;

    getticks = SDL_GetTicks();

    frametimes[frametimesindex] = getticks - frametimelast;

    frametimelast = getticks;

    framecount++;

    if (framecount < 10) {

        count = framecount;

    } else {

        count = 10;

    }

    framespersecond = 0;
    for (i = 0; i < count; i++) {
        framespersecond += frametimes[i];
    }

    framespersecond /= count;
    if (framespersecond > 0) {
        framespersecond = 1000.f / framespersecond;
        physSpeedFactor = MIN(maxPhysicsFps / framespersecond, 5);
    } else {
        physSpeedFactor = 1.0f;
    }
}

void checkTypes() {
    if (sizeof(float) != 4) {
        pError("Size of float is not 4. It is " + to_string(sizeof(float)));
        exit(33);
    }
    if (sizeof(double) != 8) {
        pError("Size of double is not 8. It is " + to_string(sizeof(double)));
        exit(33);
    }
    if (sizeof(f32) != 4) {
        pError("Size of f32 is not 4. It is " + to_string(sizeof(f32)));
        exit(33);
    }
    if (sizeof(f64) != 8) {
        pError("Size of f64 is not 8. It is " + to_string(sizeof(f64)));
        exit(33);
    }
    if (sizeof(i32) != 4) {
        pError("Size of i32 is not 4. It is " + to_string(sizeof(i32)));
        exit(33);
    }
    if (sizeof(i64) != 8) {
        pError("Size of i64 is not 8. It is " + to_string(sizeof(i64)));
        exit(33);
    }
    if (sizeof(f32) != 4) {
        pError("Size of f32 is not 4. It is " + to_string(sizeof(f32)));
        exit(33);
    }
    if (sizeof(i32v3) != 12) {
        pError("Size of i32v3 is not 12. It is " + to_string(sizeof(i32v3)));
        exit(33);
    }
    if (sizeof(f32v3) != 12) {
        pError("Size of f32v3 is not 12. It is " + to_string(sizeof(f32v3)));
        exit(33);
    }
    if (sizeof(i64v3) != 24) {
        pError("Size of i64v3 is not 24. It is " + to_string(sizeof(i64v3)));
        exit(33);
    }
    if (sizeof(f64v3) != 24) {
        pError("Size of f64v3 is not 24. It is " + to_string(sizeof(f64v3)));
        exit(33);
    }
    if (sizeof(i16v3) != 6) {
        pError("Size of i16v3 is not 6. It is " + to_string(sizeof(i16v3)));
        exit(33);
    }
    if (sizeof(i8v3) != 3) {
        pError("Size of i8v3 is not 3. It is " + to_string(sizeof(i8v3)));
        exit(33);
    }
}
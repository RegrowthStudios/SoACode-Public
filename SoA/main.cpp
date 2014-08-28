#define SOA_GAME
#include "stdafx.h"

#include <condition_variable>
#include <mutex>
#include <thread>

#include <boost\filesystem\operations.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <SDL/SDL_syswm.h>

#include "BlockData.h"
#include "Camera.h"
#include "ChunkManager.h"
#include "Collision.h"
#include "Constants.h"
#include "FileSystem.h"
#include "Frustum.h"
#include "GameManager.h"
#include "GameMenu.h"
#include "IOManager.h"
#include "InputManager.h"
#include "Item.h"
#include "OpenglManager.h"
#include "Options.h"
#include "Planet.h"
#include "Sound.h"
#include "TerrainGenerator.h"
#include "Texture2d.h"
#include "Threadpool.h"
#include "VoxelEditor.h"
#include "WorldEditor.h"
#include "shader.h"
#include "utils.h"

#include "App.h"

#ifndef GL_UNSIGNED_SHORT_5_6_5
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#define MAX(a,b) ((a)>(b)?(a):(b))

Uint32 rmask, gmask, bmask, amask;

bool graphFunctions = 0;
float speedMod = 0.0095f;
bool inGame = 0;
int counter = 0;

void gameLoop();
void worldEditorLoop();
void Initialize();
bool Control();
bool MainMenuControl();
void CalculateFps(Uint32 frametimes[10], Uint32 &frametimelast, Uint32 &framecount, volatile float &framespersecond);
int ProcessMessage(Message &message);
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

#ifdef NEW
    MainGame* mg = new App;
    mg->run();
    delete mg;
    mg = nullptr;
#endif // NEW

    Initialize();
    //ExtractFrustum(mainMenuCamera->FrustumProjectionMatrix, player->FrustumViewMatrix);
    bool isChanged = 1;

    currMenu = mainMenu;

    openglManager.BeginThread(gameLoop);
    openglManager.glThreadLoop();
    openglManager.EndThread();

    if (player) delete player;
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
    const cString execDir = IOManager::getDirectory(argv[0]);
    IOManager::setExecutableDirectory(execDir);
    delete [] execDir;

    // Set The Current Working Directory
    const cString cwPath = convertWToMBString((cwString)boost::filesystem::system_complete(cwP).c_str());
    const cString cwDir = IOManager::getDirectory(cwPath);
    IOManager::setCurrentWorkingDirectory(cwDir);
    delete [] cwPath;
    delete [] cwDir;

#ifdef DEBUG
    printf("Executable Directory:\n    %s\n", IOManager::getExecutableDirectory());
    printf("Current Working Directory:\n    %s\n\n\n", IOManager::getCurrentWorkingDirectory()
        ? IOManager::getCurrentWorkingDirectory()
        : "None Specified");
#endif // DEBUG
}

//this runs in a separate thread
void gameLoop() {
    GameManager::initializeSound();

    Uint32 frameCount = 0;
    Uint32 startTicks;
    Uint32 frametimes[10];
    Uint32 frametimelast;
    Message message;

    if (openglManager.WaitForMessage(GL_M_DONE).code == GL_M_QUIT) {
        cout << "ENDING";
        std::terminate();
    }


    frametimelast = SDL_GetTicks();
    while (GameManager::gameState != GameStates::EXIT) {
        startTicks = SDL_GetTicks();
        GameManager::soundEngine->SetMusicVolume(soundOptions.musicVolume / 100.0f);
        GameManager::soundEngine->SetEffectVolume(soundOptions.effectVolume / 100.0f);
         GameManager::soundEngine->update(player->headPosition, player->chunkDirection(), player->chunkUp());

        while (glToGame.try_dequeue(message)) {
            if (ProcessMessage(message)) {
                GameManager::gameState = GameStates::EXIT;
                break;
            }
        }

        //        if (graphicsOptions.needsWindowReload) RebuildWindow();

        GameManager::inputManager->update();

        glm::dvec3 camPos;
        GLuint st;

        if (GameManager::chunkManager) {
            const ChunkDiagnostics& cd = GameManager::chunkManager->getChunkDiagnostics();
            static unsigned int diagnosticsFrameCount = 0;
            if (diagnosticsFrameCount % 600 == 0) {
                printf("\nChunk Diagnostics:\nAwaiting Reuse:      %d\nIn Memory:           %d\nAllocation History:  %d\nFreed:               %d\n",
                    cd.numAwaitingReuse, cd.numCurrentlyAllocated, cd.totalAllocated, cd.totalFreed);
            }
            diagnosticsFrameCount++;
        }

        switch (GameManager::gameState) {

            case GameStates::PLAY:
                inGame = 1;

                st = SDL_GetTicks();
                GameManager::update(0.0, player->gridPosition, NULL);
                if (SDL_GetTicks() - st > 100) {
                    cout << "Ticks " << SDL_GetTicks() - st << "\n";
                }
                break;
            case GameStates::MAINMENU:
                inGame = 0;
                camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(mainMenuCamera.position(), 1.0));
                GameManager::planet->rotationUpdate();
                GameManager::updatePlanet(camPos, 10);
                break;
            case GameStates::ZOOMINGIN:
            case GameStates::ZOOMINGOUT:
                inGame = 0;
                mainMenuCamera.update();
                camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(mainMenuCamera.position(), 1.0));
                GameManager::updatePlanet(camPos, 10);
                break;
            case GameStates::WORLDEDITOR:
                worldEditorLoop(); //only return from this when we are done
                break;
        }

        CalculateFps(frametimes, frametimelast, frameCount, physicsFps);

        double ticks = (double)(SDL_GetTicks() - startTicks);

        if (1000.0 / maxPhysicsFps > ticks) {  //bound fps to 60
            GLuint t = SDL_GetTicks();
            SDL_Delay((Uint32)(1000.0f / maxPhysicsFps - ticks));
        }
    }

    if (inGame) {
        GameManager::onQuit();
    }

    if (GameManager::planet) delete GameManager::planet;
}

void worldEditorLoop() {
    Message message;
    Uint32 frameCount = 0;
    Uint32 startTicks;
    Uint32 frametimes[10];
    Uint32 frametimelast;

    while (GameManager::gameState == GameStates::WORLDEDITOR) {
        GameManager::inputManager->update();
        GameManager::soundEngine->SetMusicVolume(soundOptions.musicVolume / 100.0f);
        GameManager::soundEngine->SetEffectVolume(soundOptions.effectVolume / 100.0f);
        GameManager::soundEngine->update(player->headPosition, player->chunkDirection(), player->chunkUp());

        startTicks = SDL_GetTicks();

        while (glToGame.try_dequeue(message)) {
            if (ProcessMessage(message)) {
                GameManager::gameState = GameStates::EXIT;
                break;
            }
        }

        inGame = 0;
        glm::dvec3 camPos = glm::dvec3(glm::dvec4(mainMenuCamera.position(), 1.0));


        if (GameManager::worldEditor != NULL) GameManager::worldEditor->update();

        GameManager::planet->rotationUpdate();
        GameManager::updatePlanet(camPos, 10);

        //cout << physicsFps << endl;
        CalculateFps(frametimes, frametimelast, frameCount, physicsFps);
        //cout << (int)framespersecond << endl;
        if (1000.0f / maxPhysicsFps > (SDL_GetTicks() - startTicks)) {  //bound fps to 60
            Sleep((Uint32)(1000.0f / maxPhysicsFps - (SDL_GetTicks() - startTicks)));
        }
    }
}

void Initialize() {
    initializeOptions();

    player = new Player();
    player->isFlying = 0;
    player->setMoveSpeed(speedMod, 0.166f); //Average Human Running Speed = .12, 17 is a good speed

    cout << "Data uses " << sizeof(Chunk) / CHUNK_SIZE * 8 << " bits per voxel. " << endl;
    cout << "Vertices use " << sizeof(BlockVertex) << " bytes per vertex. " << endl;

    loadOptions();
    GameManager::inputManager = new InputManager();
    GameManager::inputManager->loadAxes();


    GameManager::gameState = GameStates::MAINMENU;

    GameManager::initializeSystems();
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

int ProcessMessage(Message &message) {
    PlaceBlocksMessage *pbm;

    switch (message.code) {
    case GL_M_QUIT:
        return 1;
    case GL_M_STATETRANSITION:
        switch (*((int*)message.data)) {
        case 10:
            gameToGl.enqueue(Message(GL_M_INITIALIZEVOXELS, NULL));
            GameManager::gameState = GameStates::ZOOMINGIN; //begin the zoom transition
            openglManager.zoomState = 0;
            break;
        case 11:
            gameToGl.enqueue(Message(GL_M_INITIALIZEVOXELS, NULL));
            openglManager.WaitForMessage(GL_M_DONE);
            GameManager::gameState = GameStates::ZOOMINGIN; //begin the zoom transition
            openglManager.zoomState = 0;
            break;
        case 12:
            GameManager::gameState = GameStates::ZOOMINGOUT;
            openglManager.zoomState = 2;
            break;
        case 13:
            //end session
            GameManager::gameState = GameStates::ZOOMINGOUT;
            openglManager.zoomState = 0;
            GameManager::endSession();
            break;
        case 14:
            GameManager::gameState = GameStates::WORLDEDITOR;
            break;
        case 15:
            GameManager::gameState = GameStates::MAINMENU;
            break;
        }
        delete message.data;
        gameToGl.enqueue(Message(GL_M_STATETRANSITION, NULL));
        break;
    case GL_M_PLACEBLOCKS:
        pbm = (PlaceBlocksMessage *)message.data;
        GameManager::voxelEditor->editVoxels(pbm->equippedItem);

        if (player->leftEquippedItem && player->leftEquippedItem->count == 0) {
            if (player->leftEquippedItem == player->rightEquippedItem) player->rightEquippedItem = NULL;
            player->removeItem(player->leftEquippedItem);
            player->leftEquippedItem = NULL;
        }

        if (player->rightEquippedItem && player->rightEquippedItem->count == 0) {
            if (player->leftEquippedItem == player->rightEquippedItem) player->leftEquippedItem = NULL;
            player->removeItem(player->rightEquippedItem);
            player->rightEquippedItem = NULL;
        }

        break;
    case GL_M_NEWPLANET:
        gameToGl.enqueue(Message(GL_M_NEWPLANET, NULL));
        gameToGl.enqueue(Message(GL_M_DONE, NULL));
        openglManager.WaitForMessage(GL_M_DONE);
        break;
    case GL_M_REBUILD_TERRAIN:
        GameManager::planet->flagTerrainForRebuild();
        break;
    }
    return 0;
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
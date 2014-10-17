#include "stdafx.h"
#include "OpenglManager.h"
#include "ChunkMesh.h"
#include "RenderTask.h"

#include <thread>
#include <mutex>
#include <condition_variable>

#include <SDL/SDL_syswm.h>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>

#include "BlockData.h"
#include "Camera.h"
#include "ChunkManager.h"
#include "ChunkRenderer.h"
#include "Collision.h"
#include "Constants.h"
#include "FileSystem.h"
#include "FrameBuffer.h"
#include "Frustum.h"
#include "GameManager.h"
#include "GameMenu.h"
#include "GraphicsDevice.h"
#include "GeometrySorter.h"
#include "InputManager.h"
#include "Inputs.h"
#include "Item.h"
#include "Options.h"
#include "Particles.h"
#include "Planet.h"
#include "Player.h"
#include "Sound.h"
#include "TerrainGenerator.h"
#include "TerrainPatch.h"
#include "Texture2d.h"
#include "Threadpool.h"
#include "VoxelEditor.h"
#include "VRayHelper.h"
#include "WorldEditor.h"

#include "utils.h"

vector <TextInfo> hudTexts;


moodycamel::ReaderWriterQueue <OMessage> gameToGl;
moodycamel::ReaderWriterQueue <OMessage> glToGame;

OpenglManager openglManager;

CinematicCamera mainMenuCamera;

GameMenu *currMenu;

bool glExit = 0;
bool getFrustum = 1;

void DrawGame();
void InitializeShaders();
void Initialize_SDL_OpenGL();
void InitializeObjects();
bool CheckGLError();
bool Control();
bool PlayerControl();
bool MainMenuControl();
bool EditorControl();
void RebuildWindow();
void rebuildFrameBuffer();
void UpdatePlayer();
void ZoomingUpdate();
void CalculateGlFps(Uint32 frametimes[10], Uint32 &frametimelast, Uint32 &framecount, float &framespersecond);
void RecursiveSortMeshList(vector <ChunkMesh*> &v, int start, int size);

OpenglManager::OpenglManager() : gameThread(NULL), frameBuffer(NULL), cameraInfo(NULL), zoomState(0)
{

}

OpenglManager::~OpenglManager()
{
    GameManager::gameState = GameStates::EXIT;
    EndThread();
}

void OpenglManager::glThreadLoop() {
    Uint32 frametimes[10];
    Uint32 frametimelast;
    Uint32 frameCount = 0;

    Initialize_SDL_OpenGL();

    initResolutions();

    InitializeShaders();
    debugRenderer = new DebugRenderer(); //

    hudTexts.resize(100);

    initInputs();

    DrawLoadingScreen("Loading Block Data...");
    //    GLuint stt = SDL_GetTicks();

    initConnectedTextures();

    if (!(fileManager.loadBlocks("Data/BlockData.ini"))) exit(123432);
    //    cout << SDL_GetTicks() - stt << endl;
    fileManager.saveBlocks("Data/test.ini");//
    //LoadBlockData(); //order is important.

    DrawLoadingScreen("Initializing Textures..."); //
    LoadTextures();

    SetBlockAvgTexColors();

    DrawLoadingScreen("Initializing Menus and Objects...");
    InitializeMenus();
    currMenu = mainMenu;

    InitializeObjects();

    //for use in pressure explosions
    Blocks[VISITED_NODE] = Blocks[NONE];
    Blocks[VISITED_NODE].ID = VISITED_NODE;
    Blocks[VISITED_NODE].name = "Visited Node";

    DrawLoadingScreen("Initializing planet...");

    GameManager::loadPlanet("Worlds/Aldrin/");

    mainMenuCamera.setPosition(glm::dvec3(0.0, 0.0, 1000000000));
    mainMenuCamera.setDirection(glm::vec3(0.0, 0.0, -1.0));
    mainMenuCamera.setRight(glm::vec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0));
    mainMenuCamera.setUp(glm::cross(mainMenuCamera.right(), mainMenuCamera.direction()));
    mainMenuCamera.setClippingPlane(1000000.0f, 30000000.0f);

    GameManager::initializePlanet(mainMenuCamera.position());

    mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, GameManager::planet->radius * 1.35), 3.0, glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0), glm::dvec3(0.0), GameManager::planet->radius, 0.0);


    inventoryMenu->InitializeInventory(player);

    //hudTexts.resize(30);
    GLuint startTicks;
    glToGame.enqueue(OMessage(GL_M_DONE, NULL));
    while (GameManager::gameState != GameStates::EXIT && !glExit){
        startTicks = SDL_GetTicks();
        GLenum err = glGetError();
        if (CheckGLError()){
            break;
        }
        if (graphicsOptions.needsFullscreenToggle){
            if (graphicsOptions.isFullscreen){
                SDL_SetWindowFullscreen(mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
            }
            else{
                SDL_SetWindowFullscreen(mainWindow, 0);
            }
            graphicsOptions.needsFullscreenToggle = 0;
            int w, h;
            SDL_GetWindowSize(mainWindow, &w, &h);
            graphicsOptions.windowWidth = w;
            graphicsOptions.windowHeight = h;
        }
        if (graphicsOptions.needsWindowReload){
            RebuildWindow();
        }
        else if (graphicsOptions.needsFboReload){
            rebuildFrameBuffer();
        }
        openglManager.ProcessMessages();
        if (GameManager::gameState == GameStates::PLAY){
            static int st = 0;
            //update mesh distances and sort them
            if (st == 6){
            //    UpdateMeshDistances();
                RecursiveSortMeshList(chunkMeshes, 0, chunkMeshes.size());
                st = 0;
            }
            else{
                st++;
            }


            UpdatePlayer();
        }
        else if (GameManager::gameState == GameStates::MAINMENU){
            mainMenuCamera.update();
        }
        else if (GameManager::gameState == GameStates::ZOOMINGIN || GameManager::gameState == GameStates::ZOOMINGOUT){
            ZoomingUpdate();
            mainMenuCamera.update();
        }
        else if (GameManager::gameState == GameStates::WORLDEDITOR){
            EditorState = E_MAIN;
            glWorldEditorLoop();
        }
        GameManager::inputManager->update();
        Control();
        

        bdt += glSpeedFactor * 0.01;

        DrawGame();

		SDL_GL_SwapWindow(mainWindow);
		
		if (graphicsOptions.maxFPS != 165 && 1000.0f / (float)graphicsOptions.maxFPS > (SDL_GetTicks() - startTicks)){  //bound fps to cap
			Sleep((Uint32)(1000.0f / (float)graphicsOptions.maxFPS - (SDL_GetTicks() - startTicks)));
		}

        CalculateGlFps(frametimes, frametimelast, frameCount, glFps);

    }

    GameManager::inputManager->saveAxes();

    for (size_t i = 0; i < Blocks.size(); i++){
        if (Blocks[i].active && !(i >= LOWWATER && i < FULLWATER || i > FULLWATER)){
            delete ObjectList[i];
        }
    }

    delete debugRenderer;

    //SDL_FreeSurface(screen);
    SDL_GL_DeleteContext(mainOpenGLContext);
    cout << "QUITTING";
    glToGame.enqueue(OMessage(GL_M_QUIT, NULL));

#ifdef _DEBUG 
    //    _CrtDumpMemoryLeaks();
#endif
}

void OpenglManager::glWorldEditorLoop()
{
    Uint32 frametimes[10];
    Uint32 frametimelast;
    Uint32 frameCount = 0;
    GLuint startTicks;
    while (GameManager::gameState == GameStates::WORLDEDITOR && EditorState != E_AEXIT && !glExit){
        startTicks = SDL_GetTicks();

        openglManager.ProcessMessages();

        bdt += glSpeedFactor * 0.01;

        mainMenuCamera.update();

        EditorControl();
        GameManager::worldEditor->glUpdate();

        DrawGame();
        currentUserInterface->Draw();
        
        SDL_GL_SwapWindow(mainWindow);

        if (graphicsOptions.maxFPS != 165 && 1000.0f / (float)graphicsOptions.maxFPS > (SDL_GetTicks() - startTicks)){  //bound fps to cap
            SDL_Delay((Uint32)(1000.0f / (float)graphicsOptions.maxFPS - (SDL_GetTicks() - startTicks)));
        }

        CalculateGlFps(frametimes, frametimelast, frameCount, glFps);

    }
    glToGame.enqueue(OMessage(GL_M_STATETRANSITION, new int(15)));
    openglManager.glWaitForMessage(GL_M_STATETRANSITION);
    GameManager::exitWorldEditor();
}

void OpenglManager::BeginThread(void (*func)(void))
{
    gameThread = new thread(func);
}

void OpenglManager::EndThread()
{
    if (gameThread && gameThread->joinable()) gameThread->join();
    gameThread = NULL;
}

void OpenglManager::endSession() {
    ChunkMesh* cm;
    for (int i = 0; i < chunkMeshes.size(); i++) {
        cm = chunkMeshes[i];
        if (cm->vboID != 0){
            glDeleteBuffers(1, &(cm->vboID));
            cm->vboID = 0;
        }
        if (cm->vaoID != 0){
            glDeleteVertexArrays(1, &(cm->vaoID));
            cm->vaoID = 0;
        }
        if (cm->transVaoID != 0){
            glDeleteVertexArrays(1, &(cm->transVaoID));
            cm->transVaoID = 0;
        }
        if (cm->transVboID == 0) {
            glDeleteBuffers(1, &(cm->transVboID));
            cm->transVboID = 0;
        }
        if (cm->transIndexID == 0) {
            glDeleteBuffers(1, &(cm->transIndexID));
            cm->transIndexID = 0;
        }
        if (cm->cutoutVaoID != 0){
            glDeleteVertexArrays(1, &(cm->cutoutVaoID));
            cm->cutoutVaoID = 0;
        }
        if (cm->cutoutVboID == 0) {
            glDeleteBuffers(1, &(cm->cutoutVboID));
            cm->cutoutVboID = 0;
        }
        if (cm->waterVaoID != 0){
            glDeleteBuffers(1, &(cm->waterVaoID));
            cm->waterVaoID = 0;
        }
        if (cm->waterVboID != 0){
            glDeleteBuffers(1, &(cm->waterVboID));
            cm->waterVboID = 0;
        }
    }
    std::vector<ChunkMesh*>().swap(chunkMeshes);
}

OMessage OpenglManager::WaitForMessage(int i)
{
    OMessage result;
    while (1){
        if (glToGame.try_dequeue(result)){
            if (result.code == i){
                return result;
            } else if (result.code == GL_M_QUIT) {
                return result;
            }
        }
    }

}

void OpenglManager::glWaitForMessage(int i)
{
    ProcessMessages(i);
}

void OpenglManager::ProcessMessages(int waitForMessage)
{
    OMessage message;
    void *d;
    TerrainMeshMessage *tmm;
    PlanetUpdateMessage *pum;
    int oldDetail;
    bool waiting = 0;
    if (waitForMessage != -1){
        waiting = 1;
    }

    do{
        while (gameToGl.try_dequeue(message)){
            if (message.code == waitForMessage){
                waiting = 0;
            }

            d = message.data;
            switch (message.code){
            case GL_M_ERROR: //error
                showMessage("ERROR: " + string((char *)message.data));
                delete[] message.data;
                glToGame.enqueue(OMessage(GL_M_DONE, NULL));
                break;
            case GL_M_TERRAINMESH:
                tmm = (TerrainMeshMessage *)d;
                UpdateTerrainMesh(tmm);
                break;
            case GL_M_REMOVETREES:
                tmm = (TerrainMeshMessage *)d;
                if (tmm->terrainBuffers->treeVboID != 0) glDeleteBuffers(1, &(tmm->terrainBuffers->treeVboID));
                tmm->terrainBuffers->treeVboID = 0;
                delete tmm;
                break;
            case GL_M_UPDATECAMERA:
                if (cameraInfo) delete cameraInfo;
                cameraInfo = (CameraInfo *)d;
                break;
            case GL_M_UPDATEPLANET:
                pum = (PlanetUpdateMessage *)d;
                GameManager::planet->rotationMatrix = pum->rotationMatrix;
                GameManager::planet->invRotationMatrix = glm::inverse(GameManager::planet->rotationMatrix);
                delete pum;
                break;
            case GL_M_NEWPLANET:
                GameManager::planet->clearMeshes();
                oldDetail = graphicsOptions.lodDetail;
                graphicsOptions.lodDetail = 3;
                delete GameManager::planet;
                GameManager::planet = NULL;
                mainMenuCamera.setPosition(glm::dvec3(0, 1900000, 0));
                
                GameManager::loadPlanet("Worlds/" + menuOptions.selectPlanetName + "/");
                GameManager::initializePlanet(mainMenuCamera.position());

                graphicsOptions.lodDetail = oldDetail;
                mainMenuCamera.setPosition(glm::dvec3(GameManager::planet->radius * 2.0, 0.0, -50000000.0));
                mainMenuCamera.zoomTo(glm::dvec3(GameManager::planet->radius * 2.0, 0.0, GameManager::planet->radius*0.75), 0.5, glm::dvec3(-1.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
                glToGame.enqueue(OMessage(GL_M_DONE, NULL));
                break;
            case GL_M_INITIALIZEVOXELS:
                player->initialize("Ben"); //What an awesome name that is
                GameManager::initializeVoxelWorld(player);
                glToGame.enqueue(OMessage(GL_M_DONE, NULL));
                break;
            case GL_M_CHUNKMESH:
                UpdateChunkMesh((ChunkMeshData *)(message.data));
                break;
            case GL_M_PARTICLEMESH:
                UpdateParticleMesh((ParticleMeshMessage *)(message.data));
                break;
            case GL_M_PHYSICSBLOCKMESH:
                UpdatePhysicsBlockMesh((PhysicsBlockMeshMessage *)(message.data));
                break;
            case GL_M_ENDSESSION:
                endSession();
                break;
            }
        }
    } while (waiting);
}

void OpenglManager::InitializeFrameBuffer()
{
    if (graphicsOptions.msaa > 0){
        glEnable(GL_MULTISAMPLE);
        frameBuffer = new FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT, graphicsOptions.screenWidth, graphicsOptions.screenHeight, graphicsOptions.msaa);
    }
    else{
        glDisable(GL_MULTISAMPLE);
        frameBuffer = new FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT, graphicsOptions.screenWidth, graphicsOptions.screenHeight);
    }
}

void OpenglManager::FreeFrameBuffer()
{
    if (frameBuffer != NULL) delete frameBuffer;
    frameBuffer = NULL;
}

void OpenglManager::BindFrameBuffer()
{
    // Render to our framebuffer
    frameBuffer->bind(graphicsOptions.msaa);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenglManager::DrawFrameBuffer()
{

    /*glBindTexture(GL_TEXTURE_2D, frameBuffer->renderedTextureID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    SDL_Surface *s = SDL_CreateRGBSurfaceFrom(data, graphicsOptions.screenWidth, graphicsOptions.screenHeight, 32, graphicsOptions.screenWidth * 4, 0, 0, 0, 0);
    SDL_SaveBMP(s, "testtest.bmp");
    */
    static bool start = 1;

    int flags = 0;
    if (graphicsOptions.motionBlur > 0){
        flags |= FB_SHADER_MOTIONBLUR;
        
        static glm::mat4 currMat;

        if (start){
            motionBlurShader.oldVP = player->chunkProjectionMatrix() * player->chunkViewMatrix();
            start = 0;
        }
        else{
            motionBlurShader.oldVP = currMat;
        }
        currMat = player->chunkProjectionMatrix() * player->chunkViewMatrix();
        motionBlurShader.newInverseVP = glm::inverse(currMat);
    }
    else{
        start = 0;
    }
    frameBuffer->draw(flags);
}

void DrawGame()
{

    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //we need to have a camerad

    //if (getFrustum){
    //    if (GameState != MAINMENU){
    //        ExtractFrustum(player->FrustumProjectionMatrix, player->FrustumViewMatrix);
    //    }
    //    isChanged = 0;
    //}

    if (GameManager::gameState == GameStates::MAINMENU || GameManager::gameState == GameStates::ZOOMINGIN || GameManager::gameState == GameStates::ZOOMINGOUT || 
       (GameManager::gameState == GameStates::WORLDEDITOR && GameManager::worldEditor && !GameManager::worldEditor->usingChunks)){
        if (openglManager.cameraInfo == NULL){
            openglManager.cameraInfo = new CameraInfo(mainMenuCamera.position(), mainMenuCamera.up(), mainMenuCamera.direction(), mainMenuCamera.viewMatrix(), mainMenuCamera.projectionMatrix());
        }
        else{
            openglManager.cameraInfo->position = mainMenuCamera.position();
            openglManager.cameraInfo->worldUp = mainMenuCamera.up();
            openglManager.cameraInfo->worldDir = mainMenuCamera.direction();
            openglManager.cameraInfo->viewMatrix = mainMenuCamera.viewMatrix();
            openglManager.cameraInfo->projectionMatrix = mainMenuCamera.projectionMatrix();    
        }

        openglManager.BindFrameBuffer();

        mainMenuCamera.setClippingPlane(1000000.0f, 30000000.0f);
        mainMenuCamera.updateProjection();
        glm::mat4 VP = mainMenuCamera.projectionMatrix() * mainMenuCamera.viewMatrix();
        if (GameManager::gameState == GameStates::WORLDEDITOR){
            GameManager::drawSpace(VP, 1);
        }
        else{
            GameManager::drawSpace(VP, 0);
        }
        //mainMenuCamera.SetNewProjectionMatrix(max(glm::length(mainMenuCamera.worldPosition) - GameManager::planet->radius, 1.0)*0.8, glm::length(mainMenuCamera.worldPosition)+30000000.0f);
        //cout << (int)closestLodDistance << " ";
        double clip = closestTerrainPatchDistance / (sqrt(1.0f + pow(tan(graphicsOptions.fov / 2.0), 2.0) * (pow((double)graphicsOptions.screenWidth / graphicsOptions.screenHeight, 2.0) + 1.0))*2.0);
        if (clip < 100) clip = 100;

        //cout << clip/planetScale << endl;

        mainMenuCamera.setClippingPlane(clip, MAX(300000000.0 / planetScale, closestTerrainPatchDistance + 10000000));
        mainMenuCamera.updateProjection();

        VP = mainMenuCamera.projectionMatrix() * mainMenuCamera.viewMatrix();

        glm::dmat4 fvm;
        
        if (GameManager::gameState == GameStates::WORLDEDITOR){
            fvm = glm::lookAt(
                glm::dvec3(0.0),           // Camera is here
                glm::dvec3(glm::dvec4(mainMenuCamera.direction(), 1.0)), // and looks here : at the same position, plus "direction"
                glm::dvec3(mainMenuCamera.up())                  // Head is up (set to 0,-1,0 to look upside-down)
                );

            ExtractFrustum(glm::dmat4(mainMenuCamera.projectionMatrix()), fvm, worldFrustum);
            GameManager::drawPlanet(mainMenuCamera.position(), VP, mainMenuCamera.viewMatrix(), 1.0, glm::vec3(1.0, 0.0, 0.0), 1000, 1);
        }
        else{
            fvm = glm::lookAt(
                glm::dvec3(0.0),           // Camera is here
                glm::dvec3(glm::dmat4(GameManager::planet->invRotationMatrix) * glm::dvec4(mainMenuCamera.direction(), 1.0)), // and looks here : at the same position, plus "direction"
                glm::dvec3(mainMenuCamera.up())                  // Head is up (set to 0,-1,0 to look upside-down)
                );

            ExtractFrustum(glm::dmat4(mainMenuCamera.projectionMatrix()), fvm, worldFrustum);
            GameManager::drawPlanet(mainMenuCamera.position(), VP, mainMenuCamera.viewMatrix(), 1.0, glm::vec3(1.0, 0.0, 0.0), 1000, 0);
        }
        glDisable(GL_DEPTH_TEST);
        openglManager.DrawFrameBuffer();

        glEnable(GL_DEPTH_TEST);
    }
    else if ((GameManager::gameState == GameStates::WORLDEDITOR && GameManager::worldEditor && GameManager::worldEditor->usingChunks)){
        glm::dmat4 fvm = glm::lookAt(
            glm::dvec3(0.0),           // Camera is here
            glm::dvec3(glm::dvec4(mainMenuCamera.direction(), 1.0)), // and looks here : at the same position, plus "direction"
            glm::dvec3(mainMenuCamera.up())                  // Head is up (set to 0,-1,0 to look upside-down)
            );

        Camera &chunkCamera = GameManager::worldEditor->getChunkCamera();

        ExtractFrustum(glm::dmat4(mainMenuCamera.projectionMatrix()), fvm, worldFrustum); 
        ExtractFrustum(glm::dmat4(chunkCamera.projectionMatrix()), glm::dmat4(chunkCamera.viewMatrix()), gridFrustum);

        openglManager.BindFrameBuffer();
        openglManager.Draw(chunkCamera, mainMenuCamera);
        openglManager.DrawFrameBuffer();
    
    }else{
        if (openglManager.cameraInfo == NULL){
            openglManager.cameraInfo = new CameraInfo(player->worldPosition, player->worldUp(), player->worldDirection(), player->worldViewMatrix(), player->worldProjectionMatrix());
        }
        else{
            openglManager.cameraInfo->position = player->worldPosition;
            openglManager.cameraInfo->worldUp = player->worldUp();
            openglManager.cameraInfo->worldDir = player->worldDirection();
            openglManager.cameraInfo->viewMatrix = player->worldViewMatrix();
            openglManager.cameraInfo->projectionMatrix = player->worldProjectionMatrix();
        }

        if (getFrustum){
            ExtractFrustum(glm::dmat4(player->worldProjectionMatrix()), glm::dmat4(player->worldViewMatrix()), worldFrustum);
            ExtractFrustum(glm::dmat4(player->chunkProjectionMatrix()), glm::dmat4(player->chunkViewMatrix()), gridFrustum);
        }
        openglManager.BindFrameBuffer();
        openglManager.Draw(player->getChunkCamera(), player->getWorldCamera());
        openglManager.DrawFrameBuffer();
    }

    //float sunSinTheta = sin(GameManager::planet->rotationTheta);
    //if (sunSinTheta < 0.1) sunSinTheta = 0.1;

    //if (player->underWater){
    //    DrawFullScreenQuad(glm::vec4(sunSinTheta, sunSinTheta, sunSinTheta, 0.5) * glm::vec4(0.0, 0.5, 1.0, 1.0));
    //}

    switch (GameManager::gameState){
    case GameStates::PLAY:
        if (currMenu){
            currMenu->Draw();
        }
        else{
            openglManager.DrawHud();
        }
        break;
    case GameStates::ZOOMINGIN:
    case GameStates::ZOOMINGOUT:
    case GameStates::MAINMENU:
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearDepth(1.0);
        if (currMenu) currMenu->Draw();
        break;
    case GameStates::PAUSE:
        if (currMenu) currMenu->Draw();
        break;
    case GameStates::INVENTORY:
        inventoryMenu->Draw();
        break;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InitializeShaders()
{
    
}

void Initialize_SDL_OpenGL()
{
    SDL_Init(SDL_INIT_EVERYTHING);
  
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    cout << "Video Driver: " << SDL_GetCurrentVideoDriver() << endl;
    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);
    cout << "Window Info - W: " << mode.w << " H: " << mode.h << " Refresh: " << mode.refresh_rate << " " << endl;
    graphicsOptions.nativeWidth = mode.w;
    graphicsOptions.nativeHeight = mode.h;


    //SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE ); 
    Uint32 wflags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if (graphicsOptions.isFullscreen) wflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    if (graphicsOptions.isBorderless) wflags |= SDL_WINDOW_BORDERLESS;
    mainWindow = SDL_CreateWindow("Seed Of Andromeda", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, graphicsOptions.screenWidth, graphicsOptions.screenHeight, wflags);

    if (mainWindow == NULL){
        exit(343);
    }

    int w, h;
    SDL_GetWindowSize(mainWindow, &w, &h);
    graphicsOptions.windowWidth = w;
    graphicsOptions.windowHeight = h;

    mainOpenGLContext = SDL_GL_CreateContext(mainWindow);
    if (mainOpenGLContext == NULL){
        pError("Could not make openGL context!");
        SDL_Quit();
    }
    SDL_GL_MakeCurrent(mainWindow, mainOpenGLContext);

    SDL_GL_SetSwapInterval((int)graphicsOptions.isVsync);

    GraphicsDevice* gd = new GraphicsDevice(mainWindow);
    gd->refreshInformation();


#if defined _WIN32 || defined _WIN64
    mainHGLRC = wglGetCurrentContext();
    if (mainHGLRC == NULL){
        pError("Call to wglGetCurrentContext failed in main thread!");
    }
#endif

    GLenum err = glewInit();
    if (err != GLEW_OK){
        pError("Glew failed to initialize. Your graphics card is probably WAY too old. Or you forgot to extract the .zip. It might be time for an upgrade :)");
        exit(133);
    }

    int vminor;
    int vmajor;
    glGetIntegerv(GL_MAJOR_VERSION, &vmajor);
    glGetIntegerv(GL_MINOR_VERSION, &vminor);
    printf("\n***        Opengl Version: %s\n", glGetString(GL_VERSION));
    printf("\n***       CPU Threads: %u\n", thread::hardware_concurrency());

    GLint max_layers;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_layers);
    printf("\n***       Max Texture ARray Layers: %d\n", max_layers);

    if (vmajor < 3){// || (vminor == 3 && vminor < 3)){
        char buffer[2048];
        sprintf(buffer, "Your graphics card driver does not support at least OpenGL 3.3. Your OpenGL version is \"%s\". The game will most likely not work.\n\nEither your graphics card drivers are not up to date, or your computer is using an integrated graphics card instead of your gaming card.\nYou should be able to switch to your main graphics card by right clicking your desktop and going to graphics properties.", glGetString(GL_VERSION));
        pError(buffer);
        //        exit(133);
    }

    if (!GLEW_VERSION_2_1){  // check that the machine supports the 2.1 API.
        pError("Machine does not support 2.1 GLEW API.");
        exit(134);
    }

    int textureunits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &textureunits);
    if (textureunits < 8){
        showMessage("Your graphics card does not support at least 8 texture units! It only supports " + to_string(textureunits));
        exit(1);
    }

    //msaa check
    int maxColorSamples, maxDepthSamples;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorSamples);
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxDepthSamples);
    graphicsOptions.maxMsaa = MAX(maxColorSamples, maxDepthSamples);
    cout << "Max MSAA: " << graphicsOptions.maxMsaa << endl;

    glClearColor(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f); //blue sky background
    glLineWidth(1);

    //Alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

   // InitializeText2D("Fonts/OrbitronBold.png", "Fonts/FontData.dat");
    InitializeTTF();
    
    DrawLoadingScreen("Initializing Framebuffer...");
    openglManager.InitializeFrameBuffer();


}

void OpenglManager::initResolutions() {
    i32 dispIndex = SDL_GetWindowDisplayIndex(mainWindow);
    i32 dispCount = SDL_GetNumDisplayModes(dispIndex);
    SDL_DisplayMode dMode;
    for (i32 dmi = 0; dmi < dispCount; dmi++) {
        SDL_GetDisplayMode(dispIndex, dmi, &dMode);
        SCREEN_RESOLUTIONS.push_back(ui32v2(dMode.w, dMode.h));
    }
    std::sort(SCREEN_RESOLUTIONS.begin(), SCREEN_RESOLUTIONS.end(), [] (const ui32v2& r1, const ui32v2& r2) {
        if (r1.x == r2.x) return r1.y > r2.y;
        else return r1.x > r2.x;
    });
    auto iter = std::unique(SCREEN_RESOLUTIONS.begin(), SCREEN_RESOLUTIONS.end());
    SCREEN_RESOLUTIONS.resize(iter - SCREEN_RESOLUTIONS.begin());
}

void InitializeObjects()
{
    for (size_t i = 1; i < Blocks.size(); i++){
        if (Blocks[i].active && !(i >= LOWWATER && i < FULLWATER || i > FULLWATER)){
            if (ObjectList[i] != NULL){
                printf("Object ID %d already taken by %s. Requested by %s.\n", i, ObjectList[i]->name.c_str(), Blocks[i].name.c_str());
                int a;
                cin >> a;
                exit(198);
            }
            ObjectList[i] = new Item(i, 1, Blocks[i].name, ITEM_BLOCK, 0, 0, 0);
            player->inventory.push_back(new Item(i, 9999999, Blocks[i].name, ITEM_BLOCK, 0, 0, 0));
        }
    }
}

bool CheckGLError(){
    GLenum err = glGetError();
    if (err != GL_NO_ERROR){
        switch (err){
        case GL_OUT_OF_MEMORY:
            pError("Out of memory! Try lowering the voxel view distance.");
            return 1;
        case GL_INVALID_ENUM:
            pError("GL_INVALID_ENUM - An unacceptable value is specified for an enumerated argument.");
            return 0;
        case GL_INVALID_VALUE:
            pError("GL_INVALID_VALUE - A numeric argument is out of range.");
            return 0;
        case GL_INVALID_OPERATION:
            pError("GL_INVALID_OPERATION - The specified operation is not allowed in the current state.");
            return 0;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            pError("The command is trying to render to or read from the framebuffer while the currently bound framebuffer is not framebuffer complete.");
            return 0;
        default:
            pError(("OpenGL ERROR (" + to_string(err) + string(") ")).c_str());
            return 0;
        }
    }
    return 0;
}

bool Control()
{
    
    if (GameManager::gameState == GameStates::PLAY){
        return PlayerControl();
    }
    else if (GameManager::gameState == GameStates::INVENTORY){
        SDL_SetRelativeMouseMode(SDL_FALSE);
        InventoryArgs args;
        int ret = inventoryMenu->Control(args);
        if (ret == -1 || ret == -5){
            GameManager::gameState = GameStates::PLAY;

            SDL_PumpEvents();
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        else if (ret > 0){
            if (args.arm == 0){
                if (player->leftEquippedItem == player->inventory[args.selected - 1]){
                    player->leftEquippedItem = NULL;
                }
                else{
                    player->leftEquippedItem = player->inventory[args.selected - 1];
                }
            }
            else{
                if (player->rightEquippedItem == player->inventory[args.selected - 1]){
                    player->rightEquippedItem = NULL;
                }
                else{
                    player->rightEquippedItem = player->inventory[args.selected - 1];
                }
            }
            inventoryMenu->UpdateInventoryList();
        }
        else if (ret < -1){
            if (ret > -9){ //-2 to -9 is category
                inventoryMenu->selectedCategory = ABS(ret) - 2;
                //cout << "A " << inventoryMenu->selectedCategory << endl;
                inventoryMenu->UpdateInventoryList();
            }
        }
    }
    else if (GameManager::gameState == GameStates::MAINMENU){
        if (MainMenuControl() == 0) return 0;
    }
    else if (GameManager::gameState == GameStates::PAUSE){
        int oldVoxelRenderDistance = graphicsOptions.voxelRenderDistance;
        isMouseIn = 0;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        int ret = currMenu->Control();
        switch (ret){
        case -5:
        case 1:
            GameManager::gameState = GameStates::PLAY;
            GameManager::saveState();
            SDL_PumpEvents();
            isMouseIn = 1;
            SDL_SetRelativeMouseMode(SDL_TRUE);
            currMenu = NULL;
            break;
        case 4: // texture packs
            currMenu = texturePackMenu;
            currMenu->FlushControlStates();
            break;
        case 5: // game options
            currMenu = gameOptionsMenu;
            currMenu->FlushControlStates();
            break;
        case 6: // video options
            currMenu = videoOptionsMenu;
            currMenu->FlushControlStates();
            break;
        case 7: // audio options
            currMenu = audioOptionsMenu;
            currMenu->FlushControlStates();
            break;
        case 8: //controls
            currMenu = controlsMenu;
            currMenu->FlushControlStates();
            break;
        case 10:
            currMenu = pauseMenu;
            currMenu->FlushControlStates();
            break;
        case -1:
            glToGame.enqueue(OMessage(GL_M_STATETRANSITION, new int(13)));
            DrawLoadingScreen("Deallocating Chunks...", false, glm::vec4(0.0, 0.0, 0.0, 0.3));
            openglManager.glWaitForMessage(GL_M_STATETRANSITION);
            break;
        case -2:
            GameManager::gameState = GameStates::EXIT;
            return 0;
        case 5000: //decrease texture res
            if (graphicsOptions.currTextureRes > 16){
                graphicsOptions.currTextureRes >>= 1;
                fileManager.loadTexturePack(graphicsOptions.currTexturePack);
            }
            break;
        case 5001: //increase texture res
            if (graphicsOptions.currTextureRes < graphicsOptions.defaultTextureRes){
                graphicsOptions.currTextureRes <<= 1;
                fileManager.loadTexturePack(graphicsOptions.currTexturePack);
            }
            break;
        }
        return 1;
    }
    else{
        SDL_Event evnt;
        while (SDL_PollEvent(&evnt))
        {
            GameManager::inputManager->pushEvent(evnt);
            switch (evnt.type)
            {
            case SDL_QUIT:
                return 1;
            }
        }
    }

    return 1;
}

bool PlayerControl() {
    if (currMenu){
        isMouseIn = 0;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        int ret = currMenu->Control();
        if (ret == 1 || ret == -5){
            SDL_PumpEvents();
            isMouseIn = 1;
            SDL_SetRelativeMouseMode(SDL_TRUE);
            currMenu = NULL;
        }
        if (currMenu == markerMenu){
            switch (ret){
            case 2:
                currMenu = newMarkerMenu;
                break;
            case 3:
                currMenu = deleteMarkerMenu;
                deleteMarkerMenu->Open();
                break;
            }
        }
        else if (currMenu == newMarkerMenu){
            switch (ret){
            case 2:
                if (GameManager::markers.size() < 10){
                    GameManager::addMarker(player->worldPosition, menuOptions.markerName, glm::vec3(menuOptions.markerR / 255.0f, menuOptions.markerG / 255.0f, menuOptions.markerB / 255.0f));
                }
                SDL_PumpEvents();
                isMouseIn = 1;
                SDL_SetRelativeMouseMode(SDL_TRUE);
                currMenu = NULL;
            }
        }
        else if (currMenu == deleteMarkerMenu){
            if (ret >= 1000){
                int markerIndex = ret - 1000;
                if (markerIndex < (int)GameManager::markers.size()){
                    GameManager::markers.erase(GameManager::markers.begin() + markerIndex);
                    SDL_PumpEvents();
                    isMouseIn = 1;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    currMenu = NULL;
                }
            }
        }
    }
    else{
        isMouseIn = 1;
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_Event evnt;
        while (SDL_PollEvent(&evnt))
        {
            switch (evnt.type)
            {
            case SDL_QUIT:
                return 0;
            case SDL_MOUSEMOTION:
                if (GameManager::gameState == GameStates::PLAY){
                    player->mouseMove(evnt.motion.xrel, evnt.motion.yrel);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                GameManager::inputManager->pushEvent(evnt);
                break;
            case SDL_MOUSEBUTTONUP:
                GameManager::inputManager->pushEvent(evnt);
                break;
            case SDL_KEYDOWN:
                GameManager::inputManager->pushEvent(evnt);
                break;
            case SDL_KEYUP:
                GameManager::inputManager->pushEvent(evnt);
                break;
            case SDL_MOUSEWHEEL:
                GameManager::inputManager->pushEvent(evnt);
                break;
            case SDL_WINDOWEVENT:
                if (evnt.window.type == SDL_WINDOWEVENT_LEAVE || evnt.window.type == SDL_WINDOWEVENT_FOCUS_LOST){
                    //            GameState = PAUSE;
                    //            SDL_SetRelativeMouseMode(SDL_FALSE);
                    //            isMouseIn = 0;
                }
                break;
            }
        }
    }

    bool leftPress = GameManager::inputManager->getKeyDown(INPUT_MOUSE_LEFT);
    bool rightPress = GameManager::inputManager->getKeyDown(INPUT_MOUSE_RIGHT);
    bool leftRelease = GameManager::inputManager->getKeyUp(INPUT_MOUSE_LEFT);
    bool rightRelease = GameManager::inputManager->getKeyUp(INPUT_MOUSE_RIGHT);

    clickDragActive = 0;
    if (GameManager::inputManager->getKey(INPUT_BLOCK_SELECT)){
        clickDragActive = 1;
    }
    if (GameManager::inputManager->getKeyDown(INPUT_SCAN_WSO)) {
        GameManager::scanWSO();
    }

    static bool rightMousePressed = false;
    if (GameManager::inputManager->getKey(INPUT_MOUSE_RIGHT)){
        if (!rightMousePressed || GameManager::voxelEditor->isEditing()) {
            if (!(player->rightEquippedItem)){
                if (rightPress || clickDragActive) GameManager::clickDragRay(true);
            } else if (player->rightEquippedItem->type == ITEM_BLOCK) {
                player->dragBlock = player->rightEquippedItem;
                if (rightPress || clickDragActive) GameManager::clickDragRay(false);
            }

            rightMousePressed = true;
        }
    } else {
        rightMousePressed = false;
    }

    static bool leftMousePressed = false;
    if (GameManager::inputManager->getKey(INPUT_MOUSE_LEFT)){

        if (!leftMousePressed || GameManager::voxelEditor->isEditing()) {
            if (!(player->leftEquippedItem)){
                if (leftPress || clickDragActive) GameManager::clickDragRay(true);
            } else if (player->leftEquippedItem->type == ITEM_BLOCK){
                player->dragBlock = player->leftEquippedItem;

                if (leftPress || clickDragActive) GameManager::clickDragRay(false);
            }

            leftMousePressed = true;
        }
    } else {
        leftMousePressed = false;
    }

    if (rightRelease && GameManager::voxelEditor->isEditing()){
        if ((player->rightEquippedItem && player->rightEquippedItem->type == ITEM_BLOCK) || !player->rightEquippedItem){
            glToGame.enqueue(OMessage(GL_M_PLACEBLOCKS, new PlaceBlocksMessage(player->rightEquippedItem)));
        }
        player->dragBlock = NULL;    
    }

    if (leftRelease && GameManager::voxelEditor->isEditing()){
        if ((player->leftEquippedItem && player->leftEquippedItem->type == ITEM_BLOCK) || !player->leftEquippedItem){ //if he has a block or nothing for place or break
            //error checking for unloaded chunks
            glToGame.enqueue(OMessage(GL_M_PLACEBLOCKS, new PlaceBlocksMessage(player->leftEquippedItem)));
        }
        player->dragBlock = NULL;
    }

    if (GameManager::inputManager->getKeyDown(INPUT_FLY)){
        player->flyToggle();
    }
    if (GameManager::inputManager->getKeyDown(INPUT_HUD)){
        graphicsOptions.hudMode++;
        if (graphicsOptions.hudMode == 3) graphicsOptions.hudMode = 0;
    }
    if (GameManager::inputManager->getKeyDown(INPUT_MARKER)){
        currMenu = markerMenu;
    }
    if (GameManager::inputManager->getKeyDown(INPUT_WATER_UPDATE)){
        isWaterUpdating = !isWaterUpdating;
    }
    if (GameManager::inputManager->getKeyDown(INPUT_FLASH_LIGHT)){
        player->lightActive++;
        if (player->lightActive == 3){
            player->lightActive = 0;
        }
    }
    if (GameManager::inputManager->getKeyDown(INPUT_PHYSICS_BLOCK_UPDATES)){
        globalDebug2 = !globalDebug2;
    }
    if (GameManager::inputManager->getKeyDown(INPUT_UPDATE_FRUSTUM)){
        getFrustum = !getFrustum;
    }
    if (GameManager::inputManager->getKeyDown(INPUT_DEBUG)){
        debugVarh = !debugVarh;
    }
    if (GameManager::inputManager->getKeyDown(INPUT_SONAR)){
        sonarActive = !sonarActive;
        sonarDt = 0;
    }
    if (GameManager::inputManager->getKeyDown(INPUT_PLANET_DRAW_MODE)){
        planetDrawMode += 1.0f;
        if (planetDrawMode == 3.0f) planetDrawMode = 0.0f;
    }
    if (GameManager::inputManager->getKeyDown(INPUT_GRID)){
        gridState = !gridState;
    }
    if (GameManager::inputManager->getKeyDown(INPUT_RELOAD_SHADERS)){
        InitializeShaders();
    }
    if (GameManager::inputManager->getKeyDown(INPUT_DRAW_MODE)){
        if (++drawMode == 3) drawMode = 0;
        if (drawMode == 0){
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Solid mode
        }
        else if (drawMode == 1){
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); //Point mode
        }
        else if (drawMode == 2){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Wireframe mode
        }
    }
    if (GameManager::inputManager->getKeyDown(INPUT_PAUSE)){
        GameManager::gameState = GameStates::PAUSE;
        currMenu = pauseMenu;
        isMouseIn = 0;
        GameManager::savePlayerState();
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    if (GameManager::inputManager->getKeyDown(INPUT_INVENTORY)){
        GameManager::gameState = GameStates::INVENTORY;
        isMouseIn = 0;
        GameManager::savePlayerState();
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    if (GameManager::inputManager->getKeyDown(INPUT_ZOOM)){
        if (GameManager::gameState == GameStates::PLAY) player->zoom();
    }
    if (GameManager::inputManager->getKeyDown(INPUT_RELOAD_TEXTURES)){
        ReloadTextures();
        GameManager::chunkManager->remeshAllChunks();
    }
    if (GameManager::inputManager->getKeyDown(INPUT_RELOAD_BLOCKS)){
        if (!(fileManager.loadBlocks("Data/BlockData.ini"))){
            pError("Failed to reload blocks.");
            return 0;
        }
        for (size_t i = 1; i < Blocks.size(); i++){
            if (ObjectList[i] != NULL){
                delete ObjectList[i];
                ObjectList[i] = NULL;
            }
        }
        for (size_t i = 0; i < player->inventory.size(); i++){
            delete player->inventory[i];
        }
        player->inventory.clear();
        InitializeObjects();
    }
    return false;
}

bool MainMenuControl()
{
    string oldLoadString = menuOptions.loadGameString;

    SDL_SetRelativeMouseMode(SDL_FALSE);
    int ret = currMenu->Control();
    int rv;

    //cout << oldLoadString << " " << menuOptions.loadGameString << endl;
    if (oldLoadString != menuOptions.loadGameString){ //load game new planet selection
        string ws = fileManager.getWorldString("Saves/" + menuOptions.loadGameString + "/World/");
        if (ws != ""){
            menuOptions.selectPlanetName = ws;
        }
        else{
            fileManager.createWorldFile("Saves/" + menuOptions.loadGameString + "/World/");
        }
    }
    switch (ret){
    case -6:
        glExit = 1;
        return 0;
        break;
    case -4:
        if (currMenu->overlayActive != -1){
            currMenu->overlays[currMenu->overlayActive]->FlushControlStates();
            currMenu->overlayActive = -1;
        }
        break;
    case -3:
        if (currMenu == loadGameMenu){
            if (menuOptions.loadGameString != ""){
                cout << "Deleting " << "Saves/" + menuOptions.loadGameString << endl;
                // TODO: Silent Failure Here
                fileManager.deleteDirectory("Saves/" + menuOptions.loadGameString);
                menuOptions.loadGameString = "";
                currMenu->Open();
            }
            if (currMenu->overlayActive != -1){
                currMenu->overlays[currMenu->overlayActive]->FlushControlStates();
                currMenu->overlayActive = -1;
            }
        }
        break;
    case -2:
        glExit = 1;
        return 0;
        break;
    case -1: //exit
        mainMenu->SetOverlayActive(0);
        mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, GameManager::planet->radius * 2.00), 2.0, glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0), glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        break;
    case 1: // new Game
        mainMenuCamera.zoomTo(glm::dvec3(GameManager::planet->radius * 2.0, 0.0, GameManager::planet->radius*0.75), 5.0, glm::dvec3(-1.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(2000000.0, 0.0, 2000000.0), GameManager::planet->radius, 0.0);
        currMenu = newGameMenu;
        currMenu->FlushControlStates();
        break;
    case 2: // load game
        menuOptions.loadGameString = "";
        mainMenuCamera.zoomTo(glm::dvec3(GameManager::planet->radius * 2.0, 0.0, GameManager::planet->radius*0.75), 5.0, glm::dvec3(-1.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        currMenu = loadGameMenu;
        currMenu->Open();
        currMenu->FlushControlStates();
        break;
    case 3:
        mainMenuCamera.zoomTo(glm::dvec3(GameManager::planet->radius * 2.0, 0.0, GameManager::planet->radius*0.75), 5.0, glm::dvec3(-1.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(2000000.0, 0.0, 2000000.0), GameManager::planet->radius, 0.0);
        currMenu = worldEditorSelectMenu;
        currMenu->FlushControlStates();
        break;
    case 4: // texture pack
        mainMenuCamera.zoomTo(glm::dvec3(0.0, GameManager::planet->radius * 1.3, 0.0), 5.0, glm::dvec3(0.0, -1.0, 0.0), glm::dvec3(1.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        currMenu = texturePackMenu;
        currMenu->Open();
        currMenu->FlushControlStates();
        break;
    case 5: // game options
        mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, -GameManager::planet->radius * 1.01), 5.0, glm::dvec3(1.0, 0.0, 0.0), glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(-8000000.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        currMenu = gameOptionsMenu;
        currMenu->FlushControlStates();
        break;
    case 6: // video options
        mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, -GameManager::planet->radius * 1.01), 5.0, glm::dvec3(1.0, 0.0, 0.0), glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(-8000000.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        currMenu = videoOptionsMenu;
        currMenu->FlushControlStates();
        break;
    case 7: // audio options
        mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, -GameManager::planet->radius * 1.01), 5.0, glm::dvec3(1.0, 0.0, 0.0), glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(-8000000.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        currMenu = audioOptionsMenu;
        currMenu->FlushControlStates();
        break;
    case 8: //controls
        mainMenuCamera.zoomTo(glm::dvec3(-GameManager::planet->radius * 2.55, 0.0, 0.0), 5.0, glm::dvec3(1.0, 0.0, 0.0), glm::dvec3(0.0, -1.0, 0.0), glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        currMenu = controlsMenu;
        currMenu->FlushControlStates();
        break;
    case 9: //credits
        mainMenuCamera.zoomTo(glm::dvec3(-GameManager::planet->radius * 1.55, 0.0, GameManager::planet->radius), 5.0, glm::dvec3(1.0, 0.0, 0.0), glm::dvec3(0.0, -1.0, 0.0), glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        currMenu = creditsMenu;
        currMenu->FlushControlStates();
        break;
    case 10:
        if (currMenu == videoOptionsMenu){
            mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, GameManager::planet->radius * 1.35), 5.0, glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0), glm::dvec3(-8000000.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        }
        else{
            mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, GameManager::planet->radius * 1.35), 5.0, glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0), glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        }
        saveOptions();
        currMenu = mainMenu;
        currMenu->FlushControlStates();
        break;
    case 20:
        mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, GameManager::planet->radius * 1.35), 2.0, glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0), glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
        if (currMenu->overlayActive != -1){
            currMenu->overlays[currMenu->overlayActive]->FlushControlStates();
            currMenu->overlayActive = -1;
        }
        break;
    case 31:
        if (menuOptions.newGameString != "" && menuOptions.selectPlanetName != ""){

            if ((rv = GameManager::newGame(menuOptions.newGameString)) == 0){
                //DrawLoadingScreen("Initializing Chunks...", false, glm::vec4(0.0, 0.0, 0.0, 0.3));
                glToGame.enqueue(OMessage(GL_M_STATETRANSITION, new int(10)));
                openglManager.glWaitForMessage(GL_M_STATETRANSITION);
            }
            else if (rv == 1){
                newGameMenu->overlays[0]->SetOverlayText("INVALID FILENAME: Must use numbers, ascii letters, spaces, or underscores.");
                newGameMenu->SetOverlayActive(0);
            }
            else if (rv == 2){
                newGameMenu->overlays[0]->SetOverlayText("INVALID FILENAME: That save file name is already in use.");
                newGameMenu->SetOverlayActive(0);
            }
        }
        break;
    case 32:
        if (menuOptions.loadGameString != ""){
            if (GameManager::loadGame(menuOptions.loadGameString) == 0){
                //DrawLoadingScreen("Initializing Chunks...", false, glm::vec4(0.0, 0.0, 0.0, 0.3));
                glToGame.enqueue(OMessage(GL_M_STATETRANSITION, new int(11)));
                openglManager.glWaitForMessage(GL_M_STATETRANSITION);
            }
        }
        break;
    case 33:
        if (menuOptions.loadGameString != ""){
            loadGameMenu->overlays[0]->SetOverlayText("Are you sure you want to delete " + menuOptions.loadGameString + "?");
            loadGameMenu->SetOverlayActive(0);

            //if (GameManager::LoadGame(menuOptions.loadGameString) == 0){
            //    GameManager::InitializePlayer(player);
            //    GameState = ZOOMINGIN; //begin the zoom transition
            //    zoomState = 0;
            //}
        }
        break;
    case 38:// world editor
        if (menuOptions.selectPlanetName != ""){
            mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, 0.0), 2.0, glm::dvec3(-1.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, GameManager::planet->radius * 2.0);
            GameManager::initializeWorldEditor();
            glToGame.enqueue(OMessage(GL_M_STATETRANSITION, new int(14)));
            openglManager.glWaitForMessage(GL_M_STATETRANSITION);
        }
        break;
    case 945:
        InitializeShaders();
        break;
    case 5000: //decrease texture res
        if (graphicsOptions.currTextureRes > 16){
            graphicsOptions.currTextureRes >>= 1;
            fileManager.loadTexturePack(graphicsOptions.currTexturePack);
        }
        break;
    case 5001: //increase texture res
        if (graphicsOptions.currTextureRes < graphicsOptions.defaultTextureRes){
            graphicsOptions.currTextureRes <<= 1;
            fileManager.loadTexturePack(graphicsOptions.currTexturePack);
        }
        break;
    }
    if (currMenu == newGameMenu || currMenu == worldEditorSelectMenu){
        if (menuOptions.selectPlanetName != "" && ("Worlds/" + menuOptions.selectPlanetName + "/") != GameManager::planet->dirName){
            glToGame.enqueue(OMessage(GL_M_STATETRANSITION, new int(12)));
            openglManager.glWaitForMessage(GL_M_STATETRANSITION);
        }
    }
    else if (currMenu == loadGameMenu){
        if (menuOptions.selectPlanetName != "" && ("Worlds/" + menuOptions.selectPlanetName + "/") != GameManager::planet->dirName){
            glToGame.enqueue(OMessage(GL_M_STATETRANSITION, new int(12)));
            openglManager.glWaitForMessage(GL_M_STATETRANSITION);
        }
    }
    return 1;
}

bool EditorControl()
{
    SDL_Event evnt;
    float wheelSpeed;
    float focalLength;

    while (SDL_PollEvent(&evnt))
    {
        switch (evnt.type)
        {
        case SDL_QUIT:
            glExit = 1;
            return 0;
        case SDL_MOUSEMOTION:
            GameManager::worldEditor->injectMouseMove(evnt.motion.x, evnt.motion.y);
            if (GameManager::worldEditor->usingChunks){
                if (GameManager::inputManager->getKey(INPUT_MOUSE_RIGHT)){
                    wheelSpeed = 0.01;
                    GameManager::worldEditor->getChunkCamera().offsetAngles(evnt.motion.yrel*wheelSpeed*gameOptions.mouseSensitivity, evnt.motion.xrel*wheelSpeed*gameOptions.mouseSensitivity);
                }
            } else {
                if (GameManager::inputManager->getKey(INPUT_MOUSE_RIGHT)){
                    wheelSpeed = MAX((glm::length(mainMenuCamera.getFocalLength()) - GameManager::planet->radius) / (float)GameManager::planet->radius, 0);
                    mainMenuCamera.offsetAngles(evnt.motion.yrel*wheelSpeed, evnt.motion.xrel*wheelSpeed);
                }
            }
            break;
        case SDL_MOUSEWHEEL:
            GameManager::worldEditor->injectMouseWheel(evnt.wheel.y*3.0);
            if (GameManager::inputManager->getKey(INPUT_MOUSE_RIGHT)){
                if (EditorState == E_TREE_EDITOR){
                    focalLength = GameManager::worldEditor->getChunkCamera().getFocalLength();
                    wheelSpeed = 50;

                    focalLength -= evnt.wheel.y*wheelSpeed*0.025f;
                    if (focalLength < 0){
                        focalLength = 0;
                    } else if (focalLength > 600){
                        focalLength = 600;
                    }
                    GameManager::worldEditor->getChunkCamera().setFocalLength(focalLength);

                } else{
                    focalLength = mainMenuCamera.getFocalLength();
                    wheelSpeed = 50 + MAX((glm::length(focalLength) - GameManager::planet->radius)*0.01*planetScale, 0);

                    focalLength -= evnt.wheel.y*wheelSpeed*4.0f;
                    if (focalLength < GameManager::planet->radius + 1000){
                        focalLength = GameManager::planet->radius + 1000;
                    } else if (focalLength > GameManager::planet->radius * 20){
                        focalLength = GameManager::planet->radius * 20;
                    }
                    mainMenuCamera.setFocalLength(focalLength);
                }
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            GameManager::inputManager->pushEvent(evnt);
            GameManager::worldEditor->injectMouseDown(evnt.button.button-1);
            break;
        case SDL_MOUSEBUTTONUP:
            GameManager::inputManager->pushEvent(evnt);
            GameManager::worldEditor->injectMouseUp(evnt.button.button-1);
            break;
        case SDL_KEYDOWN:
            GameManager::inputManager->pushEvent(evnt);
            GameManager::worldEditor->injectKeyboardEvent(evnt);

            //TODO:Replace all this
            if (evnt.key.keysym.sym == SDLK_n){
                isWaterUpdating = !isWaterUpdating;
            }
            else if (evnt.key.keysym.sym == SDLK_l){
                player->lightActive++;
                if (player->lightActive == 3){
                    player->lightActive = 0;
                }
            }
            else if (evnt.key.keysym.sym == SDLK_g){
                gridState = !gridState;
            }
            else if (evnt.key.keysym.sym == SDLK_p){
                globalDebug2 = !globalDebug2;
            }
            else if (evnt.key.keysym.sym == SDLK_u){
                getFrustum = !getFrustum;
            }
            else if (evnt.key.keysym.sym == SDLK_h){
                debugVarh = !debugVarh;
            }
            else if (evnt.key.keysym.sym == SDLK_r){
                sonarActive = !sonarActive;
                sonarDt = 0;
            }
            else if (evnt.key.keysym.sym == SDLK_j){
                planetDrawMode += 1.0f;
                if (planetDrawMode == 3.0f) planetDrawMode = 0.0f;
            }
            else if (evnt.key.keysym.sym == SDLK_F11){
                InitializeShaders();
            }
            else if (evnt.key.keysym.sym == SDLK_m){
                if (++drawMode == 3) drawMode = 0;
                if (drawMode == 0){
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Solid mode
                }
                else if (drawMode == 1){
                    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); //Point mode
                }
                else if (drawMode == 2){
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Wireframe mode
                }
            }
            else if (evnt.key.keysym.sym == SDLK_ESCAPE){

            }
            else if (evnt.key.keysym.sym == SDLK_F5){
                ReloadTextures();
            }

            break;
        case SDL_KEYUP:
            GameManager::inputManager->pushEvent(evnt);
            break;
        case SDL_WINDOWEVENT:
            if (evnt.window.type == SDL_WINDOWEVENT_LEAVE || evnt.window.type == SDL_WINDOWEVENT_FOCUS_LOST){
                //            GameState = PAUSE;
                //            SDL_SetRelativeMouseMode(SDL_FALSE);
                //            isMouseIn = 0;
            }
            break;
        }
    }

    return 0;
}

void RebuildWindow()
{
    if (graphicsOptions.needsWindowReload){
        glFinish();
        SDL_GL_MakeCurrent(mainWindow, 0);
        SDL_DestroyWindow(mainWindow);
        //    SDL_Window *tmp = mainWindow;
        mainWindow = NULL;

        Uint32 wflags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
        if (graphicsOptions.isFullscreen) wflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        if (graphicsOptions.isBorderless) wflags |= SDL_WINDOW_BORDERLESS;

        mainWindow = SDL_CreateWindow("Seed Of Andromeda", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, graphicsOptions.screenWidth, graphicsOptions.screenHeight, wflags);

        if (mainWindow == NULL){
            exit(343);
        }

        int w, h;
        SDL_GetWindowSize(mainWindow, &w, &h);
        graphicsOptions.windowWidth = w;
        graphicsOptions.windowHeight = h;

        SDL_GL_MakeCurrent(mainWindow, mainOpenGLContext);

        graphicsOptions.needsWindowReload = 0;
    }

    rebuildFrameBuffer();

    //SDL_DestroyWindow(tmp); //destroy the window only after initializing a new one
}

void rebuildFrameBuffer()
{
    openglManager.FreeFrameBuffer();
    openglManager.InitializeFrameBuffer();
    graphicsOptions.needsFboReload = 0;
}

void UpdatePlayer()
{
    double dist = player->facePosition.y + GameManager::planet->radius;
    player->update(isMouseIn, GameManager::planet->getGravityAccel(dist), GameManager::planet->getAirFrictionForce(dist, glm::length(player->velocity)));

    Chunk **chunks = new Chunk*[8];
    player->isGrounded = 0;
    player->setMoveMod(1.0f);
    player->canCling = 0;
    player->collisionData.yDecel = 0.0f;
    //    cout << "C";

    Chunk::modifyLock.lock();
    for (int i = 0; i < playerCollisionSteps; i++){
        player->gridPosition += player->velocity * (1.0f / playerCollisionSteps) * glSpeedFactor;
        player->facePosition += player->velocity * (1.0f / playerCollisionSteps) * glSpeedFactor;
        player->collisionData.clear();
        GameManager::voxelWorld->getClosestChunks(player->gridPosition, chunks); //DANGER HERE!
        aabbChunkCollision(player, &(player->gridPosition), chunks, 8);
        player->applyCollisionData();
    }
    Chunk::modifyLock.unlock();

    delete[] chunks;
}

void ZoomingUpdate()
{
    if (GameManager::gameState == GameStates::ZOOMINGIN){
        if (openglManager.zoomState == 0){
            currMenu = NULL;
            glm::dvec3 targetPos = glm::dvec3(glm::dmat4(GameManager::planet->rotationMatrix) * glm::dvec4(player->worldPosition, 1.0));
            glm::dvec3 targetDir = glm::dvec3(glm::dmat4(GameManager::planet->rotationMatrix) * glm::dvec4(player->worldDirection(), 1.0));
            glm::dvec3 targetRight = glm::dvec3(glm::dmat4(GameManager::planet->rotationMatrix) * glm::dvec4(player->worldRight(), 1.0));
            mainMenuCamera.zoomTo(targetPos, 5.0, targetDir, targetRight, glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
            openglManager.zoomState = 1;
        }
        else if (openglManager.zoomState == 1){
            if (mainMenuCamera.getIsZooming() == 0){
                GameManager::gameState = GameStates::PLAY;
                //        SDL_PumpEvents();
                isMouseIn = 1;
                //    SDL_SetRelativeMouseMode(SDL_TRUE);
                currMenu = NULL;
                openglManager.zoomState = 0;
            }
        }
    }
    else if (GameManager::gameState == GameStates::ZOOMINGOUT){
        if (openglManager.zoomState == 0){
            currMenu = NULL;
            mainMenuCamera.setPosition(glm::dvec3(glm::dmat4(GameManager::planet->rotationMatrix) * glm::dvec4(player->worldPosition, 1.0)));
            mainMenuCamera.setDirection(glm::vec3(GameManager::planet->rotationMatrix * glm::vec4(player->worldDirection(), 1.0)));
            mainMenuCamera.setRight(glm::vec3(GameManager::planet->rotationMatrix * glm::vec4(player->worldRight(), 1.0f)));
            mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, GameManager::planet->radius * 1.35), 4.0, glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0), glm::dvec3(2000000.0, 0.0, 2000000.0), GameManager::planet->radius, 0.0);
            openglManager.zoomState = 1;
        }
        else if (openglManager.zoomState == 1){
            if (mainMenuCamera.getIsZooming() == 0){
                GameManager::gameState = GameStates::MAINMENU;
                //            SDL_PumpEvents();
                isMouseIn = 0;
                //        SDL_SetRelativeMouseMode(SDL_FALSE);
                currMenu = mainMenu;
                openglManager.zoomState = 0;
            }
        }
        else if (openglManager.zoomState == 2){ //zooming to a new planet
            mainMenuCamera.zoomTo(glm::dvec3(GameManager::planet->radius * 2.0, 0.0, 50000000.0), 0.5, glm::dvec3(-1.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(0.0, 0.0, 0.0), GameManager::planet->radius, 0.0);
            openglManager.zoomState = 3;
        }
        else if (openglManager.zoomState == 3){
            if (mainMenuCamera.getIsZooming() == 0){
                glToGame.enqueue(OMessage(GL_M_NEWPLANET, NULL));
                openglManager.zoomState = 4;
                openglManager.glWaitForMessage(GL_M_DONE);
            }
        }
        else if (openglManager.zoomState == 4){
            if (mainMenuCamera.getIsZooming() == 0){
                GameManager::gameState = GameStates::MAINMENU;
                openglManager.zoomState = 0;
            }
        }
    }
    else{
        pError("Zooming update called with invalid gamestate.");
        exit(1);
    }
}

void OpenglManager::UpdateTerrainMesh(TerrainMeshMessage *tmm)
{
    TerrainBuffers *tb = tmm->terrainBuffers;

    if (tmm->indexSize){
        if (tb->vaoID == 0) glGenVertexArrays(1, &(tb->vaoID));
        glBindVertexArray(tb->vaoID);

        if (tb->vboID == 0) glGenBuffers(1, &(tb->vboID));
        glBindBuffer(GL_ARRAY_BUFFER, tb->vboID); // Bind the buffer (vertex array data)
        glBufferData(GL_ARRAY_BUFFER, tmm->index * sizeof(TerrainVertex), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, tmm->index * sizeof(TerrainVertex), &(tmm->verts[0]));

        if (tb->vboIndexID == 0) glGenBuffers(1, &(tb->vboIndexID));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tb->vboIndexID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, tmm->indexSize * sizeof(GLushort), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, tmm->indexSize * sizeof(GLushort), &(tmm->indices[0]));

        //vertices
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), 0);
        //UVs
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (12)));
        //normals
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (20)));
        //colors
        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (32)));
        //slope color
        glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (36)));
        //beach color
        glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (40)));
        //texureUnit, temperature, rainfall, specular
        glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (44)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
        glEnableVertexAttribArray(5);
        glEnableVertexAttribArray(6);
        glBindVertexArray(0); // Disable our Vertex Buffer Object  

        if (tmm->treeIndexSize){
            int treeIndex = (tmm->treeIndexSize * 4) / 6;
            glGenBuffers(1, &(tb->treeVboID));
            glBindBuffer(GL_ARRAY_BUFFER, tb->treeVboID); // Bind the buffer (vertex array data)
            glBufferData(GL_ARRAY_BUFFER, treeIndex * sizeof(TreeVertex), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, treeIndex * sizeof(TreeVertex), tmm->treeVerts);
            delete[] tmm->treeVerts;
        }
        else{
            if (tb->treeVboID != 0) glDeleteBuffers(1, &(tb->treeVboID));
            tb->treeVboID = 0;
        }
        tb->boundingBox = tmm->boundingBox;
        tb->drawX = tmm->drawX;
        tb->drawY = tmm->drawY;
        tb->drawZ = tmm->drawZ;
        tb->worldX = tmm->worldX;
        tb->worldY = tmm->worldY;
        tb->worldZ = tmm->worldZ;
        tb->cullRadius = tmm->cullRadius;
        tb->indexSize = tmm->indexSize;
        tb->treeIndexSize = tmm->treeIndexSize;
        delete[] tmm->verts;
        delete[] tmm->indices;
        if (tb->vecIndex == -1){
            tb->vecIndex = GameManager::planet->drawList[tmm->face].size();
            GameManager::planet->drawList[tmm->face].push_back(tb);
        }
    }
    else{
        if (tb->vecIndex != -1){
            GameManager::planet->drawList[tmm->face][tb->vecIndex] = GameManager::planet->drawList[tmm->face].back();
            GameManager::planet->drawList[tmm->face][tb->vecIndex]->vecIndex = tb->vecIndex;
            GameManager::planet->drawList[tmm->face].pop_back();
        }
        if (tb->vaoID != 0) glDeleteVertexArrays(1, &(tb->vaoID));
        if (tb->vboID != 0) glDeleteBuffers(1, &(tb->vboID));
        if (tb->treeVboID != 0) glDeleteBuffers(1, &(tb->treeVboID));
        if (tb->vboIndexID != 0) glDeleteBuffers(1, &(tb->vboIndexID));
        delete tb; //possible race condition
    }
    delete tmm;
}

inline bool mapBufferData(GLuint& vboID, GLsizeiptr size, void* src, GLenum usage) {
    // Block Vertices
    if (vboID == 0){
        glGenBuffers(1, &(vboID)); // Create the buffer ID
    }
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, usage);


    void *v = glMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

    if (v == NULL) return false;

    memcpy(v, src, size);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

void OpenglManager::UpdateChunkMesh(ChunkMeshData *cmd)
{
    ChunkMesh *cm = cmd->chunkMesh;

    if (cmd->chunkMesh == NULL){
        pError("Chunkmesh == NULL : " + to_string(cmd->debugCode));
        printf(" Chunkmesh == NULL");
    }

    //store the index data for sorting in the chunk mesh
    cm->transQuadIndices.swap(cmd->transQuadIndices);
    cm->transQuadPositions.swap(cmd->transQuadPositions);

    switch (cmd->type) {
        case MeshJobType::DEFAULT:
            if (cmd->vertices.size()) {
                if (cm->vecIndex == -1){
                    cm->vecIndex = chunkMeshes.size();
                    chunkMeshes.push_back(cm);
                }

                mapBufferData(cm->vboID, cmd->vertices.size() * sizeof(BlockVertex), &(cmd->vertices[0]), GL_STATIC_DRAW);

                ChunkRenderer::bindVao(cm);
            } else {
                if (cm->vboID != 0){
                    glDeleteBuffers(1, &(cm->vboID));
                    cm->vboID = 0;
                }
                if (cm->vaoID != 0){
                    glDeleteVertexArrays(1, &(cm->vaoID));
                    cm->vaoID = 0;
                } 
            }

            if (cmd->transVertices.size()) {
                if (cm->vecIndex == -1){
                    cm->vecIndex = chunkMeshes.size();
                    chunkMeshes.push_back(cm);
                }
    
                //vertex data
                mapBufferData(cm->transVboID, cmd->transVertices.size() * sizeof(BlockVertex), &(cmd->transVertices[0]), GL_STATIC_DRAW);

                //index data
                mapBufferData(cm->transIndexID, cm->transQuadIndices.size() * sizeof(ui32), &(cm->transQuadIndices[0]), GL_STATIC_DRAW);

                cm->needsSort = true; //must sort when changing the mesh

                ChunkRenderer::bindTransparentVao(cm);
            } else {
                if (cm->transVaoID != 0){
                    glDeleteVertexArrays(1, &(cm->transVaoID));
                    cm->transVaoID = 0;
                }
                if (cm->transVboID == 0) {
                    glDeleteBuffers(1, &(cm->transVboID));
                    cm->transVboID = 0;
                }
                if (cm->transIndexID == 0) {
                    glDeleteBuffers(1, &(cm->transIndexID));
                    cm->transIndexID = 0;
                }
            }

            if (cmd->cutoutVertices.size()) {
                if (cm->vecIndex == -1){
                    cm->vecIndex = chunkMeshes.size();
                    chunkMeshes.push_back(cm);
                }

                mapBufferData(cm->cutoutVboID, cmd->cutoutVertices.size() * sizeof(BlockVertex), &(cmd->cutoutVertices[0]), GL_STATIC_DRAW);

                ChunkRenderer::bindCutoutVao(cm);
            } else {
                if (cm->cutoutVaoID != 0){
                    glDeleteVertexArrays(1, &(cm->cutoutVaoID));
                    cm->cutoutVaoID = 0;
                }
                if (cm->cutoutVboID == 0) {
                    glDeleteBuffers(1, &(cm->cutoutVboID));
                    cm->cutoutVboID = 0;
                }
            }
            cm->meshInfo = cmd->meshInfo;
        //The missing break is deliberate!
        case MeshJobType::LIQUID:

            cm->meshInfo.waterIndexSize = cmd->meshInfo.waterIndexSize;
            if (cmd->waterVertices.size()) {
                if (cm->vecIndex == -1){
                    cm->vecIndex = chunkMeshes.size();
                    chunkMeshes.push_back(cm);
                }

                mapBufferData(cm->waterVboID, cmd->waterVertices.size() * sizeof(LiquidVertex), &(cmd->waterVertices[0]), GL_STREAM_DRAW);           

                ChunkRenderer::bindWaterVao(cm);
            } else {
                if (cm->waterVboID != 0){
                    glDeleteBuffers(1, &(cm->waterVboID));
                    cm->waterVboID = 0;
                }   
                if (cm->waterVaoID != 0){
                    glDeleteVertexArrays(1, &(cm->waterVaoID));
                    cm->waterVaoID = 0;
                }
            }
            break;
    }
    
    //If this mesh isnt in use anymore, delete it
    if (cm->vboID == 0 && cm->waterVboID == 0 && cm->transVboID == 0 && cm->cutoutVboID == 0){
        if (cm->vecIndex != -1){
            chunkMeshes[cm->vecIndex] = chunkMeshes.back();
            chunkMeshes[cm->vecIndex]->vecIndex = cm->vecIndex;
            chunkMeshes.pop_back();
        }
        delete cm;
    }


    delete cmd;
}

void OpenglManager::UpdateParticleMesh(ParticleMeshMessage *pmm)
{
    ParticleMesh *pm = pmm->mesh;
    int n = pmm->verts.size();

    if (n != 0){
        if (pm->uvBufferID == 0){
            glGenBuffers(1, &(pm->uvBufferID));
            glGenBuffers(1, &(pm->billboardVertexBufferID));
            glBindBuffer(GL_ARRAY_BUFFER, pm->uvBufferID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(particleUVs), particleUVs, GL_STATIC_DRAW);
        }

        pm->usedParticles.swap(pmm->usedParticles);
        pm->size = pmm->size;
        pm->X = pmm->X;
        pm->Y = pmm->Y;
        pm->Z = pmm->Z;

        glBindBuffer(GL_ARRAY_BUFFER, pm->billboardVertexBufferID); // Bind the buffer (vertex array data)
        glBufferData(GL_ARRAY_BUFFER, n * sizeof(BillboardVertex), NULL, GL_STREAM_DRAW);
        void *v = glMapBufferRange(GL_ARRAY_BUFFER, 0, n * sizeof(BillboardVertex), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        memcpy(v, &(pmm->verts[0]), n * sizeof(BillboardVertex));
        glUnmapBuffer(GL_ARRAY_BUFFER);

        if (pm->vecIndex == -1){
            pm->vecIndex = particleMeshes.size();
            particleMeshes.push_back(pm);
        }
    }
    else{ //clear buffers
        if (pm->uvBufferID != 0){
            glDeleteBuffers(1, &pm->billboardVertexBufferID);
            glDeleteBuffers(1, &pm->uvBufferID);
            pm->billboardVertexBufferID = 0;
            pm->uvBufferID = 0;
        }
    }
    delete pmm;
}

void OpenglManager::UpdatePhysicsBlockMesh(PhysicsBlockMeshMessage *pbmm)
{
    PhysicsBlockMesh *pbm = pbmm->mesh;
    if (pbmm->verts.size() != 0){
        glGenBuffers(1, &pbm->vboID); // Create the buffer ID
        glBindBuffer(GL_ARRAY_BUFFER, pbm->vboID); // Bind the buffer (vertex array data)
        glBufferData(GL_ARRAY_BUFFER, pbmm->verts.size() * sizeof(PhysicsBlockVertex), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, pbmm->verts.size() * sizeof(PhysicsBlockVertex), &(pbmm->verts[0]));

        if (pbm->vecIndex == -1){
            pbm->vecIndex = physicsBlockMeshes.size();
            physicsBlockMeshes.push_back(pbm);
        }
    }
    else if (pbmm->posLight.size() != 0){

        pbm->bX = pbmm->bX;
        pbm->bY = pbmm->bY;
        pbm->bZ = pbmm->bZ;
        pbm->numBlocks = pbmm->numBlocks;

        if (pbm->positionLightBufferID == 0){
            glGenBuffers(1, &pbm->positionLightBufferID);
        }
        glBindBuffer(GL_ARRAY_BUFFER, pbm->positionLightBufferID);
        glBufferData(GL_ARRAY_BUFFER, pbmm->posLight.size() * sizeof(PhysicsBlockPosLight), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, pbmm->posLight.size() * sizeof(PhysicsBlockPosLight), &(pbmm->posLight[0]));

        if (pbm->vecIndex == -1){
            pbm->vecIndex = physicsBlockMeshes.size();
            physicsBlockMeshes.push_back(pbm);
        }
    }
    else{ //delete
        if (pbm->vecIndex != -1){
            physicsBlockMeshes[pbm->vecIndex] = physicsBlockMeshes.back();
            physicsBlockMeshes[pbm->vecIndex]->vecIndex = pbm->vecIndex;
            physicsBlockMeshes.pop_back();
        }
        if (pbm->vboID != 0){
            glDeleteBuffers(1, &pbm->vboID);
        }
        if (pbm->positionLightBufferID != 0){
            glDeleteBuffers(1, &pbm->positionLightBufferID);
        }
        delete pbm;
    }
    delete pbmm;
}

void OpenglManager::UpdateMeshDistances()
{
    ChunkMesh *cm;
    int mx, my, mz;
    double dx, dy, dz;
    double cx, cy, cz;

    mx = (int)player->headPosition.x;
    my = (int)player->headPosition.y;
    mz = (int)player->headPosition.z;
    //GLuint sticks = SDL_GetTicks();

    static GLuint saveTicks = SDL_GetTicks();

    for (int i = 0; i < chunkMeshes.size(); i++){ //update distances for all chunk meshes
        cm = chunkMeshes[i];
        const glm::ivec3 &cmPos = cm->position;
        cx = (mx <= cmPos.x) ? cmPos.x : ((mx > cmPos.x + CHUNK_WIDTH) ? (cmPos.x + CHUNK_WIDTH) : mx);
        cy = (my <= cmPos.y) ? cmPos.y : ((my > cmPos.y + CHUNK_WIDTH) ? (cmPos.y + CHUNK_WIDTH) : my);
        cz = (mz <= cmPos.z) ? cmPos.z : ((mz > cmPos.z + CHUNK_WIDTH) ? (cmPos.z + CHUNK_WIDTH) : mz);
        dx = cx - mx;
        dy = cy - my;
        dz = cz - mz;
        cm->distance = sqrt(dx*dx + dy*dy + dz*dz);
    }

    //for (in ti )
}

//TODO: Use a single struct for input, and maybe split this into two calls
void OpenglManager::Draw(Camera &chunkCamera, Camera &worldCamera)
{
    float FogColor[3];
    float fogStart, fogEnd;
    glm::vec3 lightPos = glm::vec3(1.0, 0.0, 0.0);
    float theta = glm::dot(glm::dvec3(lightPos), glm::normalize(glm::dvec3(glm::dmat4(GameManager::planet->rotationMatrix) * glm::dvec4(worldCamera.position(), 1.0))));

    glm::mat4 VP;
    //********************************* TODO: PRECOMPILED HEADERS for compilation speed?
    float fogTheta = glm::clamp(theta, 0.0f, 1.0f);
    fogStart = 0;
    if (player->isUnderWater){
        GLfloat underwaterColor[4];
        underwaterColor[0] = fogTheta * 1.0 / 2;
        underwaterColor[1] = fogTheta * 1.0 / 2;
        underwaterColor[2] = fogTheta * 1.0 / 2;
        underwaterColor[3] = 1.0;
        fogEnd = 50 + fogTheta * 100;
        FogColor[0] = underwaterColor[0];
        FogColor[1] = underwaterColor[1];
        FogColor[2] = underwaterColor[2];
    }
    else{
        fogEnd = 100000;
        FogColor[0] = 1.0;
        FogColor[1] = 1.0;
        FogColor[2] = 1.0;
    }

    float st = MAX(0.0f, theta + 0.06);
    if (st > 1) st = 1;

    //far znear for maximum Terrain Patch z buffer precision
    //this is currently incorrect
   
    double nearClip = MIN((csGridWidth / 2.0 - 3.0)*32.0*0.7, 75.0) - ((double)( GameManager::chunkIOManager->getLoadListSize()) / (double)(csGridWidth*csGridWidth*csGridWidth))*55.0;
    if (nearClip < 0.1) nearClip = 0.1;
    double a = 0.0;

    a = closestTerrainPatchDistance / (sqrt(1.0f + pow(tan(graphicsOptions.fov / 2.0), 2.0) * (pow((double)graphicsOptions.screenWidth / graphicsOptions.screenHeight, 2.0) + 1.0))*2.0);
    if (a < 0) a = 0;

    double clip = MAX(nearClip / planetScale*0.5, a);

    worldCamera.setClippingPlane(clip, MAX(300000000.0 / planetScale, closestTerrainPatchDistance + 10000000));
    worldCamera.updateProjection();

    VP = worldCamera.projectionMatrix() * worldCamera.viewMatrix();

    float ambVal = st*(0.76f) + .01f;
    if (ambVal > 1.0f) ambVal = 1.0f;
    float diffVal = 1.0f - ambVal;
    glm::vec3 diffColor;

    if (theta < 0.01f){
        diffVal += (theta - 0.01) * 8;
        if (diffVal < 0.0f) diffVal = 0.0f;
    }

    int sh = (int)(theta*64.0f);
    if (theta < 0){
        sh = 0;
    }
    diffColor.r = (sunColor[sh][0] / 255.0f) * diffVal;
    diffColor.g = (sunColor[sh][1] / 255.0f) * diffVal;
    diffColor.b = (sunColor[sh][2] / 255.0f) * diffVal;

    GameManager::drawSpace(VP, 1);
    GameManager::drawPlanet(worldCamera.position(), VP, worldCamera.viewMatrix(), ambVal + 0.1, lightPos, nearClip / planetScale, 1);

    if (graphicsOptions.hudMode == 0){
        for (size_t i = 0; i < GameManager::markers.size(); i++){
            GameManager::markers[i].Draw(VP, worldCamera.position());
        }
    }

    //close znear for chunks
    VP = chunkCamera.projectionMatrix() * chunkCamera.viewMatrix();

    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);

    //*********************Blocks*******************
    lightPos = glm::normalize(glm::dvec3(glm::dmat4(glm::inverse(player->worldRotationMatrix)) * glm::dmat4(GameManager::planet->invRotationMatrix) * glm::dvec4(lightPos, 1)));
    
    const glm::vec3 chunkDirection = chunkCamera.direction();

    drawBlocks(VP, chunkCamera.position(), lightPos, diffColor, player->lightActive, ambVal, fogEnd, fogStart, FogColor, &(chunkDirection[0]));
    drawCutoutBlocks(VP, chunkCamera.position(), lightPos, diffColor, player->lightActive, ambVal, fogEnd, fogStart, FogColor, &(chunkDirection[0]));
    
    if (gridState != 0){
        GameManager::voxelWorld->getChunkManager().drawChunkLines(VP, chunkCamera.position());
    }

    glDepthMask(GL_FALSE);
    drawTransparentBlocks(VP, chunkCamera.position(), lightPos, diffColor, player->lightActive, ambVal, fogEnd, fogStart, FogColor, &(chunkDirection[0]));
    glDepthMask(GL_TRUE);

    if (sonarActive){
        glDisable(GL_DEPTH_TEST);
        DrawSonar(VP, player->headPosition);
        glEnable(GL_DEPTH_TEST);
    }


    if (GameManager::voxelEditor->isEditing()){
        int ID;
        if (player->dragBlock != NULL){
            ID = player->dragBlock->ID;
        }
        else{
            ID = 0;
        }
  
        GameManager::voxelEditor->drawGuides(chunkCamera.position(), VP, ID);
    }

    glLineWidth(1);

    if (physicsBlockMeshes.size()){
        DrawPhysicsBlocks(VP, chunkCamera.position(), lightPos, diffColor, player->lightActive, ambVal, fogEnd, fogStart, FogColor, &(chunkDirection[0]));
    }

    
    //********************Water********************
    DrawWater(VP, chunkCamera.position(), st, fogEnd, fogStart, FogColor, lightPos, diffColor, player->isUnderWater);

    if (particleMeshes.size() > 0){
        vcore::GLProgram* bProgram = GameManager::glProgramManager->getProgram("Billboard");
        bProgram->use();

        glUniform1f(bProgram->getUniform("lightType"), (GLfloat)player->lightActive);
        glUniform3fv(bProgram->getUniform("eyeNormalWorldspace"), 1, &(chunkDirection[0]));
        glUniform1f(bProgram->getUniform("sunVal"), st);
        glUniform3f(bProgram->getUniform("AmbientLight"), (GLfloat)1.1f, (GLfloat)1.1f, (GLfloat)1.1f);

        const glm::mat4 &chunkViewMatrix = chunkCamera.viewMatrix();

        glm::vec3 cameraRight(chunkViewMatrix[0][0], chunkViewMatrix[1][0], chunkViewMatrix[2][0]);
        glm::vec3 cameraUp(chunkViewMatrix[0][1], chunkViewMatrix[1][1], chunkViewMatrix[2][1]);

        glUniform3f(bProgram->getUniform("cameraUp_worldspace"), cameraUp.x, cameraUp.y, cameraUp.z);
        glUniform3f(bProgram->getUniform("cameraRight_worldspace"), cameraRight.x, cameraRight.y, cameraRight.z);


        //glDepthMask(GL_FALSE);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        for (size_t i = 0; i < particleMeshes.size(); i++){

            if (particleMeshes[i]->animated){
                ParticleBatch::drawAnimated(particleMeshes[i], player->headPosition, VP);
            }
            else{
                ParticleBatch::draw(particleMeshes[i], player->headPosition, VP);
            }
        }
        // TODO(Ben): Maybe make this a part of GLProgram?
        glVertexAttribDivisor(0, 0);
        glVertexAttribDivisor(2, 0); //restore divisors
        glVertexAttribDivisor(3, 0);
        glVertexAttribDivisor(4, 0);
        glVertexAttribDivisor(5, 0);
        

        bProgram->unuse();
    }
    debugRenderer->render(VP, glm::vec3(chunkCamera.position()));

}

const float sonarDistance = 200;
const float sonarWidth = 30;
void OpenglManager::DrawSonar(glm::mat4 &VP, glm::dvec3 &position)
{
    //*********************Blocks*******************

    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("Sonar");
    program->use();

    bindBlockPacks();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    glUniform1f(program->getUniform("sonarDistance"), sonarDistance);
    glUniform1f(program->getUniform("waveWidth"), sonarWidth);
    glUniform1f(program->getUniform("dt"), sonarDt);

    float fadeDist;
    if (NoChunkFade){
        fadeDist = (GLfloat)10000.0f;
    }
    else{
        fadeDist = (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f;
    }
    glUniform1f(program->getUniform("fadeDistance"), fadeDist);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    for (unsigned int i = 0; i < chunkMeshes.size(); i++)
    {
        ChunkRenderer::drawBlocks(chunkMeshes[i], program, position, VP);
    }
    
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    program->unuse();
}

void OpenglManager::drawBlocks(const glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir)
{
    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("Block");
    program->use();

    glUniform1f(program->getUniform("lightType"), lightActive);

    glUniform3fv(program->getUniform("eyeNormalWorldspace"), 1, eyeDir);
    glUniform1f(program->getUniform("fogEnd"), (GLfloat)fogEnd);
    glUniform1f(program->getUniform("fogStart"), (GLfloat)fogStart);
    glUniform3fv(program->getUniform("fogColor"), 1, fogColor);
    glUniform3f(program->getUniform("lightPosition_worldspace"), lightPos.x, lightPos.y, lightPos.z);
    glUniform1f(program->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(program->getUniform("specularIntensity"), graphicsOptions.specularIntensity*0.3);

    bindBlockPacks();

    glUniform1f(program->getUniform("dt"), (GLfloat)bdt);

    glUniform1f(program->getUniform("sunVal"), sunVal);

    glUniform1f(program->getUniform("alphaMult"), 1.0f);

    float blockAmbient = 0.000f;
    glUniform3f(program->getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3f(program->getUniform("lightColor"), (GLfloat)lightColor.r, (GLfloat)lightColor.g, (GLfloat)lightColor.b);

    float fadeDist;
    if (NoChunkFade){
        fadeDist = (GLfloat)10000.0f;
    }
    else{
        fadeDist = (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f;
    }

    glUniform1f(program->getUniform("fadeDistance"), fadeDist);

    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    glLineWidth(3);

    glDisable(GL_CULL_FACE);

    glm::dvec3 cpos;

    static GLuint saveTicks = SDL_GetTicks();
    bool save = 0;
    if (SDL_GetTicks() - saveTicks >= 60000){ //save once per minute
        save = 1;
        saveTicks = SDL_GetTicks();
    }

    int mx, my, mz;
    double cx, cy, cz;
    double dx, dy, dz;
    mx = (int)position.x;
    my = (int)position.y;
    mz = (int)position.z;
    ChunkMesh *cm;
    
    for (int i = chunkMeshes.size()-1; i >= 0; i--)
    {
        cm = chunkMeshes[i];
        const glm::ivec3 &cmPos = cm->position;

        //calculate distance

        cx = (mx <= cmPos.x) ? cmPos.x : ((mx > cmPos.x + CHUNK_WIDTH) ? (cmPos.x + CHUNK_WIDTH) : mx);
        cy = (my <= cmPos.y) ? cmPos.y : ((my > cmPos.y + CHUNK_WIDTH) ? (cmPos.y + CHUNK_WIDTH) : my);
        cz = (mz <= cmPos.z) ? cmPos.z : ((mz > cmPos.z + CHUNK_WIDTH) ? (cmPos.z + CHUNK_WIDTH) : mz);
        dx = cx - mx;
        dy = cy - my;
        dz = cz - mz;
        cm->distance = sqrt(dx*dx + dy*dy + dz*dz);

        if (SphereInFrustum((float)(cmPos.x + CHUNK_WIDTH / 2 - position.x), (float)(cmPos.y + CHUNK_WIDTH / 2 - position.y), (float)(cmPos.z + CHUNK_WIDTH / 2 - position.z), 28.0f, gridFrustum)){
            if (cm->distance < fadeDist + 12.5){
                cm->inFrustum = 1;
                ChunkRenderer::drawBlocks(cm, program, position, VP);
            }
            else{
                cm->inFrustum = 0;
            }
        }
        else{
            cm->inFrustum = 0;
        }
    }
    glEnable(GL_CULL_FACE);

    program->unuse();
}

void OpenglManager::drawCutoutBlocks(const glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir)
{
    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("Cutout");
    program->use();

    glUniform1f(program->getUniform("lightType"), lightActive);

    glUniform3fv(program->getUniform("eyeNormalWorldspace"), 1, eyeDir);
    glUniform1f(program->getUniform("fogEnd"), (GLfloat)fogEnd);
    glUniform1f(program->getUniform("fogStart"), (GLfloat)fogStart);
    glUniform3fv(program->getUniform("fogColor"), 1, fogColor);
    glUniform3f(program->getUniform("lightType"), lightPos.x, lightPos.y, lightPos.z);
    glUniform1f(program->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(program->getUniform("alphaMult"), graphicsOptions.specularIntensity*0.3);

    bindBlockPacks();

    glUniform1f(program->getUniform("dt"), (GLfloat)bdt);

    glUniform1f(program->getUniform("sunVal"), sunVal);

    glUniform1f(program->getUniform("alphaMult"), 1.0f);

    float blockAmbient = 0.000f;
    glUniform3f(program->getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3f(program->getUniform("lightColor"), (GLfloat)lightColor.r, (GLfloat)lightColor.g, (GLfloat)lightColor.b);

    float fadeDist;
    if (NoChunkFade){
        fadeDist = (GLfloat)10000.0f;
    } else{
        fadeDist = (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f;
    }

    glUniform1f(program->getUniform("fadeDistance"), fadeDist);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    glLineWidth(3);

    glDisable(GL_CULL_FACE);

    glm::dvec3 cpos;

    static GLuint saveTicks = SDL_GetTicks();
    bool save = 0;
    if (SDL_GetTicks() - saveTicks >= 60000){ //save once per minute
        save = 1;
        saveTicks = SDL_GetTicks();
    }

    int mx, my, mz;
    double cx, cy, cz;
    double dx, dy, dz;
    mx = (int)position.x;
    my = (int)position.y;
    mz = (int)position.z;
    ChunkMesh *cm;

    for (int i = chunkMeshes.size() - 1; i >= 0; i--)
    {
        cm = chunkMeshes[i];

        if (cm->inFrustum){
            ChunkRenderer::drawCutoutBlocks(cm, program, position, VP);
        }
    }
    glEnable(GL_CULL_FACE);

    program->unuse();

}

void OpenglManager::drawTransparentBlocks(const glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir)
{
    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("Transparency");
    program->use();

    glUniform1f(program->getUniform("lightType"), lightActive);

    glUniform3fv(program->getUniform("eyeNormalWorldspace"), 1, eyeDir);
    glUniform1f(program->getUniform("fogEnd"), (GLfloat)fogEnd);
    glUniform1f(program->getUniform("fogStart"), (GLfloat)fogStart);
    glUniform3fv(program->getUniform("fogColor"), 1, fogColor);
    glUniform3f(program->getUniform("lightPosition_worldspace"), lightPos.x, lightPos.y, lightPos.z);
    glUniform1f(program->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(program->getUniform("specularIntensity"), graphicsOptions.specularIntensity*0.3);

    bindBlockPacks();

    glUniform1f(program->getUniform("dt"), (GLfloat)bdt);

    glUniform1f(program->getUniform("sunVal"), sunVal);

    glUniform1f(program->getUniform("alphaMult"), 1.0f);

    float blockAmbient = 0.000f;
    glUniform3f(program->getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3f(program->getUniform("lightColor"), (GLfloat)lightColor.r, (GLfloat)lightColor.g, (GLfloat)lightColor.b);

    float fadeDist;
    if (NoChunkFade){
        fadeDist = (GLfloat)10000.0f;
    } else{
        fadeDist = (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f;
    }

    glUniform1f(program->getUniform("fadeDistance"), fadeDist);


    glLineWidth(3);

    glDisable(GL_CULL_FACE);

    glm::dvec3 cpos;

    static GLuint saveTicks = SDL_GetTicks();
    bool save = 0;
    if (SDL_GetTicks() - saveTicks >= 60000){ //save once per minute
        save = 1;
        saveTicks = SDL_GetTicks();
    }

  
    ChunkMesh *cm;

    static i32v3 oldPos = i32v3(0);
    bool sort = false;

    i32v3 intPosition(fastFloor(position.x), fastFloor(position.y), fastFloor(position.z));

    if (oldPos != intPosition) {
        //sort the geometry
        sort = true;
        oldPos = intPosition;
    }

    for (int i = 0; i < chunkMeshes.size(); i++)
    {
        cm = chunkMeshes[i];
        if (sort) cm->needsSort = true;

        if (cm->inFrustum){  

            if (cm->needsSort) {
                cm->needsSort = false;
                if (cm->transQuadIndices.size() != 0) {
                    GeometrySorter::sortTransparentBlocks(cm, intPosition);
                    
                    //update index data buffer
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cm->transIndexID);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cm->transQuadIndices.size() * sizeof(ui32), NULL, GL_STATIC_DRAW);
                    void* v = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, cm->transQuadIndices.size() * sizeof(ui32), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
  
                    if (v == NULL) pError("Failed to map sorted transparency buffer.");
                    memcpy(v, &(cm->transQuadIndices[0]), cm->transQuadIndices.size() * sizeof(ui32));
                    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
                }
            }
            
            ChunkRenderer::drawTransparentBlocks(cm, program, position, VP);
        } 
    }
    glEnable(GL_CULL_FACE);

    program->unuse();

}

void OpenglManager::DrawPhysicsBlocks(glm::mat4& VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir)
{
    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("PhysicsBlocks");
    program->use();

    glUniform1f(program->getUniform("lightType"), lightActive);

    glUniform1f(program->getUniform("alphaMult"), 1.0f);

    glUniform3fv(program->getUniform("eyeNormalWorldspace"), 1, eyeDir);
    glUniform1f(program->getUniform("fogEnd"), (GLfloat)fogEnd);
    glUniform1f(program->getUniform("fogStart"), (GLfloat)fogStart);
    glUniform3fv(program->getUniform("fogColor"), 1, fogColor);
    glUniform3f(program->getUniform("lightPosition_worldspace"), lightPos.x, lightPos.y, lightPos.z);
    glUniform1f(program->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(program->getUniform("specularIntensity"), graphicsOptions.specularIntensity*0.3);

    bindBlockPacks();

    glUniform1f(program->getUniform("sunVal"), sunVal);

    float blockAmbient = 0.000f;
    glUniform3f(program->getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3f(program->getUniform("lightColor"), (GLfloat)lightColor.r, (GLfloat)lightColor.g, (GLfloat)lightColor.b);

    if (NoChunkFade){
        glUniform1f(program->getUniform("fadeDistance"), (GLfloat)10000.0f);
    }
    else{
        glUniform1f(program->getUniform("fadeDistance"), (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f);
    }

    for (Uint32 i = 0; i < physicsBlockMeshes.size(); i++){
        PhysicsBlockBatch::draw(physicsBlockMeshes[i], program, position, VP);
    }
    glVertexAttribDivisor(5, 0); //restore divisors
    glVertexAttribDivisor(6, 0);
    glVertexAttribDivisor(7, 0);
    program->unuse();
}

void OpenglManager::DrawWater(glm::mat4 &VP, const glm::dvec3 &position, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, glm::vec3 &lightPos, glm::vec3 &lightColor, bool underWater)
{
    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("Water");
    program->use();

    glUniform1f(program->getUniform("sunVal"), sunVal);

    glUniform1f(program->getUniform("FogEnd"), (GLfloat)fogEnd);
    glUniform1f(program->getUniform("FogStart"), (GLfloat)fogStart);
    glUniform3fv(program->getUniform("FogColor"), 1, fogColor);

    glUniform3fv(program->getUniform("LightPosition_worldspace"), 1, &(lightPos[0]));

    if (NoChunkFade){
        glUniform1f(program->getUniform("FadeDistance"), (GLfloat)10000.0f);
    }
    else{
        glUniform1f(program->getUniform("FadeDistance"), (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f);
    }

    float blockAmbient = 0.000f;
    glUniform3f(program->getUniform("AmbientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3f(program->getUniform("LightColor"), (GLfloat)lightColor.r, (GLfloat)lightColor.g, (GLfloat)lightColor.b);

    glUniform1f(program->getUniform("dt"), (GLfloat)bdt);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, waterNormalTexture.ID);
    glUniform1i(program->getUniform("normalMap"), 6);

    if (underWater) glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    ChunkMesh *cm;
    for (unsigned int i = 0; i < chunkMeshes.size(); i++) //they are sorted backwards??
    {
        cm = chunkMeshes[i];
    
        ChunkRenderer::drawWater(cm, program, position, VP);
    }

    glDepthMask(GL_TRUE);
    if (underWater) glEnable(GL_CULL_FACE);

    program->unuse();
}

const float crossWidth = 6;
const float crossThick = 1;
static GLfloat crossHairVerts1[8] = { 683 - crossWidth, 384 - crossThick, 683 + crossWidth, 384 - crossThick, 683 + crossWidth, 384 + crossThick, 683 - crossWidth, 384 + crossThick };
static GLfloat crossHairVerts2[8] = { 683 - crossThick, 384 - crossWidth, 683 + crossThick, 384 - crossWidth, 683 + crossThick, 384 + crossWidth, 683 - crossThick, 384 + crossWidth };

void OpenglManager::DrawHud()
{
    glDisable(GL_CULL_FACE);
    if (drawMode == 2){ //still draw text when in wireframe mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    char text[256];
    double multh = 1.5;
    if (graphicsOptions.hudMode == 0){
        sprintf(text, "FPS: %3.1f", glFps);
        hudTexts[0].update(text, 25, 650, 16);
        hudTexts[0].Draw();
        sprintf(text, "physics FPS: %3.1f", physicsFps);
        hudTexts[23].update(text, 25, 630, 16);
        hudTexts[23].Draw();

        glm::dvec3 normal = glm::normalize(player->worldPosition);
        glm::dvec3 worldPos = normal * (double)GameManager::voxelWorld->getPlanet()->radius;
        glm::dvec3 playerPos = normal * ((double)GameManager::voxelWorld->getPlanet()->radius + player->headPosition.y);
        double length = glm::length(playerPos - worldPos);

    
        sprintf(text, "Position (Blocks )");
        hudTexts[1].update(text, 750 * multh, 650, 16);
        hudTexts[1].Draw();
        sprintf(text, " X: %.0lf", player->headPosition.x);
        hudTexts[17].update(text, 750 * multh, 620, 16);
        hudTexts[17].Draw();
        sprintf(text, " Y: %.0lf", player->headPosition.y);
        hudTexts[18].update(text, 750 * multh, 590, 16);
        hudTexts[18].Draw();
        sprintf(text, " Z: %.0lf", player->headPosition.z);
        hudTexts[19].update(text, 750 * multh, 560, 16);
        hudTexts[19].Draw();
        sprintf(text, " WX: %.0lf", player->worldPosition.x);
        hudTexts[20].update(text, 750 * multh, 530, 16);
        hudTexts[20].Draw();
        sprintf(text, " WY: %.0lf", player->worldPosition.y);
        hudTexts[21].update(text, 750 * multh, 500, 16);
        hudTexts[21].Draw();
        sprintf(text, " WZ: %.0lf", player->worldPosition.z);
        hudTexts[22].update(text, 750 * multh, 470, 16);
        hudTexts[22].Draw();
        
        if (player->currBiome){
            sprintf(text, "Biome: %s", player->currBiome->name.c_str());
            hudTexts[6].update(text, 25, 470, 16);
            hudTexts[6].Draw();
            sprintf(text, "Temperature: %d", player->currTemp);
            hudTexts[7].update(text, 25, 430, 16);
            hudTexts[7].Draw();
            sprintf(text, "Humidity: %d", player->currHumidity);
            hudTexts[8].update(text, 25, 390, 16);
            hudTexts[8].Draw();
        }
        else{
            sprintf(text, "Biome: Detecting...");
            hudTexts[9].update(text, 25, 470, 16);
            hudTexts[9].Draw();
            sprintf(text, "Temperature: Detecting...");
            hudTexts[10].update(text, 25, 430, 16);
            hudTexts[10].Draw();
            sprintf(text, "Humidity: Detecting...");
            hudTexts[11].update(text, 25, 390, 16);
            hudTexts[11].Draw();
        }

        sprintf(text, "Velocity (m/s): %.3lf", glm::length(player->velocity) * 60 * 0.5);
        hudTexts[12].update(text, 25, 310, 16);
        hudTexts[12].Draw();

        if (player->rightEquippedItem){
            if (player->rightEquippedItem->count){
                sprintf(text, "Right Hand: %s  X %d", player->rightEquippedItem->name.c_str(), player->rightEquippedItem->count);
            }
            else{
                sprintf(text, "Right Hand: %s", player->rightEquippedItem->name.c_str());
            }
        }
        else{
            sprintf(text, "Right Hand: NONE");
        }
        hudTexts[15].update(text, 999 * 1.333, 35, 18, 2);
        hudTexts[15].Draw();
        
        if (player->leftEquippedItem){
            if (player->leftEquippedItem->count){
                sprintf(text, "Left Hand: %s  X %d", player->leftEquippedItem->name.c_str(), player->leftEquippedItem->count);
            }
            else{
                sprintf(text, "Left Hand: %s", player->leftEquippedItem->name.c_str());
            }
        }
        else{
            sprintf(text, "Left Hand: NONE");
        }
        hudTexts[16].update(text, 25, 35, 18);
        hudTexts[16].Draw();

        int scannedItem = player->scannedBlock;
        if (scannedItem == NONE){

        }
        else{
            hudTexts[24].update(GETBLOCK(scannedItem).name.c_str(), screenWidth2d / 2 + 5, screenHeight2d / 2 + 5, 15);
            hudTexts[24].Draw();
        }
    }

  //  if (graphicsOptions.hudMode != 2){
  //      DrawImage2D(crossHairVerts1, sizeof(crossHairVerts1), boxUVs, sizeof(boxUVs), boxDrawIndices, sizeof(boxDrawIndices), BlankTextureID.ID, glm::vec4(0.5, 0.0, 0.0, 0.6));
  //      DrawImage2D(crossHairVerts2, sizeof(crossHairVerts2), boxUVs, sizeof(boxUVs), boxDrawIndices, sizeof(boxDrawIndices), BlankTextureID.ID, glm::vec4(0.5, 0.0, 0.0, 0.6));
  //  }

    if (drawMode == 2){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    glEnable(GL_CULL_FACE);
}

void CalculateGlFps(Uint32 frametimes[10], Uint32 &frametimelast, Uint32 &framecount, float &framespersecond) {

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

    }
    else {

        count = 10;

    }

    framespersecond = 0;
    for (i = 0; i < count; i++) {
        framespersecond += frametimes[i];
    }

    framespersecond /= count;
    if (framespersecond > 0){
        framespersecond = 1000.f / framespersecond;
        glSpeedFactor = MIN(60.0f / framespersecond, 5); //this might be wrong...
    }
    else{
        glSpeedFactor = 1.0f;
    }
}

void RecursiveSortMeshList(vector <ChunkMesh*> &v, int start, int size)
{
    if (size < 2) return;
    int i, j;
    ChunkMesh *pivot, *mid, *last, *tmp;

    pivot = v[start];

    //end recursion when small enough
    if (size == 2){
        if ((pivot->distance) < (v[start + 1]->distance)){
            v[start] = v[start + 1];
            v[start + 1] = pivot;

            v[start]->vecIndex = start;
            v[start + 1]->vecIndex = start + 1;

        }
        return;
    }

    mid = v[start + size / 2];
    last = v[start + size - 1];

    //variables to minimize dereferences
    int md, ld, pd;
    pd = pivot->distance;
    md = mid->distance;
    ld = last->distance;

    //calculate pivot
    if ((pd > md && md > ld) || (pd < md && md < ld)){
        v[start] = mid;

        v[start + size / 2] = pivot;


        mid->vecIndex = start;
        pivot->vecIndex = start + size / 2;


        pivot = mid;
        pd = md;
    } else if ((pd > ld && ld > md) || (pd < ld && ld < md)){
        v[start] = last;

        v[start + size - 1] = pivot;


        last->vecIndex = start;
        pivot->vecIndex = start + size - 1;


        pivot = last;
        pd = ld;
    }

    i = start + 1;
    j = start + size - 1;

    //increment and decrement pointers until they are past each other
    while (i <= j){
        while (i < start + size - 1 && (v[i]->distance) > pd) i++;
        while (j > start + 1 && (v[j]->distance) < pd) j--;

        if (i <= j){
            tmp = v[i];
            v[i] = v[j];
            v[j] = tmp;


            v[i]->vecIndex = i;
            v[j]->vecIndex = j;


            i++;
            j--;
        }
    }

    //swap pivot with rightmost element in left set
    v[start] = v[j];
    v[j] = pivot;


    v[start]->vecIndex = start;
    v[j]->vecIndex = j;


    //sort the two subsets excluding the pivot point
    RecursiveSortMeshList(v, start, j - start);
    RecursiveSortMeshList(v, j + 1, start + size - j - 1);
}
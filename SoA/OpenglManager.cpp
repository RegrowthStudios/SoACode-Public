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
#include "VoxelWorld.h"
#include "MessageManager.h"

#include "utils.h"

vector <TextInfo> hudTexts;

OpenglManager openglManager;

bool glExit = 0;
bool getFrustum = 1;

void DrawGame();
void InitializeShaders();
void Initialize_SDL_OpenGL();
void InitializeObjects();
bool CheckGLError();
bool PlayerControl();
bool MainMenuControl();
void RebuildWindow();
void rebuildFrameBuffer();

OpenglManager::OpenglManager() : gameThread(NULL), frameBuffer(NULL), cameraInfo(NULL), zoomState(0)
{

}

OpenglManager::~OpenglManager()
{
    GameManager::gameState = GameStates::EXIT;
    EndThread();
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

void OpenglManager::FreeFrameBuffer()
{
    if (frameBuffer != NULL) delete frameBuffer;
    frameBuffer = NULL;
}

void DrawGame()
{

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
    graphicsOptions.screenWidth = w;
    graphicsOptions.screenHeight = h;

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
  //  openglManager.InitializeFrameBuffer();


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
            GameManager::player->inventory.push_back(new Item(i, 9999999, Blocks[i].name, ITEM_BLOCK, 0, 0, 0));
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

bool PlayerControl() {
  
    //isMouseIn = 1;
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    //SDL_Event evnt;
    //while (SDL_PollEvent(&evnt))
    //{
    //    switch (evnt.type)
    //    {
    //    case SDL_QUIT:
    //        return 0;
    //    case SDL_MOUSEMOTION:
    //        if (GameManager::gameState == GameStates::PLAY){
    //            GameManager::player->mouseMove(evnt.motion.xrel, evnt.motion.yrel);
    //        }
    //        break;
    //    case SDL_MOUSEBUTTONDOWN:
    //        GameManager::inputManager->pushEvent(evnt);
    //        break;
    //    case SDL_MOUSEBUTTONUP:
    //        GameManager::inputManager->pushEvent(evnt);
    //        break;
    //    case SDL_KEYDOWN:
    //        GameManager::inputManager->pushEvent(evnt);
    //        break;
    //    case SDL_KEYUP:
    //        GameManager::inputManager->pushEvent(evnt);
    //        break;
    //    case SDL_MOUSEWHEEL:
    //        GameManager::inputManager->pushEvent(evnt);
    //        break;
    //    case SDL_WINDOWEVENT:
    //        if (evnt.window.type == SDL_WINDOWEVENT_LEAVE || evnt.window.type == SDL_WINDOWEVENT_FOCUS_LOST){
    //            //            GameState = PAUSE;
    //            //            SDL_SetRelativeMouseMode(SDL_FALSE);
    //            //            isMouseIn = 0;
    //        }
    //        break;
    //    }
    //}
    //

    //bool leftPress = GameManager::inputManager->getKeyDown(INPUT_MOUSE_LEFT);
    //bool rightPress = GameManager::inputManager->getKeyDown(INPUT_MOUSE_RIGHT);
    //bool leftRelease = GameManager::inputManager->getKeyUp(INPUT_MOUSE_LEFT);
    //bool rightRelease = GameManager::inputManager->getKeyUp(INPUT_MOUSE_RIGHT);

    //clickDragActive = 0;
    //if (GameManager::inputManager->getKey(INPUT_BLOCK_SELECT)){
    //    clickDragActive = 1;
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_SCAN_WSO)) {
    //    GameManager::scanWSO();
    //}

    //static bool rightMousePressed = false;
    //if (GameManager::inputManager->getKey(INPUT_MOUSE_RIGHT)){
    //    if (!rightMousePressed || GameManager::voxelEditor->isEditing()) {
    //        if (!(GameManager::player->rightEquippedItem)){
    //            if (rightPress || clickDragActive) GameManager::clickDragRay(true);
    //        } else if (GameManager::player->rightEquippedItem->type == ITEM_BLOCK) {
    //            GameManager::player->dragBlock = GameManager::player->rightEquippedItem;
    //            if (rightPress || clickDragActive) GameManager::clickDragRay(false);
    //        }

    //        rightMousePressed = true;
    //    }
    //} else {
    //    rightMousePressed = false;
    //}

    //static bool leftMousePressed = false;
    //if (GameManager::inputManager->getKey(INPUT_MOUSE_LEFT)){

    //    if (!leftMousePressed || GameManager::voxelEditor->isEditing()) {
    //        if (!(GameManager::player->leftEquippedItem)){
    //            if (leftPress || clickDragActive) GameManager::clickDragRay(true);
    //        } else if (GameManager::player->leftEquippedItem->type == ITEM_BLOCK){
    //            GameManager::player->dragBlock = GameManager::player->leftEquippedItem;

    //            if (leftPress || clickDragActive) GameManager::clickDragRay(false);
    //        }

    //        leftMousePressed = true;
    //    }
    //} else {
    //    leftMousePressed = false;
    //}

    //if (rightRelease && GameManager::voxelEditor->isEditing()){
    //    if ((GameManager::player->rightEquippedItem && GameManager::player->rightEquippedItem->type == ITEM_BLOCK) || !GameManager::player->rightEquippedItem){
    //        GameManager::messageManager->enqueue(ThreadName::PHYSICS,
    //                                             Message(MessageID::CHUNK_MESH,
    //                                             (void *)cmd));
    //        glToGame.enqueue(OMessage(GL_M_PLACEBLOCKS, new PlaceBlocksMessage(GameManager::player->rightEquippedItem)));
    //    }
    //    GameManager::player->dragBlock = NULL;
    //}

    //if (leftRelease && GameManager::voxelEditor->isEditing()){
    //    if ((GameManager::player->leftEquippedItem && GameManager::player->leftEquippedItem->type == ITEM_BLOCK) || !GameManager::player->leftEquippedItem){ //if he has a block or nothing for place or break
    //        //error checking for unloaded chunks
    //        glToGame.enqueue(OMessage(GL_M_PLACEBLOCKS, new PlaceBlocksMessage(GameManager::player->leftEquippedItem)));
    //    }
    //    GameManager::player->dragBlock = NULL;
    //}

    //if (GameManager::inputManager->getKeyDown(INPUT_FLY)){
    //    GameManager::player->flyToggle();
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_HUD)){
    //    graphicsOptions.hudMode++;
    //    if (graphicsOptions.hudMode == 3) graphicsOptions.hudMode = 0;
    //}
    //
    //if (GameManager::inputManager->getKeyDown(INPUT_WATER_UPDATE)){
    //    isWaterUpdating = !isWaterUpdating;
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_FLASH_LIGHT)){
    //    GameManager::player->lightActive++;
    //    if (GameManager::player->lightActive == 3){
    //        GameManager::player->lightActive = 0;
    //    }
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_PHYSICS_BLOCK_UPDATES)){
    //    globalDebug2 = !globalDebug2;
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_UPDATE_FRUSTUM)){
    //    getFrustum = !getFrustum;
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_DEBUG)){
    //    debugVarh = !debugVarh;
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_SONAR)){
    //    sonarActive = !sonarActive;
    //    sonarDt = 0;
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_PLANET_DRAW_MODE)){
    //    planetDrawMode += 1.0f;
    //    if (planetDrawMode == 3.0f) planetDrawMode = 0.0f;
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_GRID)){
    //    gridState = !gridState;
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_RELOAD_SHADERS)){
    //    InitializeShaders();
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_DRAW_MODE)){
    //    if (++drawMode == 3) drawMode = 0;
    //    if (drawMode == 0){
    //        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Solid mode
    //    }
    //    else if (drawMode == 1){
    //        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); //Point mode
    //    }
    //    else if (drawMode == 2){
    //        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Wireframe mode
    //    }
    //}
   
    //if (GameManager::inputManager->getKeyDown(INPUT_INVENTORY)){
    //    GameManager::gameState = GameStates::INVENTORY;
    //    isMouseIn = 0;
    //    GameManager::savePlayerState();
    //    SDL_SetRelativeMouseMode(SDL_FALSE);
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_ZOOM)){
    //    if (GameManager::gameState == GameStates::PLAY) GameManager::player->zoom();
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_RELOAD_TEXTURES)){
    //    ReloadTextures();
    //    GameManager::chunkManager->remeshAllChunks();
    //}
    //if (GameManager::inputManager->getKeyDown(INPUT_RELOAD_BLOCKS)){
    //    if (!(fileManager.loadBlocks("Data/BlockData.ini"))){
    //        pError("Failed to reload blocks.");
    //        return 0;
    //    }
    //    for (size_t i = 1; i < Blocks.size(); i++){
    //        if (ObjectList[i] != NULL){
    //            delete ObjectList[i];
    //            ObjectList[i] = NULL;
    //        }
    //    }
    //    for (size_t i = 0; i < GameManager::player->inventory.size(); i++){
    //        delete GameManager::player->inventory[i];
    //    }
    //    GameManager::player->inventory.clear();
    //    InitializeObjects();
    //}
    return false;
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
        graphicsOptions.screenWidth = w;
        graphicsOptions.screenHeight = h;

        SDL_GL_MakeCurrent(mainWindow, mainOpenGLContext);

        graphicsOptions.needsWindowReload = 0;
    }

    rebuildFrameBuffer();

    //SDL_DestroyWindow(tmp); //destroy the window only after initializing a new one
}

void rebuildFrameBuffer()
{
    openglManager.FreeFrameBuffer();
    //openglManager.InitializeFrameBuffer();
    graphicsOptions.needsFboReload = 0;
}

void OpenglManager::UpdateMeshDistances()
{
    ChunkMesh *cm;
    int mx, my, mz;
    double dx, dy, dz;
    double cx, cy, cz;

    mx = (int)GameManager::player->headPosition.x;
    my = (int)GameManager::player->headPosition.y;
    mz = (int)GameManager::player->headPosition.z;
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

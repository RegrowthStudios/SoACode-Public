#include "stdafx.h"
#include "GamePlayScreen.h"

#include <Vorb/colors.h>
#include <Vorb/Events.hpp>
#include <Vorb/GpuMemory.h>
#include <Vorb/SpriteFont.h>
#include <Vorb/SpriteBatch.h>

#include "Player.h"
#include "App.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Sound.h"
#include "MessageManager.h"
#include "SphericalTerrainPatch.h"
#include "MeshManager.h"
#include "ChunkMesh.h"
#include "ParticleMesh.h"
#include "PhysicsBlocks.h"
#include "RenderTask.h"
#include "Errors.h"
#include "ChunkRenderer.h"
#include "ChunkManager.h"
#include "VoxelWorld.h"
#include "VoxelEditor.h"
#include "DebugRenderer.h"
#include "Collision.h"
#include "Inputs.h"
#include "TexturePackLoader.h"
#include "LoadTaskShaders.h"
#include "Options.h"
#include "GamePlayScreenEvents.hpp"

#define THREAD ThreadId::UPDATE

CTOR_APP_SCREEN_DEF(GamePlayScreen, App),
    m_updateThread(nullptr),
    m_threadRunning(false), 
    m_inFocus(true),
    m_onPauseKeyDown(nullptr),
    m_onFlyKeyDown(nullptr),
    m_onGridKeyDown(nullptr),
    m_onReloadTexturesKeyDown(nullptr),
    m_onReloadShadersKeyDown(nullptr),
    m_onInventoryKeyDown(nullptr),
    m_onReloadUIKeyDown(nullptr),
    m_onHUDKeyDown(nullptr) {
    // Empty
}

i32 GamePlayScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 GamePlayScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

//#define SUBSCRIBE(ID, CLASS, VAR) \
//    VAR = inputManager->subscribe(ID, InputManager::EventType::DOWN, new CLASS(this));

void GamePlayScreen::build() {
    // Empty
}

void GamePlayScreen::destroy(const GameTime& gameTime) {
    // Destruction happens in onExit
}

void GamePlayScreen::onEntry(const GameTime& gameTime) {

    m_player = GameManager::player;
    m_player->initialize("Ben", _app->getWindow().getAspectRatio()); //What an awesome name that is
    GameManager::initializeVoxelWorld(m_player);

    // Initialize the PDA
    m_pda.init(this);

    // Initialize the Pause Menu
    m_pauseMenu.init(this);

    // Set up the rendering
    initRenderPipeline();

    // Initialize and run the update thread
    m_updateThread = new std::thread(&GamePlayScreen::updateThreadFunc, this);

    // Force him to fly... This is temporary
    m_player->isFlying = true;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    InputManager* inputManager = _app->inputManager;
    m_onPauseKeyDown = inputManager->subscribe(INPUT_PAUSE, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnPauseKeyDown(this));
    m_onFlyKeyDown = inputManager->subscribe(INPUT_FLY, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnFlyKeyDown(this));
    m_onGridKeyDown = inputManager->subscribe(INPUT_GRID, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnGridKeyDown(this));
    m_onReloadTexturesKeyDown = inputManager->subscribe(INPUT_RELOAD_TEXTURES, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnReloadTexturesKeyDown(this));
    m_onReloadShadersKeyDown = inputManager->subscribe(INPUT_RELOAD_SHADERS, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnReloadShadersKeyDown(this));
    m_onInventoryKeyDown = inputManager->subscribe(INPUT_INVENTORY, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnInventoryKeyDown(this));
    m_onReloadUIKeyDown = inputManager->subscribe(INPUT_RELOAD_UI, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnReloadUIKeyDown(this));
    m_onHUDKeyDown = inputManager->subscribe(INPUT_HUD, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnHUDKeyDown(this));
    m_onNightVisionToggle = inputManager->subscribeFunctor(INPUT_NIGHT_VISION, InputManager::EventType::DOWN, [&] (void* s, ui32 a) -> void {
        if (isInGame()) {
            m_renderPipeline.toggleNightVision();
        }
    });
    m_onNightVisionReload = inputManager->subscribeFunctor(INPUT_NIGHT_VISION_RELOAD, InputManager::EventType::DOWN, [&] (void* s, ui32 a) -> void {
        m_renderPipeline.loadNightVision();
    });
    m_onDrawMode = inputManager->subscribeFunctor(INPUT_DRAW_MODE, InputManager::EventType::DOWN, [&] (void* s, ui32 a) -> void {
        m_renderPipeline.cycleDrawMode();
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onMotion, [&] (void* s, const vui::MouseMotionEvent& e) {
        if (m_inFocus) {
            // Pass mouse motion to the player
            m_player->mouseMove(e.dx, e.dy);
        }
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onButtonDown, [&] (void* s, const vui::MouseButtonEvent& e) {
        if (isInGame()) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            m_inFocus = true;
        }
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onButtonUp, [&] (void* s, const vui::MouseButtonEvent& e) {
        if (GameManager::voxelEditor->isEditing()) {
            if (e.button == vui::MouseButton::LEFT) {
                GameManager::voxelEditor->editVoxels(m_player->leftEquippedItem);
                puts("EDIT VOXELS");
            } else if (e.button == vui::MouseButton::RIGHT) {
                GameManager::voxelEditor->editVoxels(m_player->rightEquippedItem);
                puts("EDIT VOXELS");
            }
        }
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onFocusGained, [&] (void* s, const vui::MouseEvent& e) {
        m_inFocus = true;
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onFocusLost, [&] (void* s, const vui::MouseEvent& e) {
        m_inFocus = false;
    });

    _app->inputManager->startInput();
}

void GamePlayScreen::onExit(const GameTime& gameTime) {
    
    InputManager* inputManager = _app->inputManager;

    inputManager->stopInput();
    m_hooks.dispose();

    
    inputManager->unsubscribe(INPUT_PAUSE, InputManager::EventType::DOWN, m_onPauseKeyDown);
    delete m_onPauseKeyDown;

    inputManager->unsubscribe(INPUT_FLY, InputManager::EventType::DOWN, m_onFlyKeyDown);
    delete m_onFlyKeyDown;

    inputManager->unsubscribe(INPUT_GRID, InputManager::EventType::DOWN, m_onGridKeyDown);
    delete m_onGridKeyDown;

    inputManager->unsubscribe(INPUT_RELOAD_TEXTURES, InputManager::EventType::DOWN, m_onReloadTexturesKeyDown);
    delete m_onReloadTexturesKeyDown;

    inputManager->unsubscribe(INPUT_RELOAD_SHADERS, InputManager::EventType::DOWN, m_onReloadShadersKeyDown);
    delete m_onReloadShadersKeyDown;

    inputManager->unsubscribe(INPUT_INVENTORY, InputManager::EventType::DOWN, m_onInventoryKeyDown);
    delete m_onInventoryKeyDown;

    inputManager->unsubscribe(INPUT_RELOAD_UI, InputManager::EventType::DOWN, m_onReloadUIKeyDown);
    delete m_onReloadUIKeyDown;

    inputManager->unsubscribe(INPUT_HUD, InputManager::EventType::DOWN, m_onHUDKeyDown);
    delete m_onHUDKeyDown;

    inputManager->unsubscribe(INPUT_NIGHT_VISION, InputManager::EventType::DOWN, m_onNightVisionToggle);
    delete m_onNightVisionToggle;

    inputManager->unsubscribe(INPUT_NIGHT_VISION_RELOAD, InputManager::EventType::DOWN, m_onNightVisionReload);
    delete m_onNightVisionReload;

    inputManager->unsubscribe(INPUT_DRAW_MODE, InputManager::EventType::DOWN, m_onDrawMode);
    delete m_onDrawMode;

    m_threadRunning = false;
    m_updateThread->join();
    delete m_updateThread;
    delete m_player;
    delete m_voxelWorld;
    _app->meshManager->destroy();
    m_pda.destroy();
    m_renderPipeline.destroy();
    m_pauseMenu.destroy();
}

void GamePlayScreen::onEvent(const SDL_Event& e) {
    // Empty
}

void GamePlayScreen::update(const GameTime& gameTime) {

    // TEMPORARY TIMESTEP TODO(Ben): Get rid of this damn global
    if (_app->getFps()) {
        glSpeedFactor = 60.0f / _app->getFps();
        if (glSpeedFactor > 3.0f) { // Cap variable timestep at 20fps
            glSpeedFactor = 3.0f;
        }
    }
    
    // Update the input
    handleInput();

    // Update the player
    updatePlayer();

    // Update the PDA
    if (m_pda.isOpen()) m_pda.update();

    // Updates the Pause Menu
    if (m_pauseMenu.isOpen()) m_pauseMenu.update();

    // Sort all meshes // TODO(Ben): There is redundancy here
    _app->meshManager->sortMeshes(m_player->headPosition);

    // Process any updates from the render thread
    processMessages();
}

void GamePlayScreen::draw(const GameTime& gameTime) {

    updateWorldCameraClip();

    m_renderPipeline.render();
}

void GamePlayScreen::unPause() { 
    m_pauseMenu.close(); 
    SDL_SetRelativeMouseMode(SDL_TRUE);
    m_inFocus = true;
}

i32 GamePlayScreen::getWindowWidth() const {
    return _app->getWindow().getWidth();
}

i32 GamePlayScreen::getWindowHeight() const {
    return _app->getWindow().getHeight();
}

void GamePlayScreen::initVoxels() {
    bool atSurface = 1;
    m_player = new Player;

 
    if (fileManager.loadPlayerFile(player)) {
        atSurface = 0; //dont need to set height
    }

    m_voxelWorld = new VoxelWorld;
    m_voxelWorld->initialize(player->facePosition, &player->voxelMapData, 0);

    if (atSurface) player->facePosition.y = 0;// voxelWorld->getCenterY();

    player->gridPosition = player->facePosition;

    //   player->setNearestPlanet(planet->scaledRadius, planet->atmosphere.radius, planet->facecsGridWidth);

    //   double dist = player->facePosition.y + planet->radius;
    //  player->update(1, planet->getGravityAccel(dist), planet->getAirFrictionForce(dist, glm::length(player->velocity)));
}

void GamePlayScreen::initRenderPipeline() {
    // Set up the rendering pipeline and pass in dependencies
    ui32v4 viewport(0, 0, _app->getWindow().getViewportDims());
    m_renderPipeline.init(viewport, &m_player->getChunkCamera(), &m_player->getWorldCamera(), 
                         _app, m_player, _app->meshManager, &m_pda, GameManager::glProgramManager,
                         &m_pauseMenu, m_voxelWorld->getChunkManager().getChunkSlots(0));
}

void GamePlayScreen::handleInput() {
    // Get input manager handle
    InputManager* inputManager = _app->inputManager;

    // Block placement
    if (isInGame()) {
        if (inputManager->getKeyDown(INPUT_MOUSE_LEFT) || (GameManager::voxelEditor->isEditing() && inputManager->getKey(INPUT_BLOCK_DRAG))) {
            if (!(m_player->leftEquippedItem)){
                GameManager::clickDragRay(true);
            } else if (m_player->leftEquippedItem->type == ITEM_BLOCK){
                m_player->dragBlock = m_player->leftEquippedItem;
                GameManager::clickDragRay(false);
            }
        } else if (inputManager->getKeyDown(INPUT_MOUSE_RIGHT) || (GameManager::voxelEditor->isEditing() && inputManager->getKey(INPUT_BLOCK_DRAG))) {
            if (!(m_player->rightEquippedItem)){
                GameManager::clickDragRay(true);
            } else if (m_player->rightEquippedItem->type == ITEM_BLOCK){
                m_player->dragBlock = m_player->rightEquippedItem;
                GameManager::clickDragRay(false);
            }
        }
    }

    // Update inputManager internal state
    inputManager->update();
}

void GamePlayScreen::updatePlayer() {
  //  double dist = _player->facePosition.y + GameManager::planet->getRadius();
  ////  _player->update(_inFocus, GameManager::planet->getGravityAccel(dist), GameManager::planet->getAirFrictionForce(dist, glm::length(_player->velocity)));

  //  Chunk **chunks = new Chunk*[8];
  //  _player->isGrounded = 0;
  //  _player->setMoveMod(1.0f);
  //  _player->canCling = 0;
  //  _player->collisionData.yDecel = 0.0f;

  //  // Number of steps to integrate the collision over
  //  for (int i = 0; i < PLAYER_COLLISION_STEPS; i++){
  //      _player->gridPosition += (_player->velocity / (float)PLAYER_COLLISION_STEPS) * glSpeedFactor;
  //      _player->facePosition += (_player->velocity / (float)PLAYER_COLLISION_STEPS) * glSpeedFactor;
  //      _player->collisionData.clear();
  //      GameManager::voxelWorld->getClosestChunks(_player->gridPosition, chunks); //DANGER HERE!
  //      aabbChunkCollision(_player, &(_player->gridPosition), chunks, 8);
  //      _player->applyCollisionData();
  //  }

  //  delete[] chunks;
}

void GamePlayScreen::updateThreadFunc() {
    m_threadRunning = true;

    FpsLimiter fpsLimiter;
    fpsLimiter.init(maxPhysicsFps);

    MessageManager* messageManager = GameManager::messageManager;

    Message message;

    static int saveStateTicks = SDL_GetTicks();

    while (m_threadRunning) {
        fpsLimiter.beginFrame();

        GameManager::soundEngine->SetMusicVolume(soundOptions.musicVolume / 100.0f);
        GameManager::soundEngine->SetEffectVolume(soundOptions.effectVolume / 100.0f);
        GameManager::soundEngine->update();

        //while (messageManager->tryDeque(THREAD, message)) {
            // Process the message
        //    switch (message.id) {
                
        //    }
       // }




        HeightData tmpHeightData;
        if (!voxelWorld->getChunkManager().getPositionHeightData((int)player->headPosition.x, (int)player->headPosition.z, tmpHeightData)) {
            player->currBiome = tmpHeightData.biome;
            player->currTemp = tmpHeightData.temperature;
            player->currHumidity = tmpHeightData.rainfall;
        } else {
            player->currBiome = NULL;
            player->currTemp = -1;
            player->currHumidity = -1;
        }

        voxelWorld->update(&player->getChunkCamera());

        if (inputManager->getKey(INPUT_BLOCK_SCANNER)) {
            player->scannedBlock = voxelWorld->getChunkManager().getBlockFromDir(glm::dvec3(player->chunkDirection()), player->headPosition);
        } else {
            player->scannedBlock = NONE;
        }

        particleEngine.update();

        if (SDL_GetTicks() - saveStateTicks >= 20000) {
            saveStateTicks = SDL_GetTicks();
            savePlayerState();
        }

        physicsFps = fpsLimiter.endFrame();
    }
}

void GamePlayScreen::processMessages() {

    TerrainMeshMessage* tmm;
    int j = 0,k = 0;
    MeshManager* meshManager = _app->meshManager;
    ChunkMesh* cm;
    PreciseTimer timer;
    timer.start();
    int numMessages = GameManager::messageManager->tryDequeMultiple(ThreadId::RENDERING, messageBuffer, MESSAGES_PER_FRAME);
    std::set<ChunkMesh*> updatedMeshes; // Keep track of which meshes we have already seen so we can ignore older duplicates
    for (int i = numMessages - 1; i >= 0; i--) {
        Message& message = messageBuffer[i];
        switch (message.id) {
            case MessageID::CHUNK_MESH:
                j++;
                cm = ((ChunkMeshData *)(message.data))->chunkMesh;
                if (updatedMeshes.find(cm) == updatedMeshes.end()) {
                    k++;
                    updatedMeshes.insert(cm);
                    meshManager->updateChunkMesh((ChunkMeshData *)(message.data));
                } else {
                    delete message.data;
                }
                break;
            default:
                break;
        }
    }

    for (int i = 0; i < numMessages; i++) {
        Message& message = messageBuffer[i];
        switch (message.id) {
            case MessageID::TERRAIN_MESH:
          /*      tmm = static_cast<TerrainMeshMessage*>(message.data);
                meshManager->updateTerrainMesh(tmm);*/
                break;
            case MessageID::REMOVE_TREES:
  /*              tmm = static_cast<TerrainMeshMessage*>(message.data);
                if (tmm->terrainBuffers->treeVboID != 0) glDeleteBuffers(1, &(tmm->terrainBuffers->treeVboID));
                tmm->terrainBuffers->treeVboID = 0;
                delete tmm;*/
                break;
            case MessageID::PARTICLE_MESH:
                meshManager->updateParticleMesh((ParticleMeshMessage *)(message.data));
                break;
            case MessageID::PHYSICS_BLOCK_MESH:
                meshManager->updatePhysicsBlockMesh((PhysicsBlockMeshMessage *)(message.data));
                break;
            default:
                break;
        }
    }
}

void GamePlayScreen::updateWorldCameraClip() {
    //far znear for maximum Terrain Patch z buffer precision
    //this is currently incorrect
    double nearClip = MIN((csGridWidth / 2.0 - 3.0)*32.0*0.7, 75.0) - ((double)(30.0) / (double)(csGridWidth*csGridWidth*csGridWidth))*55.0;
    if (nearClip < 0.1) nearClip = 0.1;
    double a = 0.0;
    // TODO(Ben): This is crap fix it (Sorry Brian)
    a = closestTerrainPatchDistance / (sqrt(1.0f + pow(tan(graphicsOptions.fov / 2.0), 2.0) * (pow((double)_app->getWindow().getAspectRatio(), 2.0) + 1.0))*2.0);
    if (a < 0) a = 0;

    double clip = MAX(nearClip / planetScale * 0.5, a);
    // The world camera has a dynamic clipping plane
    m_player->getWorldCamera().setClippingPlane(clip, MAX(300000000.0 / planetScale, closestTerrainPatchDistance + 10000000));
    m_player->getWorldCamera().updateProjection();
}

bool GamePlayScreen::loadPlayerFile(const cString filePath, Player* player) {
    //loadMarkers(player);
    
    FILE *file = NULL;
    file = fopen((GameManager::saveFilePath + "/Players/" + player->getName() + ".dat").c_str(), "rb");
    if (file == NULL) {
        //file doesnt exist so set spawn to random
        srand(time(NULL));
        int spawnFace = rand() % 4 + 1;
        player->voxelMapData.face = spawnFace;
        return 0;
    }
    GLubyte buffer[2048];
    int n;
    n = fread(buffer, 1, 1024, file);
    int byte = 0;
    player->facePosition.x = BufferUtils::extractFloat(buffer, (byte++) * 4);
    player->facePosition.y = BufferUtils::extractFloat(buffer, (byte++) * 4);
    player->facePosition.z = BufferUtils::extractFloat(buffer, (byte++) * 4);
    player->voxelMapData.face = BufferUtils::extractInt(buffer, (byte++) * 4);
    player->getChunkCamera().setYawAngle(BufferUtils::extractFloat(buffer, (byte++) * 4));
    player->getChunkCamera().setPitchAngle(BufferUtils::extractFloat(buffer, (byte++) * 4));
    player->isFlying = BufferUtils::extractBool(buffer, byte * 4);
    fclose(file);
    player->voxelMapData.ipos = fastFloor(player->facePosition.z / (double)CHUNK_WIDTH);
    player->voxelMapData.jpos = fastFloor(player->facePosition.x / (double)CHUNK_WIDTH);
    return 1;
}

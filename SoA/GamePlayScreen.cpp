#include "stdafx.h"
#include "GamePlayScreen.h"

#include <Vorb/colors.h>
#include <Vorb/Events.hpp>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/graphics/SpriteBatch.h>

#include "App.h"
#include "ChunkManager.h"
#include "ChunkMesh.h"
#include "ChunkRenderer.h"
#include "Collision.h"
#include "DebugRenderer.h"
#include "Errors.h"
#include "GameManager.h"
#include "GamePlayScreenEvents.hpp"
#include "GameSystem.h"
#include "GameSystemUpdater.h"
#include "InputManager.h"
#include "Inputs.h"
#include "LoadTaskShaders.h"
#include "MainMenuScreen.h"
#include "MeshManager.h"
#include "MessageManager.h"
#include "Options.h"
#include "ParticleMesh.h"
#include "PhysicsBlocks.h"
#include "RenderTask.h"
#include "SoaState.h"
#include "Sound.h"
#include "SpaceSystem.h"
#include "SphericalTerrainPatch.h"
#include "TexturePackLoader.h"
#include "VoxelEditor.h"

#define THREAD ThreadId::UPDATE

GamePlayScreen::GamePlayScreen(const App* app, const MainMenuScreen* mainMenuScreen) :
    IAppScreen<App>(app),
    m_mainMenuScreen(mainMenuScreen),
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

GamePlayScreen::~GamePlayScreen() {
    // Empty
}

i32 GamePlayScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 GamePlayScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}


void GamePlayScreen::build() {
    // Empty
}

void GamePlayScreen::destroy(const GameTime& gameTime) {
    // Destruction happens in onExit
}

void GamePlayScreen::onEntry(const GameTime& gameTime) {

    m_inputManager = new InputManager;
    initInputs(m_inputManager);

    m_soaState = m_mainMenuScreen->getSoAState();

    controller.startGame(m_soaState);

    m_gameSystemUpdater = std::make_unique<GameSystemUpdater>(&m_soaState->gameSystem, m_inputManager);

    //m_player = new Player;
    //m_player->initialize("Ben", _app->getWindow().getAspectRatio()); //What an awesome name that is
    //if (m_gameStartState->isNewGame) {
    //    m_player->voxelMapData.face = m_gameStartState->startFace;
    //    m_player->voxelMapData.ipos = m_gameStartState->startGridPos.z;
    //    m_player->voxelMapData.jpos = m_gameStartState->startGridPos.x;
    //}
    initVoxels();

    // Initialize the PDA
    m_pda.init(this, m_soaState->glProgramManager.get());

    // Initialize the Pause Menu
    m_pauseMenu.init(this, m_soaState->glProgramManager.get());

    // Set up the rendering
    initRenderPipeline();

    // Initialize and run the update thread
    m_updateThread = new std::thread(&GamePlayScreen::updateThreadFunc, this);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    m_onPauseKeyDown = m_inputManager->subscribe(INPUT_PAUSE, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnPauseKeyDown(this));
    m_onFlyKeyDown = m_inputManager->subscribe(INPUT_FLY, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnFlyKeyDown(this));
    m_onGridKeyDown = m_inputManager->subscribe(INPUT_GRID, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnGridKeyDown(this));
    m_onReloadTexturesKeyDown = m_inputManager->subscribe(INPUT_RELOAD_TEXTURES, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnReloadTexturesKeyDown(this));
    m_onReloadShadersKeyDown = m_inputManager->subscribe(INPUT_RELOAD_SHADERS, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnReloadShadersKeyDown(this));
    m_onInventoryKeyDown = m_inputManager->subscribe(INPUT_INVENTORY, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnInventoryKeyDown(this));
    m_onReloadUIKeyDown = m_inputManager->subscribe(INPUT_RELOAD_UI, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnReloadUIKeyDown(this));
    m_onHUDKeyDown = m_inputManager->subscribe(INPUT_HUD, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnHUDKeyDown(this));
    m_onNightVisionToggle = m_inputManager->subscribeFunctor(INPUT_NIGHT_VISION, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        if (isInGame()) {
            m_renderPipeline.toggleNightVision();
        }
    });

    m_onNightVisionReload = m_inputManager->subscribeFunctor(INPUT_NIGHT_VISION_RELOAD, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        m_renderPipeline.loadNightVision();
    });
    m_onDrawMode = m_inputManager->subscribeFunctor(INPUT_DRAW_MODE, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        m_renderPipeline.cycleDrawMode();
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onMotion, [&](Sender s, const vui::MouseMotionEvent& e) {
        if (m_inFocus) {
            // Pass mouse motion to the player
           // m_player->mouseMove(e.dx, e.dy);
        }
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onButtonDown, [&] (Sender s, const vui::MouseButtonEvent& e) {
        if (isInGame()) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            m_inFocus = true;
        }
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onButtonUp, [&] (Sender s, const vui::MouseButtonEvent& e) {
        if (GameManager::voxelEditor->isEditing()) {
        /*    if (e.button == vui::MouseButton::LEFT) {
                GameManager::voxelEditor->editVoxels(&m_voxelWorld->getChunkManager(),
                                                     m_voxelWorld->getPhysicsEngine(),
                                                     m_player->leftEquippedItem);
                puts("EDIT VOXELS");
            } else if (e.button == vui::MouseButton::RIGHT) {
                GameManager::voxelEditor->editVoxels(&m_voxelWorld->getChunkManager(),
                                                     m_voxelWorld->getPhysicsEngine(),
                                                     m_player->rightEquippedItem);
                puts("EDIT VOXELS");
            }*/
        }
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onFocusGained, [&](Sender s, const vui::MouseEvent& e) {
        m_inFocus = true;
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onFocusLost, [&](Sender s, const vui::MouseEvent& e) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        m_inFocus = false;
    });

    m_inputManager->startInput();
}

void GamePlayScreen::onExit(const GameTime& gameTime) {

    m_inputManager->stopInput();
    m_hooks.dispose();

    // Easy way to clear everything
    m_soaState->gameSystem = GameSystem();
    
    m_inputManager->unsubscribe(INPUT_PAUSE, InputManager::EventType::DOWN, m_onPauseKeyDown);
    delete m_onPauseKeyDown;

    m_inputManager->unsubscribe(INPUT_FLY, InputManager::EventType::DOWN, m_onFlyKeyDown);
    delete m_onFlyKeyDown;

    m_inputManager->unsubscribe(INPUT_GRID, InputManager::EventType::DOWN, m_onGridKeyDown);
    delete m_onGridKeyDown;

    m_inputManager->unsubscribe(INPUT_RELOAD_TEXTURES, InputManager::EventType::DOWN, m_onReloadTexturesKeyDown);
    delete m_onReloadTexturesKeyDown;

    m_inputManager->unsubscribe(INPUT_RELOAD_SHADERS, InputManager::EventType::DOWN, m_onReloadShadersKeyDown);
    delete m_onReloadShadersKeyDown;

    m_inputManager->unsubscribe(INPUT_INVENTORY, InputManager::EventType::DOWN, m_onInventoryKeyDown);
    delete m_onInventoryKeyDown;

    m_inputManager->unsubscribe(INPUT_RELOAD_UI, InputManager::EventType::DOWN, m_onReloadUIKeyDown);
    delete m_onReloadUIKeyDown;

    m_inputManager->unsubscribe(INPUT_HUD, InputManager::EventType::DOWN, m_onHUDKeyDown);
    delete m_onHUDKeyDown;

    m_inputManager->unsubscribe(INPUT_NIGHT_VISION, InputManager::EventType::DOWN, m_onNightVisionToggle);
    delete m_onNightVisionToggle;

    m_inputManager->unsubscribe(INPUT_NIGHT_VISION_RELOAD, InputManager::EventType::DOWN, m_onNightVisionReload);
    delete m_onNightVisionReload;

    m_inputManager->unsubscribe(INPUT_DRAW_MODE, InputManager::EventType::DOWN, m_onDrawMode);
    delete m_onDrawMode;

    m_threadRunning = false;
    m_updateThread->join();
    delete m_updateThread;
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

    m_soaState->time += 0.0000001;
    auto& npcmp = m_soaState->gameSystem.spacePositionCT.getFromEntity(m_soaState->playerEntity);
    m_soaState->spaceSystem.update(m_soaState->time, npcmp.position, nullptr);
    m_soaState->spaceSystem.glUpdate();
    
    // Update the input
    handleInput();

    m_gameSystemUpdater->update(&m_soaState->gameSystem, &m_soaState->spaceSystem, m_soaState);

    // Update the PDA
    if (m_pda.isOpen()) m_pda.update();

    // Updates the Pause Menu
    if (m_pauseMenu.isOpen()) m_pauseMenu.update();

    // Sort all meshes // TODO(Ben): There is redundancy here
    //_app->meshManager->sortMeshes(m_player->headPosition);

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
  
 //   if (loadPlayerFile(m_player)) {
  //      atSurface = 0; //don't need to set height
 //   }
//    auto cmp = _app->spaceSystem->enableVoxelsOnTarget(m_player->headPosition,
 //                                                       &m_player->voxelMapData,
 //                                                       &m_gameStartState->saveFileIom);
 //   m_chunkManager = cmp->chunkManager;

    // TODO(Ben): Holy shit this blows.
 //   m_player->setNearestPlanet(cmp->sphericalTerrainData->getRadius() * 1000.0f, 9999999999.0f,
//                               (cmp->sphericalTerrainData->getRadius() * 1000.0 * 2.0) / 32.0);
}

void GamePlayScreen::initRenderPipeline() {
    // Set up the rendering pipeline and pass in dependencies
    ui32v4 viewport(0, 0, _app->getWindow().getViewportDims());
    m_renderPipeline.init(viewport, m_soaState,
                          _app, &m_pda,
                          &m_soaState->spaceSystem,
                          &m_pauseMenu, m_chunkManager->getChunkSlots(0));
}

void GamePlayScreen::handleInput() {

    // Block placement
    if (isInGame()) {
        //if (m_inputManager->getKeyDown(INPUT_MOUSE_LEFT) || (GameManager::voxelEditor->isEditing() && m_inputManager->getKey(INPUT_BLOCK_DRAG))) {
        //    if (!(m_player->leftEquippedItem)){
        //        GameManager::clickDragRay(m_chunkManager, m_player, true);
        //    } else if (m_player->leftEquippedItem->type == ITEM_BLOCK){
        //        m_player->dragBlock = m_player->leftEquippedItem;
        //        GameManager::clickDragRay(m_chunkManager, m_player, false);
        //    }
        //} else if (m_inputManager->getKeyDown(INPUT_MOUSE_RIGHT) || (GameManager::voxelEditor->isEditing() && m_inputManager->getKey(INPUT_BLOCK_DRAG))) {
        //    if (!(m_player->rightEquippedItem)){
        //        GameManager::clickDragRay(m_chunkManager, m_player, true);
        //    } else if (m_player->rightEquippedItem->type == ITEM_BLOCK){
        //        m_player->dragBlock = m_player->rightEquippedItem;
        //        GameManager::clickDragRay(m_chunkManager, m_player, false);
        //    }
        //}
    }

    // Update inputManager internal state
    m_inputManager->update();
}

//void GamePlayScreen::updatePlayer() {

   // m_player->update(m_inputManager, true, 0.0f, 0.0f);

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
//}

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




    /*    HeightData tmpHeightData;
        if (!m_chunkManager->getPositionHeightData((int)m_player->headPosition.x, (int)m_player->headPosition.z, tmpHeightData)) {
            m_player->currBiome = tmpHeightData.biome;
            m_player->currTemp = tmpHeightData.temperature;
            m_player->currHumidity = tmpHeightData.rainfall;
        } else {
            m_player->currBiome = NULL;
            m_player->currTemp = -1;
            m_player->currHumidity = -1;
        }*/

        
      //  m_voxelWorld->update(&m_player->getChunkCamera());

        /*  if (m_inputManager->getKey(INPUT_BLOCK_SCANNER)) {
              m_player->scannedBlock = m_chunkManager->getBlockFromDir(glm::dvec3(m_player->chunkDirection()), m_player->headPosition);
              } else {
              m_player->scannedBlock = NONE;
              }*/

        if (SDL_GetTicks() - saveStateTicks >= 20000) {
            saveStateTicks = SDL_GetTicks();
      //      savePlayerState();
        }

        physicsFps = fpsLimiter.endFrame();
    }
}

void GamePlayScreen::processMessages() {

    TerrainMeshMessage* tmm;
    int j = 0,k = 0;
    MeshManager* meshManager = m_soaState->meshManager.get();
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
    //m_player->getWorldCamera().setClippingPlane(clip, MAX(300000000.0 / planetScale, closestTerrainPatchDistance + 10000000));
    //m_player->getWorldCamera().updateProjection();
}

bool GamePlayScreen::loadPlayerFile(Player* player) {
    //loadMarkers(player);
    
    std::vector<ui8> data;
//    if (!m_gameStartState->saveFileIom.readFileToData(("Players/" + player->getName() + ".dat").c_str(), data)) {
        //file doesnt exist so set spawn to random
  //      srand(time(NULL));
//        int spawnFace = rand() % 4 + 1;
//        player->voxelMapData.face = spawnFace;
 //       return 0;
 //   }

    int byte = 0;
 //   player->facePosition.x = BufferUtils::extractFloat(data.data(), (byte++) * 4);
 //   player->facePosition.y = BufferUtils::extractFloat(data.data(), (byte++) * 4);
 //   player->facePosition.z = BufferUtils::extractFloat(data.data(), (byte++) * 4);
 //   player->voxelMapData.face = BufferUtils::extractInt(data.data(), (byte++) * 4);
 //   player->getChunkCamera().setYawAngle(BufferUtils::extractFloat(data.data(), (byte++) * 4));
  //  player->getChunkCamera().setPitchAngle(BufferUtils::extractFloat(data.data(), (byte++) * 4));
  //  player->isFlying = BufferUtils::extractBool(data.data(), byte * 4);

 //   player->voxelMapData.ipos = fastFloor(player->facePosition.z / (double)CHUNK_WIDTH);
  //  player->voxelMapData.jpos = fastFloor(player->facePosition.x / (double)CHUNK_WIDTH);
    return 1;
}

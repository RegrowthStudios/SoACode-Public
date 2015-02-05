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
#include "ChunkMesher.h"
#include "ChunkRenderer.h"
#include "Collision.h"
#include "DebugRenderer.h"
#include "Errors.h"
#include "GameManager.h"
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
#include "SpaceSystemUpdater.h"
#include "SphericalTerrainPatch.h"
#include "TexturePackLoader.h"
#include "VoxelEditor.h"
#include "SoaEngine.h"

#define THREAD ThreadId::UPDATE

GamePlayScreen::GamePlayScreen(const App* app, const MainMenuScreen* mainMenuScreen) :
    IAppScreen<App>(app),
    m_mainMenuScreen(mainMenuScreen),
    m_updateThread(nullptr),
    m_threadRunning(false), 
    m_inFocus(true),
    controller(app) {
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

    initInput();

    ChunkMesher::bindVBOIndicesID();

    m_soaState = m_mainMenuScreen->getSoAState();

    controller.startGame(m_soaState);

    m_spaceSystemUpdater = std::make_unique<SpaceSystemUpdater>();
    m_gameSystemUpdater = std::make_unique<GameSystemUpdater>(m_soaState->gameSystem.get(), m_inputManager);

    // Initialize the PDA
    m_pda.init(this, m_soaState->glProgramManager.get());

    // Initialize the Pause Menu
    m_pauseMenu.init(this, m_soaState->glProgramManager.get());

    // Set up the rendering
    initRenderPipeline();

    // Initialize and run the update thread
    m_updateThread = new std::thread(&GamePlayScreen::updateThreadFunc, this);

    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void GamePlayScreen::onExit(const GameTime& gameTime) {

    m_inputManager->stopInput();
    m_hooks.dispose();

    SoaEngine::destroyGameSystem(m_soaState);

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

    globalRenderAccumulationTimer.start("Space System");

    // TEMPORARY TIMESTEP TODO(Ben): Get rid of this damn global
    if (_app->getFps()) {
        glSpeedFactor = 60.0f / _app->getFps();
        if (glSpeedFactor > 3.0f) { // Cap variable timestep at 20fps
            glSpeedFactor = 3.0f;
        }
    }
    m_spaceSystemUpdater->glUpdate(m_soaState->spaceSystem.get());

    globalRenderAccumulationTimer.start("Update Meshes");

    m_soaState->meshManager->updateMeshes(f64v3(0.0), false);

    globalRenderAccumulationTimer.start("Process Messages");
    // Update the input
    handleInput();

    // Update the PDA
    if (m_pda.isOpen()) m_pda.update();

    // Updates the Pause Menu
    if (m_pauseMenu.isOpen()) m_pauseMenu.update();

    // Sort all meshes // TODO(Ben): There is redundancy here
    //_app->meshManager->sortMeshes(m_player->headPosition);

    // Process any updates from the render thread
    processMessages();

    globalRenderAccumulationTimer.stop();
}

void GamePlayScreen::draw(const GameTime& gameTime) {
    globalRenderAccumulationTimer.start("Draw");
    updateWorldCameraClip();
    m_renderPipeline.render();
    globalRenderAccumulationTimer.stop();

    // Uncomment to time rendering
    /*  static int g = 0;
      if (++g == 10) {
      globalRenderAccumulationTimer.printAll(true);
      globalRenderAccumulationTimer.clear();
      std::cout << "\n";
      g = 0;
      }*/
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

void GamePlayScreen::initInput() {

    m_inputManager = new InputManager;
    initInputs(m_inputManager);

    m_inputManager->subscribeFunctor(INPUT_PAUSE, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        m_inFocus = false;
    });
    m_inputManager->subscribeFunctor(INPUT_GRID, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        m_renderPipeline.toggleChunkGrid();
    });
    m_inputManager->subscribeFunctor(INPUT_INVENTORY, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        if (m_pda.isOpen()) {
            m_pda.close();
            SDL_SetRelativeMouseMode(SDL_TRUE);
            m_inFocus = true;
            SDL_StartTextInput();
        } else {
            m_pda.open();
            SDL_SetRelativeMouseMode(SDL_FALSE);
            m_inFocus = false;
            SDL_StopTextInput();
        }
    });
    m_inputManager->subscribeFunctor(INPUT_NIGHT_VISION, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        if (isInGame()) {
            m_renderPipeline.toggleNightVision();
        }
    });
    m_inputManager->subscribeFunctor(INPUT_HUD, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        m_renderPipeline.cycleDevHud();
    });
    m_inputManager->subscribeFunctor(INPUT_NIGHT_VISION_RELOAD, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        m_renderPipeline.loadNightVision();
    });
    m_inputManager->subscribeFunctor(INPUT_DRAW_MODE, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        m_renderPipeline.cycleDrawMode();
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onButtonDown, [&](Sender s, const vui::MouseButtonEvent& e) {
        if (isInGame()) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            m_inFocus = true;
        }
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onButtonUp, [&](Sender s, const vui::MouseButtonEvent& e) {
        if (GameManager::voxelEditor->isEditing()) {
            //TODO(Ben): Edit voxels
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

void GamePlayScreen::initRenderPipeline() {
    // Set up the rendering pipeline and pass in dependencies
    ui32v4 viewport(0, 0, _app->getWindow().getViewportDims());
    m_renderPipeline.init(viewport, m_soaState,
                          _app, &m_pda,
                          m_soaState->spaceSystem.get(),
                          &m_pauseMenu);
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

        m_soaState->time += 0.00000000001;
        auto& npcmp = m_soaState->gameSystem->spacePosition.getFromEntity(m_soaState->playerEntity);

        m_spaceSystemUpdater->update(m_soaState->spaceSystem.get(), m_soaState->gameSystem.get(), m_soaState,
                                       m_soaState->gameSystem->spacePosition.getFromEntity(m_soaState->playerEntity).position);

        m_gameSystemUpdater->update(m_soaState->gameSystem.get(), m_soaState->spaceSystem.get(), m_soaState);


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

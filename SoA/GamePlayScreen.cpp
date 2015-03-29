#include "stdafx.h"
#include "GameplayScreen.h"

#include <Vorb/colors.h>
#include <Vorb/Events.hpp>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/utils.h>

#include "soaUtils.h"
#include "App.h"
#include "ChunkMesh.h"
#include "ChunkMeshManager.h"
#include "ChunkMesher.h"
#include "ChunkRenderer.h"
#include "Collision.h"
#include "DebugRenderer.h"
#include "Errors.h"
#include "GameManager.h"
#include "GameSystem.h"
#include "GameSystemUpdater.h"
#include "InputMapper.h"
#include "Inputs.h"
#include "LoadTaskShaders.h"
#include "MainMenuScreen.h"
#include "MeshManager.h"
#include "Options.h"
#include "ParticleMesh.h"
#include "PhysicsBlocks.h"
#include "RenderTask.h"
#include "SoaEngine.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SpaceSystemUpdater.h"
#include "TerrainPatch.h"
#include "TexturePackLoader.h"
#include "VoxelEditor.h"

GameplayScreen::GameplayScreen(const App* app, const MainMenuScreen* mainMenuScreen) :
    IAppScreen<App>(app),
    m_mainMenuScreen(mainMenuScreen),
    m_updateThread(nullptr),
    m_threadRunning(false),
    controller(app) {
    // Empty
}

GameplayScreen::~GameplayScreen() {
    // Empty
}

i32 GameplayScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 GameplayScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void GameplayScreen::build() {
    // Empty
}

void GameplayScreen::destroy(const vui::GameTime& gameTime) {
    // Destruction happens in onExit
}

void GameplayScreen::onEntry(const vui::GameTime& gameTime) {

    initInput();

    ChunkMesher::bindVBOIndicesID();

    m_soaState = m_mainMenuScreen->getSoAState();

    controller.startGame(m_soaState);

    m_spaceSystemUpdater = std::make_unique<SpaceSystemUpdater>();
    m_gameSystemUpdater = std::make_unique<GameSystemUpdater>(m_soaState, m_inputManager);

    // Initialize the PDA
    m_pda.init(this, m_soaState->glProgramManager.get());

    // Initialize the Pause Menu
    m_pauseMenu.init(this, m_soaState->glProgramManager.get());

    // Set up the rendering
    initRenderPipeline();

    // Initialize and run the update thread
    m_updateThread = new std::thread(&GameplayScreen::updateThreadFunc, this);

    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void GameplayScreen::onExit(const vui::GameTime& gameTime) {

    m_inputManager->stopInput();
    m_hooks.dispose();

    SoaEngine::destroyGameSystem(m_soaState);

    m_threadRunning = false;
    m_updateThread->join();
    delete m_updateThread;
    m_pda.destroy();
    m_renderPipeline.destroy(true);
    m_pauseMenu.destroy();
}


/// This update function runs on the render thread
void GameplayScreen::update(const vui::GameTime& gameTime) {

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

    // TODO(Ben): Move to glUpdate for voxel component
    // TODO(Ben): Don't hardcode for a single player
    auto& vpCmp = m_soaState->gameSystem->voxelPosition.getFromEntity(m_soaState->playerEntity);
    m_soaState->chunkMeshManager->update(vpCmp.gridPosition.pos, false);

    globalRenderAccumulationTimer.start("Process Messages");

    // Update the PDA
    if (m_pda.isOpen()) m_pda.update();

    // Updates the Pause Menu
    if (m_pauseMenu.isOpen()) m_pauseMenu.update();

    globalRenderAccumulationTimer.stop();
}

void GameplayScreen::updateECS() {
    SpaceSystem* spaceSystem = m_soaState->spaceSystem.get();
    GameSystem* gameSystem = m_soaState->gameSystem.get();

    m_soaState->time += m_soaState->timeStep;
    // TODO(Ben): Don't hardcode for a single player
    auto& spCmp = gameSystem->spacePosition.getFromEntity(m_soaState->playerEntity);
    auto parentNpCmpId = spaceSystem->m_sphericalGravityCT.get(spCmp.parentGravityID).namePositionComponent;
    auto& parentNpCmp = spaceSystem->m_namePositionCT.get(parentNpCmpId);
    // Calculate non-relative space position
    f64v3 trueSpacePosition = spCmp.position + parentNpCmp.position;

    m_spaceSystemUpdater->update(spaceSystem, gameSystem, m_soaState,
                                 trueSpacePosition,
                                 m_soaState->gameSystem->voxelPosition.getFromEntity(m_soaState->playerEntity).gridPosition.pos);

    m_gameSystemUpdater->update(gameSystem, spaceSystem, m_soaState);
}

void GameplayScreen::updateMTRenderState() {
    MTRenderState* state = m_renderStateManager.getRenderStateForUpdate();

    SpaceSystem* spaceSystem = m_soaState->spaceSystem.get();
    // Set all space positions
    for (auto& it : spaceSystem->m_namePositionCT) {
        state->spaceBodyPositions[it.first] = it.second.position;
    }
    // Set camera position
    auto& spCmp = m_soaState->gameSystem->spacePosition.getFromEntity(m_soaState->playerEntity);
    state->spaceCameraPos = spCmp.position;
    state->spaceCameraOrientation = spCmp.orientation;

    m_renderStateManager.finishUpdating();
}

void GameplayScreen::draw(const vui::GameTime& gameTime) {
    globalRenderAccumulationTimer.start("Draw");
    updateWorldCameraClip();

    const MTRenderState* renderState;
    // Don't render the same state twice.
    while ((renderState = m_renderStateManager.getRenderStateForRender()) == m_prevRenderState) {
        Sleep(0);
    }
    m_prevRenderState = renderState;

    // Set renderState and draw everything
    m_renderPipeline.setRenderState(renderState);
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

void GameplayScreen::unPause() { 
    m_pauseMenu.close(); 
    SDL_SetRelativeMouseMode(SDL_TRUE);
    m_soaState->isInputEnabled = true;
}

i32 GameplayScreen::getWindowWidth() const {
    return _app->getWindow().getWidth();
}

i32 GameplayScreen::getWindowHeight() const {
    return _app->getWindow().getHeight();
}

void GameplayScreen::initInput() {

    m_inputManager = new InputMapper;
    initInputs(m_inputManager);

    m_inputManager->get(INPUT_PAUSE).downEvent.addFunctor([&](Sender s, ui32 a) -> void {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        m_soaState->isInputEnabled = false;
    });
    m_inputManager->get(INPUT_GRID).downEvent.addFunctor([&](Sender s, ui32 a) -> void {
        m_renderPipeline.toggleChunkGrid();
    });
    m_inputManager->get(INPUT_INVENTORY).downEvent.addFunctor([&](Sender s, ui32 a) -> void {
        if (m_pda.isOpen()) {
            m_pda.close();
            SDL_SetRelativeMouseMode(SDL_TRUE);
            m_soaState->isInputEnabled = true;
            SDL_StartTextInput();
        } else {
            m_pda.open();
            SDL_SetRelativeMouseMode(SDL_FALSE);
            m_soaState->isInputEnabled = false;
            SDL_StopTextInput();
        }
    });
    m_inputManager->get(INPUT_NIGHT_VISION).downEvent.addFunctor([&](Sender s, ui32 a) -> void {
        if (isInGame()) {
            m_renderPipeline.toggleNightVision();
        }
    });
    m_inputManager->get(INPUT_HUD).downEvent.addFunctor([&](Sender s, ui32 a) -> void {
        m_renderPipeline.cycleDevHud();
    });
    m_inputManager->get(INPUT_NIGHT_VISION_RELOAD).downEvent.addFunctor([&](Sender s, ui32 a) -> void {
        m_renderPipeline.loadNightVision();
    });
    m_inputManager->get(INPUT_DRAW_MODE).downEvent.addFunctor([&](Sender s, ui32 a) -> void {
        m_renderPipeline.cycleDrawMode();
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s, const vui::MouseButtonEvent& e) {
        if (isInGame()) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            m_soaState->isInputEnabled = true;
        }
    });

    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&](Sender s, const vui::MouseButtonEvent& e) {
        if (GameManager::voxelEditor->isEditing()) {
            //TODO(Ben): Edit voxels
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onFocusGained, [&](Sender s, const vui::MouseEvent& e) {
        m_soaState->isInputEnabled = true;
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onFocusLost, [&](Sender s, const vui::MouseEvent& e) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        m_soaState->isInputEnabled = false;
    });

    m_inputManager->startInput();
}

void GameplayScreen::initRenderPipeline() {
    // Set up the rendering pipeline and pass in dependencies
    ui32v4 viewport(0, 0, _app->getWindow().getViewportDims());
    m_renderPipeline.init(viewport, m_soaState,
                          _app, &m_pda,
                          m_soaState->spaceSystem.get(),
                          m_soaState->gameSystem.get(),
                          &m_pauseMenu);
}

// TODO(Ben): Collision
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

/// This is the update thread
void GameplayScreen::updateThreadFunc() {
    m_threadRunning = true;

    FpsLimiter fpsLimiter;
    fpsLimiter.init(maxPhysicsFps);

    static int saveStateTicks = SDL_GetTicks();

    while (m_threadRunning) {
        fpsLimiter.beginFrame();

        updateECS();
        updateMTRenderState();

        if (SDL_GetTicks() - saveStateTicks >= 20000) {
            saveStateTicks = SDL_GetTicks();
      //      savePlayerState();
        }

        physicsFps = fpsLimiter.endFrame();
    }
}

void GameplayScreen::updateWorldCameraClip() {
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

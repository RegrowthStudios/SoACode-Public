#include "stdafx.h"
#include "GamePlayScreen.h"

#include <Vorb/os.h>
#include <Vorb/colors.h>
#include <Vorb/Events.hpp>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/utils.h>

#include "App.h"
#include "ChunkMesh.h"
#include "ChunkMeshManager.h"
#include "ChunkMesher.h"
#include "ChunkRenderer.h"
#include "Collision.h"
#include "DebugRenderer.h"
#include "DevConsole.h"
#include "Errors.h"
#include "GameManager.h"
#include "GameSystem.h"
#include "GameSystemUpdater.h"
#include "HeadComponentUpdater.h"
#include "InputMapper.h"
#include "Inputs.h"
#include "MainMenuScreen.h"
#include "ParticleMesh.h"
#include "SoaEngine.h"
#include "SoaOptions.h"
#include "SoAState.h"
#include "SpaceSystem.h"
#include "SpaceSystemUpdater.h"
#include "TerrainPatch.h"
#include "VRayHelper.h"
#include "VoxelEditor.h"
#include "soaUtils.h"

GameplayScreen::GameplayScreen(const App* app, const MainMenuScreen* mainMenuScreen) :
    IAppScreen<App>(app),
    m_mainMenuScreen(mainMenuScreen),
    m_soaState(nullptr),
    m_inputMapper(nullptr),
    controller(),
    m_spaceSystemUpdater(nullptr),
    m_gameSystemUpdater(nullptr),
    m_updateThread(nullptr),
    m_threadRunning(false),
    m_prevRenderState(nullptr),
    m_shouldReloadTarget(false),
    m_shouldReloadShaders(false),
    m_shouldToggleDevConsole(false){
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

void GameplayScreen::destroy(const vui::GameTime& gameTime VORB_UNUSED) {
    // Destruction happens in onExit
}

void GameplayScreen::onEntry(const vui::GameTime& gameTime VORB_UNUSED) {

#ifdef _WINDOWS
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#else
    struct sched_param params;
 
    params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);
#endif

    m_soaState = m_mainMenuScreen->getSoAState();

    controller.startGame(m_soaState);

    initInput();

    initConsole();

    m_spaceSystemUpdater = std::make_unique<SpaceSystemUpdater>();
    m_gameSystemUpdater = std::make_unique<GameSystemUpdater>(m_soaState, m_inputMapper);

    // Initialize the PDA
    m_pda.init(this);

    // Initialize the Pause Menu
    m_pauseMenu.init(this);

    // Set up the rendering
    initRenderPipeline();

    // Initialize and run the update thread
    m_updateThread = new std::thread(&GameplayScreen::updateThreadFunc, this);

    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void GameplayScreen::onExit(const vui::GameTime& gameTime VORB_UNUSED) {

    m_inputMapper->stopInput();
    m_hooks.dispose();

    SoaEngine::destroyGameSystem(m_soaState);

    m_threadRunning = false;
    m_updateThread->join();
    delete m_updateThread;
    m_pda.destroy();
    //m_renderPipeline.destroy(true);
    m_pauseMenu.destroy();

    m_devConsoleView.dispose();
}

/// This update function runs on the render thread
void GameplayScreen::update(const vui::GameTime& gameTime VORB_UNUSED) {

    if (m_shouldReloadShaders) {
        m_renderer.reloadShaders();
        m_shouldReloadShaders = false;
    }

    if (m_shouldToggleDevConsole) {
        m_shouldToggleDevConsole = false;
        DevConsole::getInstance().toggleFocus();
    }

    m_spaceSystemUpdater->glUpdate(m_soaState);

    // TODO(Ben): Move to glUpdate for voxel component
    // TODO(Ben): Don't hardcode for a single player
    auto& vpCmp = m_soaState->gameSystem->voxelPosition.getFromEntity(m_soaState->clientState.playerEntity);
    m_soaState->clientState.chunkMeshManager->update(vpCmp.gridPosition.pos, true);

    // Update the PDA
    if (m_pda.isOpen()) m_pda.update();

    // Updates the Pause Menu
    if (m_pauseMenu.isOpen()) m_pauseMenu.update();

    // Check for target reload
    if (m_shouldReloadTarget) {
        m_reloadLock.lock();
        printf("Reloading Target\n");
        m_soaState->threadPool->clearTasks();
        std::this_thread::sleep_for(std::chrono::microseconds(200));
        SoaEngine::reloadSpaceBody(m_soaState, m_soaState->clientState.startingPlanet, nullptr);
        m_shouldReloadTarget = false;
        m_reloadLock.unlock();
    }
}

void GameplayScreen::updateECS() {
    SpaceSystem* spaceSystem = m_soaState->spaceSystem;
    GameSystem* gameSystem = m_soaState->gameSystem;

    // Time warp
    const f64 TIME_WARP_SPEED = 100.0 + (f64)m_inputMapper->getInputState(INPUT_SPEED_TIME) * 1000.0;
    if (m_inputMapper->getInputState(INPUT_TIME_BACK)) {
        m_soaState->time -= TIME_WARP_SPEED;
    }
    if (m_inputMapper->getInputState(INPUT_TIME_FORWARD)) {
        m_soaState->time += TIME_WARP_SPEED;
    }

    m_soaState->time += m_soaState->timeStep;
    // TODO(Ben): Don't hardcode for a single player
    auto& spCmp = gameSystem->spacePosition.getFromEntity(m_soaState->clientState.playerEntity);
    auto parentNpCmpId = spaceSystem->sphericalGravity.get(spCmp.parentGravity).namePositionComponent;
    auto& parentNpCmp = spaceSystem->namePosition.get(parentNpCmpId);
    // Calculate non-relative space position
    f64v3 trueSpacePosition = spCmp.position + parentNpCmp.position;

    m_spaceSystemUpdater->update(m_soaState,
                                 trueSpacePosition,
                                 m_soaState->gameSystem->voxelPosition.getFromEntity(m_soaState->clientState.playerEntity).gridPosition.pos);

    m_gameSystemUpdater->update(gameSystem, spaceSystem, m_soaState);
}

void GameplayScreen::updateMTRenderState() {
    MTRenderState* state = m_renderStateManager.getRenderStateForUpdate();

    SpaceSystem* spaceSystem = m_soaState->spaceSystem;
    GameSystem* gameSystem = m_soaState->gameSystem;
    // Set all space positions
    for (auto& it : spaceSystem->namePosition) {
        state->spaceBodyPositions[it.first] = it.second.position;
    }
    // Set camera position
    auto& spCmp = gameSystem->spacePosition.getFromEntity(m_soaState->clientState.playerEntity);
    state->spaceCameraPos = spCmp.position;
    state->spaceCameraOrientation = spCmp.orientation;

    // Set player data
    auto& physics = gameSystem->physics.getFromEntity(m_soaState->clientState.playerEntity);
    if (physics.voxelPosition) {
        state->playerHead = gameSystem->head.getFromEntity(m_soaState->clientState.playerEntity);
        state->playerPosition = gameSystem->voxelPosition.get(physics.voxelPosition);
        state->hasVoxelPos = true;
    } else {
        state->hasVoxelPos = false;
    }
    // Debug chunk grid
    if (m_renderer.stages.chunkGrid.isActive() && m_soaState->clientState.startingPlanet) {
        // TODO(Ben): This doesn't let you go to different planets!!!
        auto& svcmp = spaceSystem->sphericalVoxel.getFromEntity(m_soaState->clientState.startingPlanet);
        auto& vpCmp = gameSystem->voxelPosition.getFromEntity(m_soaState->clientState.playerEntity);
        state->debugChunkData.clear();
        if (svcmp.chunkGrids) {
            for (ChunkHandle chunk : svcmp.chunkGrids[vpCmp.gridPosition.face].acquireActiveChunks()) {
                state->debugChunkData.emplace_back();
                state->debugChunkData.back().genLevel = chunk->genLevel;
                state->debugChunkData.back().voxelPosition = chunk->getVoxelPosition().pos;
            }
            svcmp.chunkGrids[vpCmp.gridPosition.face].releaseActiveChunks();
        }
    } else {
        std::vector<DebugChunkData>().swap(state->debugChunkData);
    }

    m_renderStateManager.finishUpdating();
}

void GameplayScreen::draw(const vui::GameTime& gameTime VORB_UNUSED) {
    globalRenderAccumulationTimer.start("Draw");

    const MTRenderState* renderState;
    // Don't render the same state twice.
    if ((renderState = m_renderStateManager.getRenderStateForRender()) == m_prevRenderState) {
        return;
    }
    m_prevRenderState = renderState;

    // Set renderState and draw everything
    m_renderer.setRenderState(renderState);
    m_renderer.render();
    globalRenderAccumulationTimer.stop();

    // Draw dev console
    m_devConsoleView.update(0.01f);
    if (DevConsole::getInstance().isFocused()) {
        m_devConsoleView.render(m_game->getWindow().getViewportDims());
    }

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
    return m_app->getWindow().getWidth();
}

i32 GameplayScreen::getWindowHeight() const {
    return m_app->getWindow().getHeight();
}

void GameplayScreen::initInput() {

    m_inputMapper = new InputMapper;
    initInputs(m_inputMapper);

    m_inputMapper->get(INPUT_PAUSE).downEvent.addFunctor([&](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) -> void {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        m_soaState->isInputEnabled = false;
    });
    m_inputMapper->get(INPUT_GRID).downEvent.addFunctor([&](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) -> void {
        m_renderer.toggleChunkGrid();
    });
    m_inputMapper->get(INPUT_INVENTORY).downEvent.addFunctor([&](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) -> void {
       /* if (m_pda.isOpen()) {
            m_pda.close();
            SDL_SetRelativeMouseMode(SDL_TRUE);
            m_soaState->isInputEnabled = true;
            SDL_StartTextInput();
        } else {
            m_pda.open();
            SDL_SetRelativeMouseMode(SDL_FALSE);
            m_soaState->isInputEnabled = false;
            SDL_StopTextInput();
        }*/
        SDL_SetRelativeMouseMode(SDL_FALSE);
        m_inputMapper->stopInput();
        m_soaState->isInputEnabled = false;
    });
    m_inputMapper->get(INPUT_NIGHT_VISION).downEvent.addFunctor([&](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) -> void {
        if (isInGame()) {
            m_renderer.toggleNightVision();
        }
    });
    m_inputMapper->get(INPUT_HUD).downEvent.addFunctor([&](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) -> void {
        m_renderer.cycleDevHud();
    });
    m_inputMapper->get(INPUT_NIGHT_VISION_RELOAD).downEvent.addFunctor([&](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) -> void {
        m_renderer.loadNightVision();
    });

    m_inputMapper->get(INPUT_RELOAD_SHADERS).downEvent += makeDelegate(*this, &GameplayScreen::onReloadShaders);
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s VORB_UNUSED, const vui::MouseButtonEvent& e VORB_UNUSED) {
        if (isInGame()) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            m_soaState->isInputEnabled = true;
        }
    });

    m_inputMapper->get(INPUT_RELOAD_TARGET).downEvent += makeDelegate(*this, &GameplayScreen::onReloadTarget);

    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&](Sender s VORB_UNUSED, const vui::MouseButtonEvent& e VORB_UNUSED) {
        if (GameManager::voxelEditor->isEditing()) {
            //TODO(Ben): Edit voxels
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s VORB_UNUSED, const vui::MouseButtonEvent& e VORB_UNUSED) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        m_inputMapper->startInput();
        m_soaState->isInputEnabled = true;     
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onFocusLost, [&](Sender s VORB_UNUSED, const vui::MouseEvent& e VORB_UNUSED) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        m_inputMapper->stopInput();
        m_soaState->isInputEnabled = false;
    });
    
    // Temporary dev console
    // TODO(Ben): Don't use functor
    vui::InputDispatcher::key.onKeyDown.addFunctor([&](Sender, const vui::KeyEvent& e VORB_UNUSED) {
        if (e.keyCode == VKEY_GRAVE) {
            m_shouldToggleDevConsole = true;
            if (!DevConsole::getInstance().isFocused()) {
                m_inputMapper->stopInput();
                m_soaState->isInputEnabled = false;
            } else {
                m_inputMapper->startInput();
                m_soaState->isInputEnabled = true;
            }
        }
    }
    );
 
    { // Player movement events
        vecs::ComponentID parkourCmp = m_soaState->gameSystem->parkourInput.getComponentID(m_soaState->clientState.playerEntity);
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_FORWARD).downEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).moveForward = true;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_FORWARD).upEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).moveForward = false;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_BACKWARD).downEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).moveBackward = true;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_BACKWARD).upEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).moveBackward = false;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_LEFT).downEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).moveLeft = true;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_LEFT).upEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).moveLeft = false;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_RIGHT).downEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).moveRight = true;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_RIGHT).upEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).moveRight = false;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_JUMP).downEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).jump = true;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_JUMP).upEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).jump = false;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_CROUCH).downEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).crouch = true;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_CROUCH).upEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).crouch = false;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_SPRINT).downEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).sprint = true;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_SPRINT).upEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).sprint = false;
        });
        // TODO(Ben): Different parkour input
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_SPRINT).downEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).parkour = true;
        });
        m_hooks.addAutoHook(m_inputMapper->get(INPUT_SPRINT).upEvent, [=](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
            m_soaState->gameSystem->parkourInput.get(parkourCmp).parkour = false;
        });
        m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s VORB_UNUSED, const vui::MouseButtonEvent& e VORB_UNUSED) {
            if (m_soaState->clientState.playerEntity) {
                vecs::EntityID pid = m_soaState->clientState.playerEntity;
                f64v3 pos = controller.getEntityEyeVoxelPosition(m_soaState, pid);
                f32v3 dir = controller.getEntityViewVoxelDirection(m_soaState, pid);
                auto& vp = m_soaState->gameSystem->voxelPosition.getFromEntity(pid);
                vecs::ComponentID svid = vp.parentVoxel;
                ChunkGrid& grid = m_soaState->spaceSystem->sphericalVoxel.get(svid).chunkGrids[vp.gridPosition.face];
                VoxelRayFullQuery q = VRayHelper().getFullQuery(pos, dir, 100.0, grid);
                m_soaState->clientState.voxelEditor.setStartPosition(q.outer.location);

                std::cout << "DIST " << glm::length(f64v3(q.outer.location) - pos);
                printVec("Start ", q.outer.location);
                m_renderer.debugRenderer->drawLine(pos, pos + f64v3(dir) * 100.0, color::Red);
            }
        });
        m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&](Sender s VORB_UNUSED, const vui::MouseButtonEvent& e VORB_UNUSED) {
            if (m_soaState->clientState.playerEntity) {
                vecs::EntityID pid = m_soaState->clientState.playerEntity;
                f64v3 pos = controller.getEntityEyeVoxelPosition(m_soaState, pid);
                f32v3 dir = controller.getEntityViewVoxelDirection(m_soaState, pid);
                auto& vp = m_soaState->gameSystem->voxelPosition.getFromEntity(pid);
                vecs::ComponentID svid = vp.parentVoxel;
                ChunkGrid& grid = m_soaState->spaceSystem->sphericalVoxel.get(svid).chunkGrids[vp.gridPosition.face];
                VoxelRayFullQuery q = VRayHelper().getFullQuery(pos, dir, 100.0, grid);
                m_soaState->clientState.voxelEditor.setEndPosition(q.outer.location);
                printVec("End ", q.outer.location);
                ItemStack& iStack = m_soaState->gameSystem->inventory.getFromEntity(pid).items[0];

                m_soaState->clientState.voxelEditor.editVoxels(grid, &iStack);
            }
        });
        // Mouse movement
        vecs::ComponentID headCmp = m_soaState->gameSystem->head.getComponentID(m_soaState->clientState.playerEntity);
        m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [=](Sender s VORB_UNUSED, const vui::MouseMotionEvent& e VORB_UNUSED) {
            HeadComponentUpdater::rotateFromMouse(m_soaState->gameSystem, headCmp, -e.dx, e.dy, 0.002f);
        });
    }

    vui::InputDispatcher::window.onClose += makeDelegate(*this, &GameplayScreen::onWindowClose);

    m_inputMapper->get(INPUT_SCREENSHOT).downEvent.addFunctor([&](Sender s VORB_UNUSED, ui32 i VORB_UNUSED) {
        m_renderer.takeScreenshot(); });
    m_inputMapper->get(INPUT_DRAW_MODE).downEvent += makeDelegate(*this, &GameplayScreen::onToggleWireframe);

    m_inputMapper->startInput();
}

void GameplayScreen::initConsole() {
    vui::GameWindow& window = m_game->getWindow();
    // TODO(Ben): Dispose
    m_devConsoleView.init(&DevConsole::getInstance(), 5,
                          f32v2(20.0f, window.getHeight() - 60.0f),
                          window.getWidth() - 40.0f);
    DevConsole::getInstance().addCommand("exit");
    DevConsole::getInstance().addListener("exit", [](void*, const nString&) {
        exit(0);
    }, nullptr);
}

void GameplayScreen::initRenderPipeline() {
    m_renderer.debugRenderer = m_soaState->clientState.debugRenderer;
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

#ifdef _WINDOWS    
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#else//_WINDOWS
    struct sched_param params;
 
    params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);
#endif//_WINDOWS

    FpsLimiter fpsLimiter;
    fpsLimiter.init(60.0f);
    // TODO(Matthew): This statement wasn't used, eventually log/display FPS.
    //f32 fps;

    static int saveStateTicks = SDL_GetTicks();

    while (m_threadRunning) {
        fpsLimiter.beginFrame();

        m_reloadLock.lock();
        updateECS(); // TODO(Ben): Entity destruction in this thread calls opengl stuff.....
        updateMTRenderState();
        m_reloadLock.unlock();

        if (SDL_GetTicks() - saveStateTicks >= 20000) {
            saveStateTicks = SDL_GetTicks();
      //      savePlayerState();
        }

        //fps = fpsLimiter.endFrame();
    }
}

void GameplayScreen::onReloadShaders(Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
    printf("Reloading Shaders\n");
    m_shouldReloadShaders = true;
}
void GameplayScreen::onReloadTarget(Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
    m_shouldReloadTarget = true;
}

void GameplayScreen::onQuit(Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
    SoaEngine::destroyAll(m_soaState);
    exit(0);
}

void GameplayScreen::onToggleWireframe(Sender s VORB_UNUSED, ui32 i VORB_UNUSED) {
    m_renderer.toggleWireframe();
}

void GameplayScreen::onWindowClose(Sender s) {
    onQuit(s, 0);
}
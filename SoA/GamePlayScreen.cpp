#include "stdafx.h"
#include "GamePlayScreen.h"

#include "Player.h"
#include "App.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Sound.h"
#include "MessageManager.h"
#include "Planet.h"
#include "TerrainPatch.h"
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
#include "GpuMemory.h"
#include "SpriteFont.h"
#include "SpriteBatch.h"
#include "colors.h"
#include "Options.h"
#include "GamePlayScreenEvents.hpp"
#include "Events.hpp"

#define THREAD ThreadId::UPDATE

CTOR_APP_SCREEN_DEF(GamePlayScreen, App),
    _updateThread(nullptr),
    _threadRunning(false), 
    _inFocus(true),
    _onPauseKeyDown(nullptr),
    _onFlyKeyDown(nullptr),
    _onGridKeyDown(nullptr),
    _onReloadTexturesKeyDown(nullptr),
    _onReloadShadersKeyDown(nullptr),
    _onInventoryKeyDown(nullptr),
    _onReloadUIKeyDown(nullptr),
    _onHUDKeyDown(nullptr) {
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
    
}

void GamePlayScreen::destroy(const GameTime& gameTime) {
    
}

void GamePlayScreen::onEntry(const GameTime& gameTime) {

    _player = GameManager::player;
    _player->initialize("Ben", _app->getWindow().getAspectRatio()); //What an awesome name that is
    GameManager::initializeVoxelWorld(_player);

    // Initialize the PDA
    _pda.init(this);

    // Set up the rendering
    initRenderPipeline();

    // Initialize and run the update thread
    _updateThread = new std::thread(&GamePlayScreen::updateThreadFunc, this);

    // Force him to fly... This is temporary
    _player->isFlying = true;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    InputManager* inputManager = GameManager::inputManager;
    _onPauseKeyDown = inputManager->subscribe(INPUT_PAUSE, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnPauseKeyDown(this));
    _onFlyKeyDown = inputManager->subscribe(INPUT_FLY, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnFlyKeyDown(this));
    _onGridKeyDown = inputManager->subscribe(INPUT_GRID, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnGridKeyDown(this));
    _onReloadTexturesKeyDown = inputManager->subscribe(INPUT_RELOAD_TEXTURES, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnReloadTexturesKeyDown(this));
    _onReloadShadersKeyDown = inputManager->subscribe(INPUT_RELOAD_SHADERS, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnReloadShadersKeyDown(this));
    _onInventoryKeyDown = inputManager->subscribe(INPUT_INVENTORY, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnInventoryKeyDown(this));
    _onReloadUIKeyDown = inputManager->subscribe(INPUT_RELOAD_UI, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnReloadUIKeyDown(this));
    _onHUDKeyDown = inputManager->subscribe(INPUT_HUD, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnHUDKeyDown(this));
    _onNightVisionToggle = inputManager->subscribeFunctor(INPUT_NIGHT_VISION, InputManager::EventType::DOWN, [&] (void* s, ui32 a) -> void {
        _renderPipeline.toggleNightVision();
    });
    _onNightVisionReload = inputManager->subscribeFunctor(INPUT_NIGHT_VISION_RELOAD, InputManager::EventType::DOWN, [&] (void* s, ui32 a) -> void {
        _renderPipeline.loadNightVision();
    });
}

void GamePlayScreen::onExit(const GameTime& gameTime) {
    InputManager* inputManager = GameManager::inputManager;
    inputManager->unsubscribe(INPUT_PAUSE, InputManager::EventType::DOWN, _onPauseKeyDown);
    delete _onPauseKeyDown;

    inputManager->unsubscribe(INPUT_FLY, InputManager::EventType::DOWN, _onFlyKeyDown);
    delete _onFlyKeyDown;

    inputManager->unsubscribe(INPUT_GRID, InputManager::EventType::DOWN, _onGridKeyDown);
    delete _onGridKeyDown;

    inputManager->unsubscribe(INPUT_RELOAD_TEXTURES, InputManager::EventType::DOWN, _onReloadTexturesKeyDown);
    delete _onReloadTexturesKeyDown;

    inputManager->unsubscribe(INPUT_RELOAD_SHADERS, InputManager::EventType::DOWN, _onReloadShadersKeyDown);
    delete _onReloadShadersKeyDown;

    inputManager->unsubscribe(INPUT_INVENTORY, InputManager::EventType::DOWN, _onInventoryKeyDown);
    delete _onInventoryKeyDown;

    inputManager->unsubscribe(INPUT_RELOAD_UI, InputManager::EventType::DOWN, _onReloadUIKeyDown);
    delete _onReloadUIKeyDown;

    inputManager->unsubscribe(INPUT_HUD, InputManager::EventType::DOWN, _onHUDKeyDown);
    delete _onHUDKeyDown;

    inputManager->unsubscribe(INPUT_NIGHT_VISION, InputManager::EventType::DOWN, _onNightVisionToggle);
    delete _onNightVisionToggle;

    inputManager->unsubscribe(INPUT_NIGHT_VISION_RELOAD, InputManager::EventType::DOWN, _onNightVisionReload);
    delete _onNightVisionReload;

    _threadRunning = false;
    _updateThread->join();
    delete _updateThread;
    _app->meshManager->destroy();
    _pda.destroy();
    _renderPipeline.destroy();
}

void GamePlayScreen::onEvent(const SDL_Event& e) {

    // Push the event to the input manager
    GameManager::inputManager->pushEvent(e);

    // Push event to PDA if its open
    if (_pda.isOpen()) {
        _pda.onEvent(e);
    }

    // Handle custom input
    switch (e.type) {
        case SDL_MOUSEMOTION:
            if (_inFocus) {
                // Pass mouse motion to the player
                _player->mouseMove(e.motion.xrel, e.motion.yrel);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            onMouseDown(e);
            break;
        case SDL_MOUSEBUTTONUP:
            onMouseUp(e);
            break;
        case SDL_WINDOWEVENT:
            if (e.window.type == SDL_WINDOWEVENT_LEAVE || e.window.type == SDL_WINDOWEVENT_FOCUS_LOST){
                 SDL_SetRelativeMouseMode(SDL_FALSE);
                 _inFocus = false;
                 SDL_StopTextInput();
            } else if (e.window.type == SDL_WINDOWEVENT_ENTER) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
                _inFocus = true;
                SDL_StartTextInput();
            }
        default:
            break;
    }
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
    _pda.update();

    // Sort all meshes // TODO(Ben): There is redundancy here
    _app->meshManager->sortMeshes(_player->headPosition);

    // Process any updates from the render thread
    processMessages();
}

void GamePlayScreen::draw(const GameTime& gameTime) {

    updateWorldCameraClip();

    _renderPipeline.render();
}

i32 GamePlayScreen::getWindowWidth() const {
    return _app->getWindow().getWidth();
}

i32 GamePlayScreen::getWindowHeight() const {
    return _app->getWindow().getHeight();
}

void GamePlayScreen::initRenderPipeline() {
    // Set up the rendering pipeline and pass in dependencies
    ui32v4 viewport(0, 0, _app->getWindow().getViewportDims());
    _renderPipeline.init(viewport, &_player->getChunkCamera(), &_player->getWorldCamera(), 
                         _app, _player, _app->meshManager, &_pda, GameManager::glProgramManager,
                         GameManager::chunkManager->getChunkSlots(0));
}

void GamePlayScreen::handleInput() {
    // Get input manager handle
    InputManager* inputManager = GameManager::inputManager;

    // Block placement
    if (!_pda.isOpen()) {
        if (inputManager->getKeyDown(INPUT_MOUSE_LEFT) || (GameManager::voxelEditor->isEditing() && inputManager->getKey(INPUT_BLOCK_DRAG))) {
            if (!(_player->leftEquippedItem)){
                GameManager::clickDragRay(true);
            } else if (_player->leftEquippedItem->type == ITEM_BLOCK){
                _player->dragBlock = _player->leftEquippedItem;
                GameManager::clickDragRay(false);
            }
        } else if (inputManager->getKeyDown(INPUT_MOUSE_RIGHT) || (GameManager::voxelEditor->isEditing() && inputManager->getKey(INPUT_BLOCK_DRAG))) {
            if (!(_player->rightEquippedItem)){
                GameManager::clickDragRay(true);
            } else if (_player->rightEquippedItem->type == ITEM_BLOCK){
                _player->dragBlock = _player->rightEquippedItem;
                GameManager::clickDragRay(false);
            }
        }
    }

    // Update inputManager internal state
    inputManager->update();
}

void GamePlayScreen::onMouseDown(const SDL_Event& e) {
    SDL_StartTextInput();
    if (!_pda.isOpen()) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        _inFocus = true;
    }
}

void GamePlayScreen::onMouseUp(const SDL_Event& e) {
    if (e.button.button == SDL_BUTTON_LEFT) {
        if (GameManager::voxelEditor->isEditing()) {
            GameManager::voxelEditor->editVoxels(_player->leftEquippedItem);
        }
    } else if (e.button.button == SDL_BUTTON_RIGHT) {
        if (GameManager::voxelEditor->isEditing()) {
            GameManager::voxelEditor->editVoxels(_player->rightEquippedItem);
        }
    }
}

void GamePlayScreen::updatePlayer() {
    double dist = _player->facePosition.y + GameManager::planet->radius;
    _player->update(_inFocus, GameManager::planet->getGravityAccel(dist), GameManager::planet->getAirFrictionForce(dist, glm::length(_player->velocity)));

    Chunk **chunks = new Chunk*[8];
    _player->isGrounded = 0;
    _player->setMoveMod(1.0f);
    _player->canCling = 0;
    _player->collisionData.yDecel = 0.0f;

    // Number of steps to integrate the collision over
    for (int i = 0; i < PLAYER_COLLISION_STEPS; i++){
        _player->gridPosition += (_player->velocity / (float)PLAYER_COLLISION_STEPS) * glSpeedFactor;
        _player->facePosition += (_player->velocity / (float)PLAYER_COLLISION_STEPS) * glSpeedFactor;
        _player->collisionData.clear();
        GameManager::voxelWorld->getClosestChunks(_player->gridPosition, chunks); //DANGER HERE!
        aabbChunkCollision(_player, &(_player->gridPosition), chunks, 8);
        _player->applyCollisionData();
    }

    delete[] chunks;
}

void GamePlayScreen::updateThreadFunc() {
    _threadRunning = true;

    FpsLimiter fpsLimiter;
    fpsLimiter.init(maxPhysicsFps);

    MessageManager* messageManager = GameManager::messageManager;

    Message message;

    while (_threadRunning) {
        fpsLimiter.beginFrame();

        GameManager::soundEngine->SetMusicVolume(soundOptions.musicVolume / 100.0f);
        GameManager::soundEngine->SetEffectVolume(soundOptions.effectVolume / 100.0f);
        GameManager::soundEngine->update();

        while (messageManager->tryDeque(THREAD, message)) {
            // Process the message
            switch (message.id) {
                
            }
        }

        f64v3 camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(_player->getWorldCamera().getPosition(), 1.0));

        GameManager::update();

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
                tmm = static_cast<TerrainMeshMessage*>(message.data);
                meshManager->updateTerrainMesh(tmm);
                break;
            case MessageID::REMOVE_TREES:
                tmm = static_cast<TerrainMeshMessage*>(message.data);
                if (tmm->terrainBuffers->treeVboID != 0) glDeleteBuffers(1, &(tmm->terrainBuffers->treeVboID));
                tmm->terrainBuffers->treeVboID = 0;
                delete tmm;
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
    _player->getWorldCamera().setClippingPlane(clip, MAX(300000000.0 / planetScale, closestTerrainPatchDistance + 10000000));
    _player->getWorldCamera().updateProjection();
}

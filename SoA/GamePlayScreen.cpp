#include "stdafx.h"
#include "GamePlayScreen.h"

#include "Player.h"
#include "FrameBuffer.h"
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

#define THREAD ThreadId::UPDATE

// Each mode includes the previous mode
enum DevUiModes { 
    DEVUIMODE_NONE, 
    DEVUIMODE_CROSSHAIR,
    DEVUIMODE_HANDS, 
    DEVUIMODE_FPS, 
    DEVUIMODE_ALL };

CTOR_APP_SCREEN_DEF(GamePlayScreen, App),
    _updateThread(nullptr),
    _threadRunning(false), 
    _inFocus(true),
    _devHudSpriteBatch(nullptr),
    _devHudSpriteFont(nullptr),
    _devHudMode(DEVUIMODE_CROSSHAIR) {
    // Empty
}

i32 GamePlayScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 GamePlayScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void GamePlayScreen::build(){

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

}

void GamePlayScreen::onExit(const GameTime& gameTime) {
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

    // Update the input
    handleInput();

    // Update the player
    updatePlayer();

    // Update the PDA
    _pda.update();

    // Sort all meshes // TODO(Ben): There is redundance here
    _app->meshManager->sortMeshes(_player->headPosition);

    // Process any updates from the render thread
    processMessages();
}

void GamePlayScreen::draw(const GameTime& gameTime) {
   
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
    _renderPipeline.init(viewport, &_player->getChunkCamera(), &_player->getWorldCamera(), _app->meshManager, GameManager::glProgramManager);
}

void GamePlayScreen::handleInput() {
    // Get input manager handle
    InputManager* inputManager = GameManager::inputManager;
    // Handle key inputs
    if (inputManager->getKeyDown(INPUT_PAUSE)) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        _inFocus = false;
    }
    if (inputManager->getKeyDown(INPUT_FLY)) {
        _player->flyToggle();
    }
    if (inputManager->getKeyDown(INPUT_GRID)) {
        gridState = !gridState;
    }
    if (inputManager->getKeyDown(INPUT_RELOAD_TEXTURES)) {
        // Free atlas
        vg::GpuMemory::freeTexture(blockPack.textureInfo.ID);
        // Free all textures
        GameManager::textureCache->destroy();
        // Reload textures
        GameManager::texturePackLoader->loadAllTextures("Textures/TexturePacks/" + graphicsOptions.texturePackString + "/");
        GameManager::texturePackLoader->uploadTextures();
        GameManager::texturePackLoader->writeDebugAtlases();
        GameManager::texturePackLoader->setBlockTextures(Blocks);

        GameManager::getTextureHandles();

        // Initialize all the textures for blocks.
        for (size_t i = 0; i < Blocks.size(); i++) {
            Blocks[i].InitializeTexture();
        }

        GameManager::texturePackLoader->destroy();
    }
    if (inputManager->getKeyDown(INPUT_RELOAD_SHADERS)) {
        GameManager::glProgramManager->destroy();
        LoadTaskShaders shaderTask;
        shaderTask.load();
        // Need to reinitialize the render pipeline with new shaders
        _renderPipeline.destroy();
        initRenderPipeline();
    }
    if (inputManager->getKeyDown(INPUT_INVENTORY)) {
        if (_pda.isOpen()) {
            _pda.close();
            SDL_SetRelativeMouseMode(SDL_TRUE);
            _inFocus = true;
            SDL_StartTextInput();
        } else {
            _pda.open();
            SDL_SetRelativeMouseMode(SDL_FALSE);
            _inFocus = false;
            SDL_StopTextInput();
        }
    }
    if (inputManager->getKeyDown(INPUT_RELOAD_UI)) {
        if (_pda.isOpen()) {
            _pda.close();
        }
        _pda.destroy();
        _pda.init(this);
    }

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

    // Dev hud
    if (inputManager->getKeyDown(INPUT_HUD)) {
        _devHudMode++;
        if (_devHudMode > DEVUIMODE_ALL) {
            _devHudMode = DEVUIMODE_NONE;
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

void GamePlayScreen::drawDevHUD() {
    f32v2 windowDims(_app->getWindow().getWidth(), _app->getWindow().getHeight());
    char buffer[256];
    // Lazily load spritebatch
    if (!_devHudSpriteBatch) {
        _devHudSpriteBatch = new SpriteBatch(true, true);
        _devHudSpriteFont = new SpriteFont("Fonts\\chintzy.ttf", 32);
    }

    _devHudSpriteBatch->begin();

    // Draw crosshair
    const f32v2 cSize(26.0f);
    _devHudSpriteBatch->draw(crosshairTexture.ID, 
                             (windowDims - cSize) / 2.0f,
                             cSize,
                             ColorRGBA8(255, 255, 255, 128));
    
    int offset = 0;
    int fontHeight = _devHudSpriteFont->getFontHeight();

    // Fps Counters
    if (_devHudMode >= DEVUIMODE_FPS) {
        
        std::sprintf(buffer, "Render FPS: %.0f", _app->getFps());
        _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                       buffer,
                                       f32v2(0.0f, fontHeight * offset++),
                                       f32v2(1.0f),
                                       color::White);

        std::sprintf(buffer, "Physics FPS: %.0f", physicsFps);
        _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                       buffer,
                                       f32v2(0.0f, fontHeight * offset++),
                                       f32v2(1.0f),
                                       color::White);
    }

    // Items in hands
    if (_devHudMode >= DEVUIMODE_HANDS) {
        const f32v2 SCALE(0.75f);
        // Left Hand
        if (_player->leftEquippedItem) {
            std::sprintf(buffer, "Left Hand: %s (%d)",
                        _player->leftEquippedItem->name.c_str(),
                        _player->leftEquippedItem->count);

            _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                           buffer,
                                           f32v2(0.0f, windowDims.y - fontHeight),
                                           SCALE,
                                           color::White);
        }
        // Right Hand
        if (_player->rightEquippedItem) {
            std::sprintf(buffer, "Right Hand: %s (%d)",
                         _player->rightEquippedItem->name.c_str(),
                         _player->rightEquippedItem->count);

            _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                           buffer,
                                           f32v2(windowDims.x - _devHudSpriteFont->measure(buffer).x, windowDims.y - fontHeight),
                                           SCALE,
                                           color::White);
        }
    }
    
    // Other debug text
    if (_devHudMode >= DEVUIMODE_ALL) {
        const f32v2 NUMBER_SCALE(0.75f);
        // Grid position
        offset++;
        _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                       "Grid Position",
                                       f32v2(0.0f, fontHeight * offset++),
                                       f32v2(1.0f),
                                       color::White);
        std::sprintf(buffer, "X %.2f", _player->headPosition.x);
        _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                       buffer,
                                       f32v2(0.0f, fontHeight * offset++),
                                       NUMBER_SCALE,
                                       color::White);
        std::sprintf(buffer, "Y %.2f", _player->headPosition.y);
        _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                       buffer,
                                       f32v2(0.0f, fontHeight * offset++),
                                       NUMBER_SCALE,
                                       color::White);
        std::sprintf(buffer, "Z %.2f", _player->headPosition.z);
        _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                       buffer,
                                       f32v2(0.0f, fontHeight * offset++),
                                       NUMBER_SCALE,
                                       color::White);
        
        // World position
        offset++;
        _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                       "World Position",
                                       f32v2(0.0f, fontHeight * offset++),
                                       f32v2(1.0f),
                                       color::White);
        std::sprintf(buffer, "X %-9.2f", _player->worldPosition.x);
        _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                       buffer,
                                       f32v2(0.0f, fontHeight * offset++),
                                       NUMBER_SCALE,
                                       color::White);
        std::sprintf(buffer, "Y %-9.2f", _player->worldPosition.y);
        _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                       buffer,
                                       f32v2(0.0f, fontHeight * offset++),
                                       NUMBER_SCALE,
                                       color::White);
        std::sprintf(buffer, "Z %-9.2f", _player->worldPosition.z);
        _devHudSpriteBatch->drawString(_devHudSpriteFont,
                                       buffer,
                                       f32v2(0.0f, fontHeight * offset++),
                                       NUMBER_SCALE,
                                       color::White);
    }

    _devHudSpriteBatch->end();
    // Render to the screen
    _devHudSpriteBatch->renderBatch(windowDims);
}

void GamePlayScreen::updatePlayer() {
    double dist = _player->facePosition.y + GameManager::planet->radius;
    _player->update(_inFocus, GameManager::planet->getGravityAccel(dist), GameManager::planet->getAirFrictionForce(dist, glm::length(_player->velocity)));

    Chunk **chunks = new Chunk*[8];
    _player->isGrounded = 0;
    _player->setMoveMod(1.0f);
    _player->canCling = 0;
    _player->collisionData.yDecel = 0.0f;
    //    cout << "C";

    // Number of steps to integrate the collision over
    Chunk::modifyLock.lock();
    for (int i = 0; i < PLAYER_COLLISION_STEPS; i++){
        _player->gridPosition += (_player->velocity / (float)PLAYER_COLLISION_STEPS) * glSpeedFactor;
        _player->facePosition += (_player->velocity / (float)PLAYER_COLLISION_STEPS) * glSpeedFactor;
        _player->collisionData.clear();
        GameManager::voxelWorld->getClosestChunks(_player->gridPosition, chunks); //DANGER HERE!
        aabbChunkCollision(_player, &(_player->gridPosition), chunks, 8);
        _player->applyCollisionData();
    }
    Chunk::modifyLock.unlock();

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

        f64v3 camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(_player->getWorldCamera().position(), 1.0));

        GameManager::update();

        physicsFps = fpsLimiter.endFrame();
    }
}

void GamePlayScreen::processMessages() {

    TerrainMeshMessage* tmm;
    Message message;

    MeshManager* meshManager = _app->meshManager;

    while (GameManager::messageManager->tryDeque(ThreadId::RENDERING, message)) {
        switch (message.id) {
            case MessageID::TERRAIN_MESH:
                meshManager->updateTerrainMesh(static_cast<TerrainMeshMessage*>(message.data));
                break;
            case MessageID::REMOVE_TREES:
                tmm = static_cast<TerrainMeshMessage*>(message.data);
                if (tmm->terrainBuffers->treeVboID != 0) glDeleteBuffers(1, &(tmm->terrainBuffers->treeVboID));
                tmm->terrainBuffers->treeVboID = 0;
                delete tmm;
                break;
            case MessageID::CHUNK_MESH:
                meshManager->updateChunkMesh((ChunkMeshData *)(message.data));
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
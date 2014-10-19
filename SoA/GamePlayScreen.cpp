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


#define THREAD ThreadName::PHYSICS

CTOR_APP_SCREEN_DEF(GamePlayScreen, App),
    _updateThread(nullptr),
    _threadRunning(false){
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
    _player->initialize("Ben"); //What an awesome name that is
    GameManager::initializeVoxelWorld(_player);

    // Initialize and run the update thread
    _updateThread = new std::thread(&GamePlayScreen::updateThreadFunc, this);

    // Initialize the cameras
    // Initialize the camera
    _planetCamera.setPosition(glm::dvec3(0.0, 0.0, 1000000000));
    _planetCamera.setDirection(glm::vec3(0.0, 0.0, -1.0));
    _planetCamera.setRight(glm::vec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0));
    _planetCamera.setUp(glm::cross(_planetCamera.right(), _planetCamera.direction()));
    _planetCamera.setClippingPlane(1000000.0f, 30000000.0f);

}

void GamePlayScreen::onExit(const GameTime& gameTime) {
    _threadRunning = false;
    _updateThread->join();
    delete _updateThread;
}

void GamePlayScreen::onEvent(const SDL_Event& e) {
    GameManager::inputManager->pushEvent(e);
}

void GamePlayScreen::update(const GameTime& gameTime) {
    MessageManager* messageManager = GameManager::messageManager;

    // Process any updates from the render thread
    processMessages();
}

void GamePlayScreen::draw(const GameTime& gameTime) {
    FrameBuffer* frameBuffer = _app->frameBuffer;

    frameBuffer->bind();
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    _app->drawFrameBuffer(_player->chunkProjectionMatrix() * _player->chunkViewMatrix());

    const ui32v2 viewPort(graphicsOptions.screenWidth, graphicsOptions.screenHeight);
    frameBuffer->unBind(viewPort);
}

void GamePlayScreen::updateThreadFunc() {
    _threadRunning = true;

    FpsLimiter fpsLimiter;
    fpsLimiter.init(maxPhysicsFps);

    MessageManager* messageManager = GameManager::messageManager;

    Message message;

    while (_threadRunning) {
        fpsLimiter.begin();

        GameManager::soundEngine->SetMusicVolume(soundOptions.musicVolume / 100.0f);
        GameManager::soundEngine->SetEffectVolume(soundOptions.effectVolume / 100.0f);
        GameManager::soundEngine->update();

        while (messageManager->tryDeque(THREAD, message)) {
            // Process the message
            switch (message.id) {
                
            }
        }

        _planetCamera.update();

        f64v3 camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(_planetCamera.position(), 1.0));

        GameManager::update();

        physicsFps = fpsLimiter.end();
    }
}

void GamePlayScreen::processMessages() {

    TerrainMeshMessage* tmm;
    Message message;

    MeshManager* meshManager = _app->meshManager;

    while (GameManager::messageManager->tryDeque(ThreadName::RENDERING, message)) {
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
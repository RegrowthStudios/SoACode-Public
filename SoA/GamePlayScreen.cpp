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
#include "GpuMemory.h"

#define THREAD ThreadName::PHYSICS

CTOR_APP_SCREEN_DEF(GamePlayScreen, App),
    _updateThread(nullptr),
    _threadRunning(false), 
    _inFocus(true){
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

    // Force him to fly... This is temporary
    _player->isFlying = true;

    SDL_SetRelativeMouseMode(SDL_TRUE);

}

void GamePlayScreen::onExit(const GameTime& gameTime) {
    _threadRunning = false;
    _updateThread->join();
    delete _updateThread;
    _app->meshManager->destroy();
}

void GamePlayScreen::onEvent(const SDL_Event& e) {

    // Push the event to the input manager
    GameManager::inputManager->pushEvent(e);

    // Handle custom input
    switch (e.type) {
        case SDL_MOUSEMOTION:
            if (_inFocus) {
                // Pass mouse motion to the player
                _player->mouseMove(e.motion.xrel, e.motion.yrel);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (e.button.button == SDL_BUTTON_LEFT) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
                _inFocus = true;
            }
        case SDL_WINDOWEVENT:
            if (e.window.type == SDL_WINDOWEVENT_LEAVE || e.window.type == SDL_WINDOWEVENT_FOCUS_LOST){
                 SDL_SetRelativeMouseMode(SDL_FALSE);
                 _inFocus = false;
            } else if (e.window.type == SDL_WINDOWEVENT_ENTER) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
                _inFocus = true;
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

    // Sort all meshes // TODO(Ben): There is redundance here
    _app->meshManager->sortMeshes(_player->headPosition);

    // Process any updates from the render thread
    processMessages();
}

void GamePlayScreen::draw(const GameTime& gameTime) {
    FrameBuffer* frameBuffer = _app->frameBuffer;

    frameBuffer->bind();
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ExtractFrustum(glm::dmat4(_player->worldProjectionMatrix()), glm::dmat4(_player->worldViewMatrix()), worldFrustum);
    ExtractFrustum(glm::dmat4(_player->chunkProjectionMatrix()), glm::dmat4(_player->chunkViewMatrix()), gridFrustum);
    
    drawVoxelWorld();

    glDisable(GL_DEPTH_TEST);
    _app->drawFrameBuffer(_player->chunkProjectionMatrix() * _player->chunkViewMatrix());
    glEnable(GL_DEPTH_TEST);

    const ui32v2 viewPort(graphicsOptions.screenWidth, graphicsOptions.screenHeight);
    frameBuffer->unBind(viewPort);
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
        GameManager::texturePackLoader->loadAllTextures();
        LoadTextures();
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
    // Update inputManager internal state
    inputManager->update();
}

void GamePlayScreen::drawVoxelWorld() {

    Camera& worldCamera = _player->getWorldCamera();
    Camera& chunkCamera = _player->getChunkCamera();

    MeshManager* meshManager = _app->meshManager;

    float FogColor[3];
    float fogStart, fogEnd;
    glm::vec3 lightPos = glm::vec3(1.0, 0.0, 0.0);
    float theta = glm::dot(glm::dvec3(lightPos), glm::normalize(glm::dvec3(glm::dmat4(GameManager::planet->rotationMatrix) * glm::dvec4(worldCamera.position(), 1.0))));

    glm::mat4 VP;
    //********************************* TODO: PRECOMPILED HEADERS for compilation speed?
    float fogTheta = glm::clamp(theta, 0.0f, 1.0f);
    fogStart = 0;
    if (_player->isUnderWater){
        GLfloat underwaterColor[4];
        underwaterColor[0] = fogTheta * 1.0 / 2;
        underwaterColor[1] = fogTheta * 1.0 / 2;
        underwaterColor[2] = fogTheta * 1.0 / 2;
        underwaterColor[3] = 1.0;
        fogEnd = 50 + fogTheta * 100;
        FogColor[0] = underwaterColor[0];
        FogColor[1] = underwaterColor[1];
        FogColor[2] = underwaterColor[2];
    } else{
        fogEnd = 100000;
        FogColor[0] = 1.0;
        FogColor[1] = 1.0;
        FogColor[2] = 1.0;
    }

    float st = MAX(0.0f, theta + 0.06);
    if (st > 1) st = 1;

    //far znear for maximum Terrain Patch z buffer precision
    //this is currently incorrect

    double nearClip = MIN((csGridWidth / 2.0 - 3.0)*32.0*0.7, 75.0) - ((double)(GameManager::chunkIOManager->getLoadListSize()) / (double)(csGridWidth*csGridWidth*csGridWidth))*55.0;
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
    lightPos = glm::normalize(glm::dvec3(glm::dmat4(glm::inverse(_player->worldRotationMatrix)) * glm::dmat4(GameManager::planet->invRotationMatrix) * glm::dvec4(lightPos, 1)));

    const glm::vec3 chunkDirection = chunkCamera.direction();

    ChunkRenderer::drawBlocks(meshManager->getChunkMeshes(), VP, chunkCamera.position(), lightPos, diffColor, _player->lightActive, ambVal, fogEnd, fogStart, FogColor, &(chunkDirection[0]));
    ChunkRenderer::drawCutoutBlocks(meshManager->getChunkMeshes(), VP, chunkCamera.position(), lightPos, diffColor, _player->lightActive, ambVal, fogEnd, fogStart, FogColor, &(chunkDirection[0]));

    // TODO(Ben): Globals are bad mkay?
    if (gridState != 0) {
        GameManager::voxelWorld->getChunkManager().drawChunkLines(VP, chunkCamera.position());
    }

    glDepthMask(GL_FALSE);
    ChunkRenderer::drawTransparentBlocks(meshManager->getChunkMeshes(), VP, chunkCamera.position(), lightPos, diffColor, _player->lightActive, ambVal, fogEnd, fogStart, FogColor, &(chunkDirection[0]));
    glDepthMask(GL_TRUE);

    if (sonarActive){
        glDisable(GL_DEPTH_TEST);
        ChunkRenderer::drawSonar(meshManager->getChunkMeshes(), VP, _player->headPosition);
        glEnable(GL_DEPTH_TEST);
    }


    if (GameManager::voxelEditor->isEditing()){
        int ID;
        if (_player->dragBlock != NULL){
            ID = _player->dragBlock->ID;
        } else{
            ID = 0;
        }

        GameManager::voxelEditor->drawGuides(chunkCamera.position(), VP, ID);
    }

    glLineWidth(1);

    // TODO(Ben): Render the physics blocks again.
  /*  if (physicsBlockMeshes.size()){
        DrawPhysicsBlocks(VP, chunkCamera.position(), lightPos, diffColor, player->lightActive, ambVal, fogEnd, fogStart, FogColor, &(chunkDirection[0]));
    }*/


    //********************Water********************
    ChunkRenderer::drawWater(meshManager->getChunkMeshes(), VP, chunkCamera.position(), st, fogEnd, fogStart, FogColor, lightPos, diffColor, _player->isUnderWater);

    // TODO(Ben): Render the particles
    //if (particleMeshes.size() > 0){
    //    vcore::GLProgram* bProgram = GameManager::glProgramManager->getProgram("Billboard");
    //    bProgram->use();

    //    glUniform1f(bProgram->getUniform("lightType"), (GLfloat)player->lightActive);
    //    glUniform3fv(bProgram->getUniform("eyeNormalWorldspace"), 1, &(chunkDirection[0]));
    //    glUniform1f(bProgram->getUniform("sunVal"), st);
    //    glUniform3f(bProgram->getUniform("AmbientLight"), (GLfloat)1.1f, (GLfloat)1.1f, (GLfloat)1.1f);

    //    const glm::mat4 &chunkViewMatrix = chunkCamera.viewMatrix();

    //    glm::vec3 cameraRight(chunkViewMatrix[0][0], chunkViewMatrix[1][0], chunkViewMatrix[2][0]);
    //    glm::vec3 cameraUp(chunkViewMatrix[0][1], chunkViewMatrix[1][1], chunkViewMatrix[2][1]);

    //    glUniform3f(bProgram->getUniform("cameraUp_worldspace"), cameraUp.x, cameraUp.y, cameraUp.z);
    //    glUniform3f(bProgram->getUniform("cameraRight_worldspace"), cameraRight.x, cameraRight.y, cameraRight.z);


    //    //glDepthMask(GL_FALSE);

    //    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //    for (size_t i = 0; i < particleMeshes.size(); i++){

    //        if (particleMeshes[i]->animated){
    //            ParticleBatch::drawAnimated(particleMeshes[i], player->headPosition, VP);
    //        } else{
    //            ParticleBatch::draw(particleMeshes[i], player->headPosition, VP);
    //        }
    //    }
    //    // TODO(Ben): Maybe make this a part of GLProgram?
    //    glVertexAttribDivisor(0, 0);
    //    glVertexAttribDivisor(2, 0); //restore divisors
    //    glVertexAttribDivisor(3, 0);
    //    glVertexAttribDivisor(4, 0);
    //    glVertexAttribDivisor(5, 0);


    //    bProgram->unuse();
    //}
    GameManager::debugRenderer->render(VP, glm::vec3(chunkCamera.position()));
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
        _player->gridPosition += _player->velocity * (1.0f / PLAYER_COLLISION_STEPS) * glSpeedFactor;
        _player->facePosition += _player->velocity * (1.0f / PLAYER_COLLISION_STEPS) * glSpeedFactor;
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
        fpsLimiter.begin();

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
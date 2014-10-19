#include "stdafx.h"

#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>

#include "App.h"
#include "GamePlayScreen.h"
#include "MainMenuScreen.h"
#include "IAwesomiumAPI.h"
#include "ChunkManager.h"
#include "Errors.h"
#include "Player.h"
#include "Planet.h"
#include "InputManager.h"
#include "Inputs.h"
#include "GameManager.h"
#include "OpenglManager.h"
#include "Sound.h"
#include "Options.h"
#include "MessageManager.h"
#include "VoxelEditor.h"
#include "Frustum.h"
#include "TerrainPatch.h"
#include "FrameBuffer.h"
#include "FileSystem.h"
#include "MeshManager.h"

#define THREAD ThreadName::PHYSICS

CTOR_APP_SCREEN_DEF(MainMenuScreen, App) ,
    _updateThread(nullptr),
    _threadRunning(false){
    // Empty
}

i32 MainMenuScreen::getNextScreen() const {
    return _app->scrGamePlay->getIndex();
}
i32 MainMenuScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void MainMenuScreen::build() {
    // Empty
}

void MainMenuScreen::destroy(const GameTime& gameTime) {
    // Empty
}

void MainMenuScreen::onEntry(const GameTime& gameTime) {

    // Initialize the camera
    _camera.setPosition(glm::dvec3(0.0, 0.0, 1000000000));
    _camera.setDirection(glm::vec3(0.0, 0.0, -1.0));
    _camera.setRight(glm::vec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0));
    _camera.setUp(glm::cross(_camera.right(), _camera.direction()));
    _camera.setClippingPlane(1000000.0f, 30000000.0f);
    _camera.zoomTo(glm::dvec3(0.0, 0.0, GameManager::planet->radius * 1.35), 3.0, glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0), glm::dvec3(0.0), GameManager::planet->radius, 0.0);

    // Initialize the user interface
    _awesomiumInterface.init("UI/MainMenu/", "index.html", graphicsOptions.screenWidth, graphicsOptions.screenHeight, &_api, this);

    // Run the update thread for updating the planet
    _updateThread = new thread(&MainMenuScreen::updateThreadFunc, this);
}

void MainMenuScreen::onExit(const GameTime& gameTime) {
    _threadRunning = false;
    _updateThread->join();
    delete _updateThread;
    _awesomiumInterface.destroy();
}

void MainMenuScreen::onEvent(const SDL_Event& e) {
    _awesomiumInterface.handleEvent(e);
    GameManager::inputManager->pushEvent(e);

    // Check for reloading the UI
    if (GameManager::inputManager->getKeyDown(INPUT_RELOAD_UI)) {
        std::cout << "\n\nReloading MainMenu UI...\n\n";
        _awesomiumInterface.destroy();
        _awesomiumInterface.init("UI/MainMenu/", "index.html", graphicsOptions.screenWidth, graphicsOptions.screenHeight, &_api, this);
    }
}

void MainMenuScreen::update(const GameTime& gameTime) {

    _awesomiumInterface.update();

    _camera.update();
    GameManager::inputManager->update();

    MeshManager* meshManager = _app->meshManager;

    TerrainMeshMessage* tmm;
    Message message;
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
            default:
                break;
        }
    }

    bdt += glSpeedFactor * 0.01;
}

void MainMenuScreen::draw(const GameTime& gameTime) {

    FrameBuffer* frameBuffer = _app->frameBuffer;

    // Bind the framebuffer and clear it
    frameBuffer->bind();
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    // Set the camera clipping plane for rendering the skybox and update the projection matrix
    _camera.setClippingPlane(1000000.0f, 30000000.0f);
    _camera.updateProjection();
    glm::mat4 VP = _camera.projectionMatrix() * _camera.viewMatrix();

    // Draw space
    GameManager::drawSpace(VP, 0);
    
    // Calculate the near clipping plane
    double clip = closestTerrainPatchDistance / (sqrt(1.0f + pow(tan(graphicsOptions.fov / 2.0), 2.0) * (pow((double)graphicsOptions.screenWidth / graphicsOptions.screenHeight, 2.0) + 1.0))*2.0);
    if (clip < 100) clip = 100;

    // Set the clipping plane for the camera for the planet
    _camera.setClippingPlane(clip, MAX(300000000.0 / planetScale, closestTerrainPatchDistance + 10000000));
    _camera.updateProjection();

    VP = _camera.projectionMatrix() * _camera.viewMatrix();

    glm::dmat4 fvm;
    fvm = glm::lookAt(
        glm::dvec3(0.0),           // Camera is here
        glm::dvec3(glm::dmat4(GameManager::planet->invRotationMatrix) * glm::dvec4(_camera.direction(), 1.0)), // and looks here : at the same position, plus "direction"
        glm::dvec3(_camera.up())                  // Head is up (set to 0,-1,0 to look upside-down)
        );

    // Extract the frustum for frustum culling
    ExtractFrustum(glm::dmat4(_camera.projectionMatrix()), fvm, worldFrustum);

    // Draw the planet using the _camera
    GameManager::drawPlanet(_camera.position(), VP, _camera.viewMatrix(), 1.0, glm::vec3(1.0, 0.0, 0.0), 1000, 0);

    glDisable(GL_DEPTH_TEST);
    
    // Render the framebuffer with HDR post processing
    // TODO(Ben): fix this
    const f32m4 identity(1.0f);
    _app->drawFrameBuffer(identity);

    const ui32v2 viewPort(graphicsOptions.screenWidth, graphicsOptions.screenHeight);
    frameBuffer->unBind(viewPort);

    // Render the awesomium user interface
    _awesomiumInterface.draw(GameManager::glProgramManager->getProgram("Texture2D"));
    glEnable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MainMenuScreen::loadGame(const nString& fileName) {
    std::cout << "Loading Game: " << fileName << std::endl;

    // Make the save directories, in case they were deleted
    fileManager.makeSaveDirectories(fileName);
    if (fileManager.setSaveFile(fileName) != 0) {
        cout << "Could not set save file.\n";
        return;
    }
    // Check the planet string
    string planetName = fileManager.getWorldString(fileName + "/World/");
    if (planetName == "") {
        cout << "NO PLANET NAME";
        return;
    }

    // Set the save file path
    GameManager::saveFilePath = fileName;
    // Check the chunk version
    GameManager::chunkIOManager->checkVersion();

    _state = ScreenState::CHANGE_NEXT;
}

void MainMenuScreen::updateThreadFunc() {

    _threadRunning = true;

    Message message;

    MessageManager* messageManager = GameManager::messageManager;
    /*
    messageManager->waitForMessage(THREAD, MessageID::DONE, message);
    if (message.id == MessageID::QUIT) {
        std::terminate();
    }*/

    FpsLimiter fpsLimiter;
    fpsLimiter.init(maxPhysicsFps);

    while (_threadRunning) {

        fpsLimiter.begin();

        GameManager::soundEngine->SetMusicVolume(soundOptions.musicVolume / 100.0f);
        GameManager::soundEngine->SetEffectVolume(soundOptions.effectVolume / 100.0f);
        GameManager::soundEngine->update();

        while (messageManager->tryDeque(THREAD, message)) {
            // Process the message
            switch (message.id) {
                case MessageID::NEW_PLANET:
                    messageManager->enqueue(THREAD, Message(MessageID::NEW_PLANET, NULL));
                    messageManager->enqueue(THREAD, Message(MessageID::DONE, NULL));
                    messageManager->waitForMessage(THREAD, MessageID::DONE, message);
                    break;
            }
        }

        f64v3 camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(_camera.position(), 1.0));

        GameManager::planet->rotationUpdate();
        GameManager::updatePlanet(camPos, 10);
        
        physicsFps = fpsLimiter.end();
    }
}

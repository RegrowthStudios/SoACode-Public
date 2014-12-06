#include "stdafx.h"

#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>

#include "App.h"
#include "ChunkManager.h"
#include "Errors.h"
#include "FileSystem.h"
#include "Frustum.h"
#include "GameManager.h"
#include "GamePlayScreen.h"
#include "IAwesomiumAPI.h"
#include "InputManager.h"
#include "Inputs.h"
#include "LoadTaskShaders.h"
#include "MainMenuScreen.h"
#include "MeshManager.h"
#include "MessageManager.h"
#include "Options.h"
#include "Player.h"
#include "Sound.h"
#include "SpaceSystem.h"
#include "TerrainPatch.h"
#include "VoxelEditor.h"

#define THREAD ThreadId::UPDATE

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
    _camera.init(_app->getWindow().getAspectRatio());
    _camera.setPosition(glm::dvec3(0.0, 20000000000.0, 0.0));
    _camera.setDirection(glm::vec3(0.0, -1.0, 0.0));
    _camera.setUp(glm::cross(_camera.getRight(), _camera.getDirection()));
    _camera.setClippingPlane(10000.0f, 3000000000000.0f);
    _camera.zoomTo(glm::dvec3(0.0, 80000000000.0, 0.0), 3.0, glm::dvec3(0.0, -1.0, 0.0), glm::dvec3(-1.0, 0, 0.0), 0.0);

    // Initialize the user interface
    _awesomiumInterface.init("UI/MainMenu/",
                             "MainMenu_UI",
                             "index.html", 
                             _app->getWindow().getWidth(), 
                             _app->getWindow().getHeight(),
                             this);

    // Init rendering
    initRenderPipeline();

    // Run the update thread for updating the planet
    _updateThread = new std::thread(&MainMenuScreen::updateThreadFunc, this);
}

void MainMenuScreen::onExit(const GameTime& gameTime) {
    _threadRunning = false;
    _updateThread->join();
    delete _updateThread;
    _awesomiumInterface.destroy();
    _renderPipeline.destroy();
}

void MainMenuScreen::onEvent(const SDL_Event& e) {
    _awesomiumInterface.handleEvent(e);
    GameManager::inputManager->pushEvent(e);

    // Check for reloading the UI
    if (GameManager::inputManager->getKeyDown(INPUT_RELOAD_UI)) {
        std::cout << "\n\nReloading MainMenu UI...\n\n";
        _awesomiumInterface.destroy();
        _awesomiumInterface.init("UI/MainMenu/",
                                 "MainMenu_UI", 
                                 "index.html", 
                                 _app->getWindow().getWidth(),
                                 _app->getWindow().getHeight(),
                                 this);
    }

    
}

void MainMenuScreen::update(const GameTime& gameTime) {


    if (GameManager::inputManager->getKey(INPUT_MOUSE_RIGHT)) {
        f32v3 eulerAngles(0.0, 0.01, 0.0);
        f32q rot(eulerAngles);
        _camera.applyRotation(rot);
    }
    if (GameManager::inputManager->getKey(INPUT_MOUSE_LEFT)) {
        f32v3 eulerAngles(0.01, 0.0, 0.0);
        f32q rot(eulerAngles);
        _camera.applyRotation(rot);
    }

    _awesomiumInterface.update();

    _camera.update();
    GameManager::inputManager->update();

    MeshManager* meshManager = _app->meshManager;

    TerrainMeshMessage* tmm;
    Message message;
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
            default:
                break;
        }
    }
    // Check for shader reload
    if (GameManager::inputManager->getKeyDown(INPUT_RELOAD_SHADERS)) {
        GameManager::glProgramManager->destroy();
        LoadTaskShaders shaderTask;
        shaderTask.load();
        // Reload the pipeline with new shaders
        _renderPipeline.destroy();
        initRenderPipeline();
    }

    bdt += glSpeedFactor * 0.01;
}

void MainMenuScreen::draw(const GameTime& gameTime) {

    updateWorldCameraClip();

    _renderPipeline.render();
}

void MainMenuScreen::initRenderPipeline() {
    // Set up the rendering pipeline and pass in dependencies
    ui32v4 viewport(0, 0, _app->getWindow().getViewportDims());
    _renderPipeline.init(viewport, &_camera, &_awesomiumInterface,
                         _app->spaceSystem, GameManager::glProgramManager);
}

void MainMenuScreen::loadGame(const nString& fileName) {
    std::cout << "Loading Game: " << fileName << std::endl;

    // Make the save directories, in case they were deleted
    fileManager.makeSaveDirectories(fileName);
    if (fileManager.setSaveFile(fileName) != 0) {
        std::cout << "Could not set save file.\n";
        return;
    }
    // Check the planet string
    nString planetName = fileManager.getWorldString(fileName + "/World/");
    if (planetName == "") {
        std::cout << "NO PLANET NAME";
        return;
    }

    // Set the save file path
    GameManager::saveFilePath = fileName;
    // Check the chunk version
    GameManager::chunkIOManager->checkVersion();

    _state = ScreenState::CHANGE_NEXT;
}


void MainMenuScreen::newGame(const nString& fileName) {
    std::cout << "Making new game: " << fileName << std::endl;

    // Make the save directories, in case they were deleted
    fileManager.makeSaveDirectories(fileName);
    if (fileManager.setSaveFile(fileName) != 0) {
        std::cout << "Could not set save file.\n";
        return;
    }
   
    // Save the world file
    nString worldText("Aldrin");
    _ioManager.writeStringToFile((fileName + "/World/world.txt").c_str(), worldText);

    // Set the save file path
    GameManager::saveFilePath = fileName;
    // Save the chunk version
    GameManager::chunkIOManager->saveVersionFile();

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

        fpsLimiter.beginFrame();

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

      //  f64v3 camPos = glm::dvec3((glm::dmat4(GameManager::planet->invRotationMatrix)) * glm::dvec4(_camera.getPosition(), 1.0));
        static double time = 0.0;
        time += 10.0;

        _app->spaceSystem->update(time);
        
        physicsFps = fpsLimiter.endFrame();
    }
}

void MainMenuScreen::updateWorldCameraClip() {
    //far znear for maximum Terrain Patch z buffer precision
    //this is currently incorrect
    //double nearClip = MIN((csGridWidth / 2.0 - 3.0)*32.0*0.7, 75.0) - ((double)(30.0) / (double)(csGridWidth*csGridWidth*csGridWidth))*55.0;
    //if (nearClip < 0.1) nearClip = 0.1;
    //double a = 0.0;
    //// TODO(Ben): This is crap fix it (Sorry Brian)
    //a = closestTerrainPatchDistance / (sqrt(1.0f + pow(tan(graphicsOptions.fov / 2.0), 2.0) * (pow((double)_app->getWindow().getAspectRatio(), 2.0) + 1.0))*2.0);
    //if (a < 0) a = 0;

    //double clip = MAX(nearClip / planetScale * 0.5, a);
    double clip = 10000.0;
    // The world camera has a dynamic clipping plane
//    _camera.setClippingPlane(1.0f, 200.0f);
 //   _camera.setClippingPlane(clip, MAX(300000000.0 / planetScale, closestTerrainPatchDistance + 10000000));
    _camera.updateProjection();
}

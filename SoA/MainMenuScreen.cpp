#include "stdafx.h"
#include "MainMenuScreen.h"

#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>

#include "App.h"

#include "ChunkManager.h"
#include "Errors.h"
#include "FileSystem.h"
#include "Frustum.h"
#include "GameManager.h"
#include "GamePlayScreen.h"
#include "GamePlayScreen.h"
#include "IAwesomiumAPI.h"
#include "IAwesomiumAPI.h"
#include "InputManager.h"
#include "Inputs.h"
#include "LoadTaskShaders.h"
#include "MainMenuScreen.h"
#include "MainMenuScreenEvents.hpp"
#include "MainMenuSystemViewer.h"
#include "MeshManager.h"
#include "MessageManager.h"
#include "Options.h"
#include "SoAState.h"
#include "Sound.h"
#include "SpaceSystem.h"
#include "SphericalTerrainPatch.h"
#include "VoxelEditor.h"

#define THREAD ThreadId::UPDATE

CTOR_APP_SCREEN_DEF(MainMenuScreen, App) ,
    m_updateThread(nullptr),
    m_threadRunning(false){
    // Empty
}

MainMenuScreen::~MainMenuScreen() {
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

    m_soaState = std::make_unique<SoaState>();

    m_camera.init(_app->getWindow().getAspectRatio());

    m_inputManager = new InputManager;

    m_mainMenuSystemViewer = std::make_unique<MainMenuSystemViewer>(_app->getWindow().getViewportDims(),
                                                                    &m_camera, _app->spaceSystem, m_inputManager);

    // Initialize the user interface
    m_awesomiumInterface.init("UI/MainMenu/",
                             "MainMenu_UI",
                             "index.html", 
                             _app->getWindow().getWidth(), 
                             _app->getWindow().getHeight(),
                             this);

   

    // Init rendering
    initRenderPipeline();
    m_onReloadShadersKeyDown = m_inputManager->subscribe(INPUT_RELOAD_SHADERS, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnMainMenuReloadShadersKeyDown(this));

    // Run the update thread for updating the planet
    m_updateThread = new std::thread(&MainMenuScreen::updateThreadFunc, this);

    m_inputManager->startInput();
}

void MainMenuScreen::onExit(const GameTime& gameTime) {
    m_inputManager->stopInput();

    m_mainMenuSystemViewer.reset();

    m_threadRunning = false;
    m_updateThread->join();
    delete m_updateThread;
    m_awesomiumInterface.destroy();
    m_renderPipeline.destroy();

    m_inputManager->unsubscribe(INPUT_RELOAD_SHADERS, InputManager::EventType::DOWN, m_onReloadShadersKeyDown);
    delete m_onReloadShadersKeyDown;

    delete m_inputManager;
}

void MainMenuScreen::onEvent(const SDL_Event& e) {

    // Check for reloading the UI
    if (m_inputManager->getKeyDown(INPUT_RELOAD_UI)) {
        std::cout << "\n\nReloading MainMenu UI...\n\n";
        m_awesomiumInterface.destroy();
        m_awesomiumInterface.init("UI/MainMenu/",
                                 "MainMenu_UI", 
                                 "index.html",
                                 _app->getWindow().getWidth(),
                                 _app->getWindow().getHeight(),
                                 this);
    }

}

void MainMenuScreen::update(const GameTime& gameTime) {

    m_awesomiumInterface.update();
    
    static double time = 0.0;
    time += 0.0001;

    m_mainMenuSystemViewer->update();

    _app->spaceSystem->update(time, m_camera.getPosition());
    _app->spaceSystem->glUpdate();

    m_camera.update();
    m_inputManager->update(); // TODO: Remove

    MeshManager* meshManager = _app->meshManager;

    TerrainMeshMessage* tmm;
    Message message;
    while (GameManager::messageManager->tryDeque(ThreadId::RENDERING, message)) {
        switch (message.id) {
            case MessageID::TERRAIN_MESH:
          //      meshManager->updateTerrainMesh(static_cast<TerrainMeshMessage*>(message.data));
                break;
            case MessageID::REMOVE_TREES:
                /*       tmm = static_cast<TerrainMeshMessage*>(message.data);
                       if (tmm->terrainBuffers->treeVboID != 0) glDeleteBuffers(1, &(tmm->terrainBuffers->treeVboID));
                       tmm->terrainBuffers->treeVboID = 0;*/
                delete tmm;
                break;
            default:
                break;
        }
    }

    // Check for shader reload
    if (m_inputManager->getKeyDown(INPUT_RELOAD_SHADERS)) {
        GameManager::glProgramManager->destroy();
        LoadTaskShaders shaderTask(nullptr);
        shaderTask.load();
        // Reload the pipeline with new shaders
        m_renderPipeline.destroy();
        initRenderPipeline();
    }

    bdt += glSpeedFactor * 0.01;
}

void MainMenuScreen::draw(const GameTime& gameTime) {

    updateWorldCameraClip();

    m_renderPipeline.render();
}

void MainMenuScreen::initRenderPipeline() {
    // Set up the rendering pipeline and pass in dependencies
    ui32v4 viewport(0, 0, _app->getWindow().getViewportDims());
    m_renderPipeline.init(viewport, &m_camera, &m_awesomiumInterface,
                         _app->spaceSystem, m_mainMenuSystemViewer.get(),
                         GameManager::glProgramManager);
}

void MainMenuScreen::loadGame(const nString& fileName) {
    std::cout << "Loading Game: " << fileName << std::endl;

    // Make the save directories, in case they were deleted
    fileManager.makeSaveDirectories(fileName);
    if (!m_soaState->saveFileIom.directoryExists(fileName.c_str())) {
        std::cout << "Could not set save file.\n";
        return;
    }
    m_soaState->saveFileIom.setSearchDirectory(fileName.c_str());

    // Check the planet string
    nString planetName = fileManager.getWorldString(fileName + "/World/");
    if (planetName == "") {
        std::cout << "NO PLANET NAME";
        return;
    }

    _state = ScreenState::CHANGE_NEXT;
}


void MainMenuScreen::newGame(const nString& fileName) {

    if (m_mainMenuSystemViewer->getSelectedCubeFace() == -1) {
        return;
    }

    const f32v2& selectedGridPos = m_mainMenuSystemViewer->getSelectedGridPos();

    m_soaState->isNewGame = true;
    m_soaState->startFace = m_mainMenuSystemViewer->getSelectedCubeFace();
    m_soaState->startGridPos = f32v3(selectedGridPos.x, 0.0f, selectedGridPos.y);

    std::cout << "Making new game: " << fileName << std::endl;

    // Make the save directories, in case they were deleted
    fileManager.makeSaveDirectories(fileName);
    if (!m_soaState->saveFileIom.directoryExists(fileName.c_str())) {
        std::cout << "Could not set save file.\n";
        return;
    }
    m_soaState->saveFileIom.setSearchDirectory(fileName.c_str());

    // Save the world file
    nString worldText("Aldrin");
    m_ioManager.writeStringToFile((fileName + "/World/world.txt").c_str(), worldText);

    _state = ScreenState::CHANGE_NEXT;
}

void MainMenuScreen::updateThreadFunc() {

    m_threadRunning = true;

    Message message;

    MessageManager* messageManager = GameManager::messageManager;
    /*
    messageManager->waitForMessage(THREAD, MessageID::DONE, message);
    if (message.id == MessageID::QUIT) {
        std::terminate();
    }*/

    FpsLimiter fpsLimiter;
    fpsLimiter.init(maxPhysicsFps);

    while (m_threadRunning) {

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
    m_camera.updateProjection();
}

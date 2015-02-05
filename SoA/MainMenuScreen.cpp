#include "stdafx.h"
#include "MainMenuScreen.h"

#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>
#include <Vorb/sound/SoundEngine.h>
#include <Vorb/sound/SoundListener.h>

#include "AmbienceLibrary.h"
#include "AmbiencePlayer.h"
#include "App.h"

#include "ChunkManager.h"
#include "DebugRenderer.h"
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
#include "LoadScreen.h"
#include "LoadTaskShaders.h"
#include "MainMenuScreen.h"
#include "MainMenuSystemViewer.h"
#include "MeshManager.h"
#include "MessageManager.h"
#include "Options.h"
#include "SoAState.h"
#include "SoaEngine.h"
#include "Sound.h"
#include "SpaceSystem.h"
#include "SpaceSystemUpdater.h"
#include "SphericalTerrainPatch.h"
#include "VoxelEditor.h"

#define THREAD ThreadId::UPDATE

MainMenuScreen::MainMenuScreen(const App* app, const LoadScreen* loadScreen) :
    IAppScreen<App>(app),
    m_loadScreen(loadScreen),
    m_updateThread(nullptr),
    m_threadRunning(false) {
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

    // Get the state handle
    m_soaState = m_loadScreen->getSoAState();
                             
    m_camera.init(_app->getWindow().getAspectRatio());

    initInput();

    m_mainMenuSystemViewer = std::make_unique<MainMenuSystemViewer>(_app->getWindow().getViewportDims(),
                                                                    &m_camera, m_soaState->spaceSystem.get(), m_inputManager);
    m_engine = new vsound::Engine;
    m_engine->init();
    m_ambLibrary = new AmbienceLibrary;
    m_ambLibrary->addTrack("Menu", "Track1", "Data/Music/Abyss.mp3");
    m_ambLibrary->addTrack("Menu", "Track2", "Data/Music/BGM Creepy.mp3");
    m_ambLibrary->addTrack("Menu", "Track3", "Data/Music/BGM Unknown.mp3");
    m_ambLibrary->addTrack("Menu", "Track4", "Data/Music/Stranded.mp3");
    m_ambPlayer = new AmbiencePlayer;
    m_ambPlayer->init(m_engine, m_ambLibrary);
    m_ambPlayer->setToTrack("Menu", 50);

    m_spaceSystemUpdater = std::make_unique<SpaceSystemUpdater>();

    // Initialize the user interface
    m_awesomiumInterface.init("UI/MainMenu/",
                             "MainMenu_UI",
                             "index.html", 
                             _app->getWindow().getWidth(), 
                             _app->getWindow().getHeight(),
                             this);

    // Init rendering
    initRenderPipeline();

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

    delete m_inputManager;

    m_ambPlayer->dispose();
    m_engine->dispose();
    delete m_ambLibrary;
    delete m_ambPlayer;
    delete m_engine;
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
    
    m_mainMenuSystemViewer->update();

    m_soaState->time += 0.0001;
    m_spaceSystemUpdater->update(m_soaState->spaceSystem.get(), m_soaState->gameSystem.get(), m_soaState, m_camera.getPosition());
    m_spaceSystemUpdater->glUpdate(m_soaState->spaceSystem.get());

    m_camera.update();
    m_inputManager->update(); // TODO: Remove

    // Check for shader reload
    //if (m_inputManager->getKeyDown(INPUT_RELOAD_SHADERS)) {
    //    GameManager::glProgramManager->destroy();
    //    LoadTaskShaders shaderTask(nullptr);
    //    shaderTask.load();
    //    // Reload the pipeline with new shaders
    //    m_renderPipeline.destroy();
    //    initRenderPipeline();
    //}

    bdt += glSpeedFactor * 0.01;

    m_ambPlayer->update((f32)gameTime.elapsed);
    m_engine->update(vsound::Listener());
}

void MainMenuScreen::draw(const GameTime& gameTime) {

    updateWorldCameraClip();

    m_renderPipeline.render();
}

void MainMenuScreen::initInput() {
    m_inputManager = new InputManager;
    initInputs(m_inputManager);

    // Reload space system event
    m_inputManager->subscribeFunctor(INPUT_RELOAD_SYSTEM, InputManager::EventType::DOWN, [&](Sender s, ui32 a) -> void {
        SoaEngine::destroySpaceSystem(m_soaState);
        SoaEngine::SpaceSystemLoadData loadData;
        loadData.filePath = "StarSystems/Trinity";
        SoaEngine::loadSpaceSystem(m_soaState, loadData);
        m_mainMenuSystemViewer = std::make_unique<MainMenuSystemViewer>(_app->getWindow().getViewportDims(),
                                                                        &m_camera, m_soaState->spaceSystem.get(), m_inputManager);
        m_renderPipeline.destroy();
        m_renderPipeline = MainMenuRenderPipeline();
        initRenderPipeline();
    });
}

void MainMenuScreen::initRenderPipeline() {
    // Set up the rendering pipeline and pass in dependencies
    ui32v4 viewport(0, 0, _app->getWindow().getViewportDims());
    m_renderPipeline.init(viewport, &m_camera, &m_awesomiumInterface,
                          m_soaState->spaceSystem.get(), m_mainMenuSystemViewer.get(),
                          m_soaState->glProgramManager.get());
}

void MainMenuScreen::loadGame(const nString& fileName) {
    std::cout << "Loading Game: " << fileName << std::endl;

    initSaveIomanager(fileName);

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
    m_soaState->startSpacePos = m_mainMenuSystemViewer->getClickPos();
    m_soaState->startingPlanet = m_mainMenuSystemViewer->getSelectedPlanet();

    std::cout << "Making new game: " << fileName << std::endl;

    initSaveIomanager(fileName);  

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

void MainMenuScreen::initSaveIomanager(const vio::Path& savePath) {

    vio::IOManager& ioManager = m_soaState->saveFileIom;
    // Make sure the Saves and savePath directories exist
    ioManager.setSearchDirectory("");
    ioManager.makeDirectory("Saves");
    ioManager.makeDirectory(savePath);

    ioManager.setSearchDirectory(savePath);

    ioManager.makeDirectory("players");
    ioManager.makeDirectory("system");
    ioManager.makeDirectory("cache");
}

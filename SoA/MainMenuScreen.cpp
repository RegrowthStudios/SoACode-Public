#include "stdafx.h"
#include "MainMenuScreen.h"

#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>
#include <Vorb/sound/SoundEngine.h>
#include <Vorb/sound/SoundListener.h>

#include "AmbienceLibrary.h"
#include "AmbiencePlayer.h"
#include "App.h"

#include "DebugRenderer.h"
#include "Errors.h"
#include "FileSystem.h"
#include "Frustum.h"
#include "GameManager.h"
#include "GameplayScreen.h"
#include "GameplayScreen.h"
#include "InputMapper.h"
#include "Inputs.h"
#include "LoadScreen.h"
#include "MainMenuScreen.h"
#include "MainMenuSystemViewer.h"
#include "MeshManager.h"
#include "SoaOptions.h"
#include "SoAState.h"
#include "SoaEngine.h"
#include "SpaceSystem.h"
#include "SpaceSystemUpdater.h"
#include "TerrainPatch.h"
#include "VoxelEditor.h"

MainMenuScreen::MainMenuScreen(const App* app, vui::GameWindow* window, const LoadScreen* loadScreen) :
    IAppScreen<App>(app),
    m_loadScreen(loadScreen),
    m_updateThread(nullptr),
    m_threadRunning(false),
    m_window(window) {
    // Empty
}

MainMenuScreen::~MainMenuScreen() {
    // Empty
}

i32 MainMenuScreen::getNextScreen() const {
    return m_app->scrGamePlay->getIndex();
}

i32 MainMenuScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void MainMenuScreen::build() {
    // Empty
}

void MainMenuScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void MainMenuScreen::onEntry(const vui::GameTime& gameTime) {

    // Get the state handle
    m_soaState = m_loadScreen->getSoAState();
                             
    m_camera.init(m_window->getAspectRatio());

    initInput();

    m_mainMenuSystemViewer = std::make_unique<MainMenuSystemViewer>(m_window->getViewportDims(),
                                                                    &m_camera, m_soaState->spaceSystem.get(), m_inputMapper);
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
    m_spaceSystemUpdater->init(m_soaState);

    // Initialize the user interface
    m_formFont.init("Fonts/orbitron_bold-webfont.ttf", 32);
    initUI();

    // Init rendering
    initRenderPipeline();

    // Run the update thread for updating the planet
    m_updateThread = new std::thread(&MainMenuScreen::updateThreadFunc, this);

}

void MainMenuScreen::onExit(const vui::GameTime& gameTime) {
    vui::InputDispatcher::window.onResize -= makeDelegate(*this, &MainMenuScreen::onWindowResize);
    vui::InputDispatcher::window.onClose -= makeDelegate(*this, &MainMenuScreen::onWindowClose);
    SoaEngine::optionsController.OptionsChange -= makeDelegate(*this, &MainMenuScreen::onOptionsChange);
    m_inputMapper->stopInput();
    m_formFont.dispose();
    m_ui.dispose();

    m_mainMenuSystemViewer.reset();

    m_threadRunning = false;
    m_updateThread->join();
    delete m_updateThread;
    m_renderPipeline.destroy(true);

    delete m_inputMapper;

    m_ambPlayer->dispose();
    m_engine->dispose();
    delete m_ambLibrary;
    delete m_ambPlayer;
    delete m_engine;
}

void MainMenuScreen::update(const vui::GameTime& gameTime) {

    // Check for UI reload
    if (m_shouldReloadUI) {
        reloadUI();
    }

    m_ui.update();

    { // Handle time warp
        const f64 TIME_WARP_SPEED = 1000.0 + (f64)m_inputMapper->getInputState(INPUT_SPEED_TIME) * 10000.0;
        if (m_inputMapper->getInputState(INPUT_TIME_BACK)) m_soaState->time -= TIME_WARP_SPEED;
        if (m_inputMapper->getInputState(INPUT_TIME_FORWARD)) m_soaState->time += TIME_WARP_SPEED;
    }

    m_soaState->time += m_soaState->timeStep;
    m_spaceSystemUpdater->update(m_soaState, m_camera.getPosition(), f64v3(0.0));
    m_spaceSystemUpdater->glUpdate(m_soaState);
    m_mainMenuSystemViewer->update();

    bdt += glSpeedFactor * 0.01;

    m_ambPlayer->update((f32)gameTime.elapsed);
    m_engine->update(vsound::Listener());
}

void MainMenuScreen::draw(const vui::GameTime& gameTime) {

    m_camera.updateProjection();

    m_renderPipeline.render();
}

void MainMenuScreen::initInput() {
    m_inputMapper = new InputMapper;
    initInputs(m_inputMapper);
    // Reload space system event
  
    m_inputMapper->get(INPUT_RELOAD_SYSTEM).downEvent += makeDelegate(*this, &MainMenuScreen::onReloadSystem);
    m_inputMapper->get(INPUT_RELOAD_SHADERS).downEvent += makeDelegate(*this, &MainMenuScreen::onReloadShaders);
    m_inputMapper->get(INPUT_RELOAD_UI).downEvent.addFunctor([&](Sender s, ui32 i) { m_shouldReloadUI = true; });
    m_inputMapper->get(INPUT_EXIT).downEvent += makeDelegate(*this, &MainMenuScreen::onQuit);
    m_inputMapper->get(INPUT_TOGGLE_UI).downEvent.addFunctor([&](Sender s, ui32 i) {
        m_renderPipeline.toggleUI(); });
    m_inputMapper->get(INPUT_TOGGLE_AR).downEvent.addFunctor([&](Sender s, ui32 i) {
        m_renderPipeline.toggleAR(); });
    m_inputMapper->get(INPUT_CYCLE_COLOR_FILTER).downEvent.addFunctor([&](Sender s, ui32 i) {
        m_renderPipeline.cycleColorFilter(); });
    m_inputMapper->get(INPUT_SCREENSHOT).downEvent.addFunctor([&](Sender s, ui32 i) {
        m_renderPipeline.takeScreenshot(); });

    vui::InputDispatcher::window.onResize += makeDelegate(*this, &MainMenuScreen::onWindowResize);
    vui::InputDispatcher::window.onClose += makeDelegate(*this, &MainMenuScreen::onWindowClose);
    SoaEngine::optionsController.OptionsChange += makeDelegate(*this, &MainMenuScreen::onOptionsChange);

    m_inputMapper->startInput();
}

void MainMenuScreen::initRenderPipeline() {
    // Set up the rendering pipeline and pass in dependencies
    ui32v4 viewport(0, 0, m_window->getViewportDims());
    m_renderPipeline.init(m_soaState, viewport, &m_ui, &m_camera,
                          m_soaState->spaceSystem.get(),
                          m_mainMenuSystemViewer.get());
}

void MainMenuScreen::initUI() {
    const ui32v2& vdims = m_window->getViewportDims();
    m_ui.init("Data/UI/Forms/main_menu.form.lua", this, &m_app->getWindow(), f32v4(0.0f, 0.0f, (f32)vdims.x, (f32)vdims.y), &m_formFont);
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

    m_state = vui::ScreenState::CHANGE_NEXT;
}

void MainMenuScreen::newGame(const nString& fileName) {

    if (m_mainMenuSystemViewer->getSelectedCubeFace() == -1) {
        return;
    }

    m_soaState->isNewGame = true;
    m_soaState->startFace = m_mainMenuSystemViewer->getSelectedCubeFace();
    m_soaState->startSpacePos = m_mainMenuSystemViewer->getClickPos();
    m_soaState->startingPlanet = m_mainMenuSystemViewer->getSelectedPlanet();

    std::cout << "Making new game: " << fileName << std::endl;

    initSaveIomanager(fileName);  

    m_state = vui::ScreenState::CHANGE_NEXT;
}

void MainMenuScreen::updateThreadFunc() {

    m_threadRunning = true;

    /*
    messageManager->waitForMessage(THREAD, MessageID::DONE, message);
    if (message.id == MessageID::QUIT) {
        std::terminate();
    }*/

    FpsLimiter fpsLimiter;
    fpsLimiter.init(maxPhysicsFps);

    while (m_threadRunning) {

        fpsLimiter.beginFrame();

        physicsFps = fpsLimiter.endFrame();
    }
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

void MainMenuScreen::reloadUI() {
    m_ui.dispose();
    initUI();

    m_shouldReloadUI = false;
    printf("UI was reloaded.\n");
}

void MainMenuScreen::onReloadSystem(Sender s, ui32 a) {
    SoaEngine::destroySpaceSystem(m_soaState);
    SoaEngine::SpaceSystemLoadData loadData;
    loadData.filePath = "StarSystems/Trinity";
    SoaEngine::loadSpaceSystem(m_soaState, loadData);
    CinematicCamera tmp = m_camera; // Store camera so the view doesn't change
    m_mainMenuSystemViewer = std::make_unique<MainMenuSystemViewer>(m_window->getViewportDims(),
                                                                    &m_camera, m_soaState->spaceSystem.get(), m_inputMapper);
    m_camera = tmp; // Restore old camera
    m_renderPipeline.destroy(true);
    m_renderPipeline = MainMenuRenderPipeline();
    initRenderPipeline();
}

void MainMenuScreen::onReloadShaders(Sender s, ui32 a) {
    printf("Reloading Shaders\n");
    m_renderPipeline.reloadShaders();
}

void MainMenuScreen::onQuit(Sender s, ui32 a) {
    SoaEngine::destroyAll(m_soaState);
    exit(0);
}

void MainMenuScreen::onWindowResize(Sender s, const vui::WindowResizeEvent& e) {
    SoaEngine::optionsController.setInt("Screen Width", e.w);
    SoaEngine::optionsController.setInt("Screen Height", e.h);
    soaOptions.get(OPT_SCREEN_WIDTH).value.i = e.w;
    soaOptions.get(OPT_SCREEN_HEIGHT).value.i = e.h;
    m_ui.onOptionsChanged();
    m_camera.setAspectRatio(m_window->getAspectRatio());
}

void MainMenuScreen::onWindowClose(Sender s) {
    onQuit(s, 0);
}

void MainMenuScreen::onOptionsChange(Sender s) {
    m_window->setScreenSize(soaOptions.get(OPT_SCREEN_WIDTH).value.i, soaOptions.get(OPT_SCREEN_HEIGHT).value.i);
    m_window->setBorderless(soaOptions.get(OPT_BORDERLESS).value.b);
    m_window->setFullscreen(soaOptions.get(OPT_FULLSCREEN).value.b);
    if (soaOptions.get(OPT_VSYNC).value.b) {
        m_window->setSwapInterval(vui::GameSwapInterval::V_SYNC);
    } else {
        m_window->setSwapInterval(vui::GameSwapInterval::UNLIMITED_FPS);
    }
    TerrainPatch::setQuality(soaOptions.get(OPT_PLANET_DETAIL).value.i);
}

#include "stdafx.h"
#include "MainMenuScreen.h"

#include <Vorb/sound/SoundEngine.h>
#include <Vorb/sound/SoundListener.h>

#include "AmbienceLibrary.h"
#include "AmbiencePlayer.h"
#include "App.h"

#include "DebugRenderer.h"
#include "Errors.h"
#include "Frustum.h"
#include "GameManager.h"
#include "GamePlayScreen.h"
#include "GameplayLoadScreen.h"
#include "InputMapper.h"
#include "Inputs.h"
#include "MainMenuLoadScreen.h"
#include "MainMenuScreen.h"
#include "MainMenuSystemViewer.h"
#include "SoaOptions.h"
#include "SoAState.h"
#include "SoaEngine.h"
#include "SpaceSystem.h"
#include "SpaceSystemUpdater.h"
#include "TerrainPatch.h"
#include "VoxelEditor.h"

MainMenuScreen::MainMenuScreen(const App* app, CommonState* state) :
    IAppScreen<App>(app),
    m_updateThread(nullptr),
    m_threadRunning(false),
    m_commonState(state),
    m_soaState(state->state),
    m_window(state->window) {
    // Empty
}

MainMenuScreen::~MainMenuScreen() {
    // Empty
}

i32 MainMenuScreen::getNextScreen() const {
    return m_app->scrGameplayLoad->getIndex();
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
    m_mainMenuSystemViewer = m_soaState->clientState.systemViewer;

    m_soaState->clientState.spaceCamera.init(m_window->getAspectRatio());
    

    initInput();

    m_soaState->clientState.systemViewer->init(m_window->getViewportDims(),
                                               &m_soaState->clientState.spaceCamera, m_soaState->spaceSystem, m_inputMapper);
    m_mainMenuSystemViewer = m_soaState->clientState.systemViewer;

    m_ambLibrary = new AmbienceLibrary;
    m_ambLibrary->addTrack("Menu", "Andromeda Fallen", "Data/Sound/Music/Andromeda Fallen.ogg");
    m_ambLibrary->addTrack("Menu", "Brethren", "Data/Sound/Music/Brethren.mp3");
    m_ambLibrary->addTrack("Menu", "Crystalite", "Data/Sound/Music/Crystalite.mp3");
    m_ambLibrary->addTrack("Menu", "Stranded", "Data/Sound/Music/Stranded.mp3");
    m_ambLibrary->addTrack("Menu", "Toxic Haze", "Data/Sound/Music/Toxic Haze.mp3");
    m_ambLibrary->addTrack("Menu", "BGM Unknown", "Data/Sound/Music/BGM Unknown.mp3");
    m_ambPlayer = new AmbiencePlayer;
    m_ambPlayer->init(m_commonState->soundEngine, m_ambLibrary);

    m_spaceSystemUpdater = std::make_unique<SpaceSystemUpdater>();
    m_spaceSystemUpdater->init(m_soaState);

    // Initialize the user interface
    m_formFont.init("Fonts/orbitron_bold-webfont.ttf", 32);
    initUI();

    // Init rendering
    initRenderPipeline();

    // TODO(Ben): Do this or something
    // Run the update thread for updating the planet
    //m_updateThread = new std::thread(&MainMenuScreen::updateThreadFunc, this);

    m_ambPlayer->setVolume(soaOptions.get(OPT_MUSIC_VOLUME).value.f);
    m_ambPlayer->setToTrack("Menu", 3);

    m_isFullscreen = soaOptions.get(OPT_BORDERLESS).value.b;
    m_isBorderless = soaOptions.get(OPT_FULLSCREEN).value.b;
}

void MainMenuScreen::onExit(const vui::GameTime& gameTime) {
    vui::InputDispatcher::window.onResize -= makeDelegate(*this, &MainMenuScreen::onWindowResize);
    vui::InputDispatcher::window.onClose -= makeDelegate(*this, &MainMenuScreen::onWindowClose);
    SoaEngine::optionsController.OptionsChange -= makeDelegate(*this, &MainMenuScreen::onOptionsChange);
    m_renderer.setShowUI(false);
    m_formFont.dispose();
    m_ui.dispose();

    m_soaState->clientState.systemViewer->stopInput();

    m_threadRunning = false;
    //m_updateThread->join();
    //delete m_updateThread;

    delete m_inputMapper;

    m_ambPlayer->dispose();
    delete m_ambLibrary;
    delete m_ambPlayer;
}

void MainMenuScreen::update(const vui::GameTime& gameTime) {

    // Check for UI reload
    if (m_shouldReloadUI) {
        reloadUI();
    }

    if (m_newGameClicked) newGame("TEST");

    if (m_uiEnabled) m_ui.update();

    { // Handle time warp
        const f64 TIME_WARP_SPEED = 1000.0 + (f64)m_inputMapper->getInputState(INPUT_SPEED_TIME) * 10000.0;
        bool isWarping = false;
        if (m_inputMapper->getInputState(INPUT_TIME_BACK)) {
            isWarping = true;
            m_soaState->time -= TIME_WARP_SPEED;
        }
        if (m_inputMapper->getInputState(INPUT_TIME_FORWARD)) {
            isWarping = true;
            m_soaState->time += TIME_WARP_SPEED;
        }
        if (isWarping) {
            m_soaState->clientState.spaceCamera.setSpeed(1.0);
        } else {
            m_soaState->clientState.spaceCamera.setSpeed(0.3);
        }
    }

    //m_soaState->time += m_soaState->timeStep;
    m_spaceSystemUpdater->update(m_soaState, m_soaState->clientState.spaceCamera.getPosition(), f64v3(0.0));
    m_spaceSystemUpdater->glUpdate(m_soaState);
    m_mainMenuSystemViewer->update();

    m_ambPlayer->update((f32)gameTime.elapsed);
    m_commonState->soundEngine->update(vsound::Listener());
}

void MainMenuScreen::draw(const vui::GameTime& gameTime) {
    m_soaState->clientState.spaceCamera.updateProjection();
    m_renderer.render();
}

void MainMenuScreen::initInput() {
    m_inputMapper = new InputMapper;
    initInputs(m_inputMapper);
    // Reload space system event
  
    m_inputMapper->get(INPUT_RELOAD_SYSTEM).downEvent += makeDelegate(*this, &MainMenuScreen::onReloadSystem);
    m_inputMapper->get(INPUT_RELOAD_SHADERS).downEvent += makeDelegate(*this, &MainMenuScreen::onReloadShaders);
    m_inputMapper->get(INPUT_RELOAD_UI).downEvent.addFunctor([&](Sender s, ui32 i) { m_shouldReloadUI = true; });
    m_inputMapper->get(INPUT_TOGGLE_UI).downEvent += makeDelegate(*this, &MainMenuScreen::onToggleUI);
    // TODO(Ben): addFunctor = memory leak
    m_inputMapper->get(INPUT_TOGGLE_AR).downEvent.addFunctor([&](Sender s, ui32 i) {
        m_renderer.toggleAR(); });
    m_inputMapper->get(INPUT_CYCLE_COLOR_FILTER).downEvent.addFunctor([&](Sender s, ui32 i) {
        m_renderer.cycleColorFilter(); });
    m_inputMapper->get(INPUT_SCREENSHOT).downEvent.addFunctor([&](Sender s, ui32 i) {
        m_renderer.takeScreenshot(); });
    m_inputMapper->get(INPUT_DRAW_MODE).downEvent += makeDelegate(*this, &MainMenuScreen::onToggleWireframe);

    vui::InputDispatcher::window.onResize += makeDelegate(*this, &MainMenuScreen::onWindowResize);
    vui::InputDispatcher::window.onClose += makeDelegate(*this, &MainMenuScreen::onWindowClose);
    SoaEngine::optionsController.OptionsChange += makeDelegate(*this, &MainMenuScreen::onOptionsChange);

    m_inputMapper->startInput();
}

void MainMenuScreen::initRenderPipeline() {
    //m_renderer.init(m_window, m_commonState->loadContext, this);
}

void MainMenuScreen::initUI() {
    const ui32v2& vdims = m_window->getViewportDims();
    m_ui.init("Data/UI/Forms/main_menu.form.lua", this, &m_app->getWindow(), f32v4(0.0f, 0.0f, (f32)vdims.x, (f32)vdims.y), &m_formFont);
}

void MainMenuScreen::loadGame(const nString& fileName) {
    //std::cout << "Loading Game: " << fileName << std::endl;

    //initSaveIomanager(fileName);

    //// Check the planet string
    //nString planetName = fileManager.getWorldString(fileName + "/World/");
    //if (planetName == "") {
    //    std::cout << "NO PLANET NAME";
    //    return;
    //}

    //m_state = vui::ScreenState::CHANGE_NEXT;
}

void MainMenuScreen::newGame(const nString& fileName) {

    if (!m_mainMenuSystemViewer->getSelectedPlanet()) {
        m_newGameClicked = false;
        return;
    }

    m_soaState->clientState.isNewGame = true;
    m_soaState->clientState.startSpacePos = m_mainMenuSystemViewer->getClickPos();
    f64v3 normal = glm::normalize(m_soaState->clientState.startSpacePos);
    // Don't spawn underwater
    if (glm::length(m_soaState->clientState.startSpacePos) < m_mainMenuSystemViewer->getTargetRadius()) {
        m_soaState->clientState.startSpacePos = normal * m_mainMenuSystemViewer->getTargetRadius();
    }
    // Push out by 5 voxels
    m_soaState->clientState.startSpacePos += glm::normalize(m_soaState->clientState.startSpacePos) * 5.0 * KM_PER_VOXEL;

    m_soaState->clientState.startingPlanet = m_mainMenuSystemViewer->getSelectedPlanet();
    vecs::EntityID startingPlanet = m_soaState->clientState.startingPlanet;

    { // Compute start location
        SpaceSystem* spaceSystem = m_soaState->spaceSystem;
        auto& arcmp = spaceSystem->axisRotation.getFromEntity(startingPlanet);

        m_soaState->clientState.startSpacePos = arcmp.currentOrientation * m_soaState->clientState.startSpacePos;
    }

    std::cout << "Making new game: " << fileName << std::endl;

    initSaveIomanager(fileName);  

    m_state = vui::ScreenState::CHANGE_NEXT;
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
    SoaEngine::loadSpaceSystem(m_soaState, "StarSystems/Trinity");
    CinematicCamera tmp = m_soaState->clientState.spaceCamera; // Store camera so the view doesn't change
    m_soaState->clientState.systemViewer->init(m_window->getViewportDims(),
                                               &m_soaState->clientState.spaceCamera, m_soaState->spaceSystem,
                                   m_inputMapper);
    m_soaState->clientState.spaceCamera = tmp; // Restore old camera
    m_renderer.dispose(m_commonState->loadContext);
    initRenderPipeline();
}

void MainMenuScreen::onReloadShaders(Sender s, ui32 a) {
    printf("Reloading Shaders...\n");
    m_renderer.dispose(m_commonState->loadContext);

    m_renderer.init(m_commonState->window, m_commonState->loadContext, this, m_commonState);
    m_renderer.hook();
    m_commonState->loadContext.begin();
    m_renderer.load(m_commonState->loadContext);

    
    while (!m_renderer.isLoaded()) {
        m_commonState->loadContext.processRequests(1);
        SDL_Delay(1);
    }

    m_commonState->loadContext.end();

    printf("Done!\n");
}

void MainMenuScreen::onQuit(Sender s, ui32 a) {
    m_window->saveSettings();
    SoaEngine::destroyAll(m_soaState);
    exit(0);
}

void MainMenuScreen::onWindowResize(Sender s, const vui::WindowResizeEvent& e) {
    SoaEngine::optionsController.setInt("Screen Width", e.w);
    SoaEngine::optionsController.setInt("Screen Height", e.h);
    soaOptions.get(OPT_SCREEN_WIDTH).value.i = e.w;
    soaOptions.get(OPT_SCREEN_HEIGHT).value.i = e.h;
    if (m_uiEnabled) m_ui.onOptionsChanged();
    m_soaState->clientState.spaceCamera.setAspectRatio(m_window->getAspectRatio());
    m_mainMenuSystemViewer->setViewport(ui32v2(e.w, e.h));
}

void MainMenuScreen::onWindowClose(Sender s) {
    onQuit(s, 0);
}

void MainMenuScreen::onOptionsChange(Sender s) {
    bool fullscreen = soaOptions.get(OPT_FULLSCREEN).value.b;
    bool borderless = soaOptions.get(OPT_BORDERLESS).value.b;
    bool screenChanged = false;
    ui32v2 screenSize = m_window->getViewportDims();
    if (screenSize.x != (ui32)soaOptions.get(OPT_SCREEN_WIDTH).value.i ||
        screenSize.y != (ui32)soaOptions.get(OPT_SCREEN_HEIGHT).value.i) {
        m_window->setScreenSize(soaOptions.get(OPT_SCREEN_WIDTH).value.i, soaOptions.get(OPT_SCREEN_HEIGHT).value.i);
        screenChanged = true;
    }
    m_window->setFullscreen(fullscreen);
    m_window->setBorderless(borderless);

    if (soaOptions.get(OPT_VSYNC).value.b) {
        m_window->setSwapInterval(vui::GameSwapInterval::V_SYNC);
    } else {
        m_window->setSwapInterval(vui::GameSwapInterval::UNLIMITED_FPS);
    }
    TerrainPatch::setQuality(soaOptions.get(OPT_PLANET_DETAIL).value.i);
    m_ambPlayer->setVolume(soaOptions.get(OPT_MUSIC_VOLUME).value.f);

    // Re-center the window
    if (screenChanged || m_isFullscreen != fullscreen || m_isBorderless != borderless) {
        m_isFullscreen = fullscreen;
        m_isBorderless = borderless;
        m_window->setPosition(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
}

void MainMenuScreen::onToggleUI(Sender s, ui32 i) {
    m_renderer.toggleUI();
    m_uiEnabled = !m_uiEnabled;
    if (m_uiEnabled) {
        initUI();
    } else {
        m_ui.dispose();
    }
}

void MainMenuScreen::onToggleWireframe(Sender s, ui32 i) {
    m_renderer.toggleWireframe();
}
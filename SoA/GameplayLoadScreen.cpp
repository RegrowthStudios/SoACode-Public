#include "stdafx.h"
#include "GameplayLoadScreen.h"

#include "App.h"
#include "CommonState.h"
#include "GamePlayScreen.h"
#include "LoadTaskBlockData.h"
#include "MainMenuScreen.h"
#include "LoadTaskTextures.h"
#include "SoaEngine.h"
#include "SoAState.h"

GameplayLoadScreen::GameplayLoadScreen(const App* app, CommonState* state, MainMenuScreen* mainMenuScreen, GameplayScreen* gameplayScreen) :
IAppScreen<App>(app),
m_commonState(state),
m_mainMenuScreen(mainMenuScreen),
m_gameplayScreen(gameplayScreen) {
    // Empty
}

i32 GameplayLoadScreen::getNextScreen() const {
    return m_app->scrGamePlay->getIndex();
}

i32 GameplayLoadScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void GameplayLoadScreen::build() {
    // Empty
}

void GameplayLoadScreen::destroy(const vui::GameTime& gameTime VORB_UNUSED) {
    // Empty
}

void GameplayLoadScreen::onEntry(const vui::GameTime& gameTime VORB_MAYBE_UNUSED) {

    addLoadTask("BlockData", new LoadTaskBlockData(&m_commonState->state->blocks,
        &m_commonState->state->clientState.blockTextureLoader,
        &m_commonState->loadContext));

  //  addLoadTask("Textures", new LoadTaskTextures);
  //  m_monitor.setDep("Textures", "BlockData");

    m_gameplayScreen->m_renderer.init(m_commonState->window, m_commonState->loadContext, m_gameplayScreen, m_commonState);
    m_gameplayScreen->m_renderer.hook();
    m_commonState->loadContext.begin();
    m_gameplayScreen->m_renderer.load(m_commonState->loadContext);

    // Start the tasks
    m_monitor.start();
}

void GameplayLoadScreen::onExit(const vui::GameTime& gameTime VORB_MAYBE_UNUSED) {
    // Dispose our borrowed renderer
    m_mainMenuScreen->m_renderer.dispose(m_commonState->loadContext);
    // Disable main menu viewer
    m_commonState->stages.spaceSystem.setSystemViewer(nullptr);
    m_commonState->loadContext.end();
}

void GameplayLoadScreen::update(const vui::GameTime& gameTime VORB_MAYBE_UNUSED) {

    // Perform OpenGL calls
    m_glrpc.processRequests(1);
    m_commonState->loadContext.processRequests(1);
    m_gameplayScreen->m_renderer.updateGL();

    // Defer texture loading
    static bool loadedTextures = false;
    // End condition
    if (!loadedTextures && m_gameplayScreen->m_renderer.isLoaded() && m_monitor.isTaskFinished("BlockData")) {
        // Post process the planets
        for (auto& it : m_commonState->state->spaceSystem->sphericalTerrain) {
            // auto& cmp = it.second;
            SoaEngine::initVoxelGen(it.second.planetGenData, m_commonState->state->blocks);
        }
        m_commonState->state->clientState.blockTextures->update();
        m_commonState->state->clientState.blockTextures->writeDebugAtlases();
        //m_commonState->state->blockTextures->save(&m_commonState->state->blocks);
        m_state = vui::ScreenState::CHANGE_NEXT;
        loadedTextures = true;
    }
}

void GameplayLoadScreen::draw(const vui::GameTime& gameTime VORB_MAYBE_UNUSED) {
    m_commonState->state->clientState.spaceCamera.updateProjection();
    m_mainMenuScreen->m_renderer.render();
}

void GameplayLoadScreen::addLoadTask(const nString& name, ILoadTask* task) {
    // Add the load task to the monitor
    m_loadTasks.push_back(task);
    m_monitor.addTask(name, m_loadTasks.back());
}
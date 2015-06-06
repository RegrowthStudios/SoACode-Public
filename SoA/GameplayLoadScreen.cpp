#include "stdafx.h"
#include "GameplayLoadScreen.h"

#include "App.h"
#include "CommonState.h"
#include "GameplayScreen.h"
#include "LoadTaskBlockData.h"
#include "LoadTaskTextures.h"
#include "SoaEngine.h"

GameplayLoadScreen::GameplayLoadScreen(const App* app, CommonState* state, MainMenuScreen* mainMenuScreen) :
IAppScreen<App>(app),
m_commonState(state),
m_monitor(),
m_glrpc(),
m_mainMenuScreen(mainMenuScreen) {
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

void GameplayLoadScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void GameplayLoadScreen::onEntry(const vui::GameTime& gameTime) {

    addLoadTask("BlockData", new LoadTaskBlockData);

    addLoadTask("Textures", new LoadTaskTextures);
    m_monitor.setDep("Textures", "BlockData");

    // Start the tasks
    m_monitor.start();
}

void GameplayLoadScreen::onExit(const vui::GameTime& gameTime) {
    // Empty
}

void GameplayLoadScreen::update(const vui::GameTime& gameTime) {

    // Perform OpenGL calls
    m_glrpc.processRequests(1);

    // Defer texture loading
    static bool loadedTextures = false;
    if (!loadedTextures && m_monitor.isTaskFinished("Textures")) {
        GameManager::texturePackLoader->uploadTextures();
        GameManager::texturePackLoader->writeDebugAtlases();
        GameManager::texturePackLoader->setBlockTextures(Blocks);

        GameManager::getTextureHandles();

        SetBlockAvgTexColors();

        //load the emitters
        for (size_t i = 0; i < Blocks.size(); i++) {
            if (Blocks[i].active) {
                if (Blocks[i].emitterName.size()) {
                //    Blocks[i].emitter = fileManager.loadEmitter(Blocks[i].emitterName);
                }
                if (Blocks[i].emitterOnBreakName.size()) {
               //     Blocks[i].emitterOnBreak = fileManager.loadEmitter(Blocks[i].emitterOnBreakName);
                }
                if (Blocks[i].emitterRandomName.size()) {
                //    Blocks[i].emitterRandom = fileManager.loadEmitter(Blocks[i].emitterRandomName);
                }
            }
        }

        // It has no texture
        for (i32 i = 0; i < 6; i++) Blocks[0].base[i] = -1;

        // Post process the planets
        SoaEngine::setPlanetBlocks(m_commonState->state);
        m_state = vui::ScreenState::CHANGE_NEXT;
        loadedTextures = true;
    }
}

void GameplayLoadScreen::draw(const vui::GameTime& gameTime) {
    throw std::logic_error("The method or operation is not implemented.");
}

void GameplayLoadScreen::addLoadTask(const nString& name, ILoadTask* task) {
    // Add the load task to the monitor
    m_loadTasks.push_back(task);
    m_monitor.addTask(name, m_loadTasks.back());
}
#include "stdafx.h"
#include "LoadScreen.h"

#include <Vorb/colors.h>
#include <Vorb/graphics/GLStates.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/graphics/SpriteBatch.h>

#include "App.h"
#include "BlockPack.h"
#include "ChunkMeshManager.h"
#include "DebugRenderer.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "Inputs.h"
#include "LoadTaskBlockData.h"
#include "LoadTaskGameManager.h"
#include "LoadTaskShaders.h"
#include "LoadTaskStarSystem.h"
#include "LoadTaskTextures.h"
#include "MainMenuScreen.h"
#include "MeshManager.h"
#include "MusicPlayer.h"
#include "ParticleEmitter.h"
#include "SoaFileSystem.h"
#include "SoaState.h"

#include "TexturePackLoader.h"

const color4 LOAD_COLOR_TEXT(205, 205, 205, 255);
const color4 LOAD_COLOR_BG_LOADING(105, 5, 5, 255);
const color4 LOAD_COLOR_BG_FINISHED(25, 105, 5, 255);

CTOR_APP_SCREEN_DEF(LoadScreen, App),
_sf(nullptr),
_sb(nullptr),
_monitor(),
m_glrpc() {
    // Empty
}

LoadScreen::~LoadScreen() {
    // Empty
}

i32 LoadScreen::getNextScreen() const {
    return _app->scrMainMenu->getIndex();
}
i32 LoadScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void LoadScreen::build() {
    // Empty
}
void LoadScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void LoadScreen::onEntry(const vui::GameTime& gameTime) {
    SoaFileSystem fs;
    fs.init();
    MusicPlayer mp;
    mp.refreshLists(fs);

    m_soaState = std::make_unique<SoaState>();
    SoaEngine::initState(m_soaState.get());

    // Make LoadBar Resources
    _sb = new vg::SpriteBatch(true, true);
    _sf = new vg::SpriteFont();
    _sf->init("Fonts/orbitron_bold-webfont.ttf", 32);

    // Add Tasks Here
    addLoadTask("GameManager", "Core Systems", new LoadTaskGameManager);
  
    addLoadTask("Shaders", "Shaders", new LoadTaskShaders(&m_glrpc, m_soaState->glProgramManager.get()));
    _monitor.setDep("Shaders", "GameManager");

    addLoadTask("BlockData", "Block Data", new LoadTaskBlockData);
    _monitor.setDep("BlockData", "GameManager");

    addLoadTask("SpaceSystem", "SpaceSystem", new LoadTaskStarSystem(&m_glrpc, "StarSystems/Trinity", m_soaState.get()));
    _monitor.setDep("SpaceSystem", "GameManager");

    addLoadTask("Textures", "Textures", new LoadTaskTextures);
    _monitor.setDep("Textures", "BlockData");
    _monitor.setDep("Textures", "SpaceSystem");

    // Start the tasks
    _monitor.start();

    // Clear State For The Screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
}
void LoadScreen::onExit(const vui::GameTime& gameTime) {
    _sf->dispose();
    delete _sf;
    _sf = nullptr;

    _sb->dispose();
    delete _sb;
    _sb = nullptr;

    // Free the vector memory
    std::vector<LoadBar>().swap(_loadBars);

    for (ui32 i = 0; i < _loadTasks.size(); i++) {
        // Free memory
        delete _loadTasks[i];
        _loadTasks[i] = nullptr;
    }
    std::vector<ILoadTask*>().swap(_loadTasks);

    // Restore default rasterizer state
    vg::RasterizerState::CULL_CLOCKWISE.set();
}

void LoadScreen::update(const vui::GameTime& gameTime) {
    static ui64 fCounter = 0;

    for (ui32 i = 0; i < _loadTasks.size(); i++) {
        if (_loadTasks[i] != nullptr && _loadTasks[i]->isFinished()) {
            // Make The Task Visuals Disappear
            _loadBars[i].setColor(color::Black, color::Teal);
            _loadBars[i].retract();
        }

        // Update Visual Position
        _loadBars[i].update((f32)gameTime.elapsed);
    }

    // Perform OpenGL calls
    fCounter++;
    m_glrpc.processRequests(1);

    // Defer texture loading
    static bool loadedTextures = false;
    if (!loadedTextures && _monitor.isTaskFinished("Textures")) {
        GameManager::texturePackLoader->uploadTextures();
        GameManager::texturePackLoader->writeDebugAtlases();
        GameManager::texturePackLoader->setBlockTextures(Blocks);

        GameManager::getTextureHandles();

        SetBlockAvgTexColors();

        //load the emitters
        for (int i = 0; i < Blocks.size(); i++) {
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
        SoaEngine::setPlanetBlocks(m_soaState.get());
        _state = vui::ScreenState::CHANGE_NEXT;
        loadedTextures = true;
        
        
    }
}
void LoadScreen::draw(const vui::GameTime& gameTime) {
    const vui::GameWindow* w = &_game->getWindow();

    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _sb->begin();
    for (ui32 i = 0; i < _loadTasks.size(); i++) {
        _loadBars[i].draw(_sb, _sf, 0, 0.8f);
    }

    _sb->end(vg::SpriteSortMode::BACK_TO_FRONT);

    _sb->renderBatch(f32v2(w->getWidth(), w->getHeight()), &vg::SamplerState::LINEAR_WRAP, &vg::DepthState::NONE, &vg::RasterizerState::CULL_NONE);
    checkGlError("Draw()");
    
}

void LoadScreen::addLoadTask(const nString& name, const cString loadText, ILoadTask* task) {
    // Add the load task to the monitor
    _loadTasks.push_back(task);
    _monitor.addTask(name, _loadTasks.back());

    // Load bar properties
    LoadBarCommonProperties lbcp(f32v2(500, 0), f32v2(500, 60), 800.0f, f32v2(10, 10), 40.0f);
    // Add the new loadbar and get its index
    int i = _loadBars.size();
    _loadBars.emplace_back();

    // Set the properties
    _loadBars[i].setCommonProperties(lbcp);
    _loadBars[i].setStartPosition(f32v2(-lbcp.offsetLength, 30 + i * lbcp.size.y));
    _loadBars[i].expand();
    _loadBars[i].setColor(color::Black, color::Maroon);
    _loadBars[i].setText(loadText);
}
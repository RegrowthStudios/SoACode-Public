#include "stdafx.h"
#include "LoadScreen.h"

#include "App.h"
#include "BlockData.h"
#include "colors.h"
#include "DebugRenderer.h"
#include "FileSystem.h"
#include "FrameBuffer.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Inputs.h"
#include "LoadTaskShaders.h"
#include "LoadTaskGameManager.h"
#include "LoadTaskOptions.h"
#include "LoadTaskBlockData.h"
#include "LoadTaskPlanet.h"
#include "LoadTaskSound.h"
#include "MainMenuScreen.h"
#include "ParticleEmitter.h"
#include "Player.h"
#include "SamplerState.h"

#include "SpriteFont.h"
#include "SpriteBatch.h";
#include "RasterizerState.h"
#include "DepthState.h"

const ColorRGBA8 LOAD_COLOR_TEXT(205, 205, 205, 255);
const ColorRGBA8 LOAD_COLOR_BG_LOADING(105, 5, 5, 255);
const ColorRGBA8 LOAD_COLOR_BG_FINISHED(25, 105, 5, 255);

CTOR_APP_SCREEN_DEF(LoadScreen, App),
_sf(nullptr),
_sb(nullptr),
_monitor() {
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
void LoadScreen::destroy(const GameTime& gameTime) {
    // Empty
}

void LoadScreen::onEntry(const GameTime& gameTime) {

    // Make LoadBar Resources
    _sb = new SpriteBatch(GameManager::glProgramManager, true, true);
    _sf = new SpriteFont("Fonts/orbitron_bold-webfont.ttf", 32);

    // Add Tasks Here
    addLoadTask("GameManager", "Core Systems", new LoadTaskGameManager);
  
    addLoadTask("Sound", "Sound", new LoadTaskSound);
    _monitor.setDep("Sound", "GameManager");

    addLoadTask("Game Options", "Game Options", new LoadTaskOptions);

    addLoadTask("BlockData", "Block Data", new LoadTaskBlockData);
    _monitor.setDep("BlockData", "GameManager");
    _monitor.setDep("BlockData", "Game Options");

    // Start the tasks
    _monitor.start();

    // Clear State For The Screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
}
void LoadScreen::onExit(const GameTime& gameTime) {
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
}

void LoadScreen::onEvent(const SDL_Event& e) {
    // Empty
}
void LoadScreen::update(const GameTime& gameTime) {
    for (ui32 i = 0; i < _loadTasks.size(); i++) {
        if (_loadTasks[i] != nullptr && _loadTasks[i]->isFinished()) {
            // Make The Task Visuals Disappear
            _loadBars[i].setColor(color::Black, color::Teal);
            _loadBars[i].retract();
        }

        // Update Visual Position
        _loadBars[i].update((f32)gameTime.elapsed);
    }

    // Defer shader loading
    static bool loadedShaders = false;
    if (!loadedShaders && _monitor.isTaskFinished("GameManager")) {
        // Do this synchronously for now
        LoadTaskShaders loadTaskShader;
        loadTaskShader.load();
        loadedShaders = true;
    }

    // Defer texture loading
    static bool loadedTextures = false;
    if (!loadedTextures && _monitor.isTaskFinished("BlockData")) {
        LoadTextures();
        //load the texture pack
        fileManager.loadTexturePack("Textures/TexturePacks/" + graphicsOptions.texturePackString);
        SetBlockAvgTexColors();

        //load the emitters
        for (int i = 0; i < 4096; i++) {
            if (Blocks[i].active) {
                if (Blocks[i].emitterName.size()) {
                    Blocks[i].emitter = fileManager.loadEmitter(Blocks[i].emitterName);
                }
                if (Blocks[i].emitterOnBreakName.size()) {
                    Blocks[i].emitterOnBreak = fileManager.loadEmitter(Blocks[i].emitterOnBreakName);
                }
                if (Blocks[i].emitterRandomName.size()) {
                    Blocks[i].emitterRandom = fileManager.loadEmitter(Blocks[i].emitterRandomName);
                }
            }
        }

        //It has no texture
        Blocks[0].pxTex = -1;
        Blocks[0].pyTex = -1;
        Blocks[0].pzTex = -1;
        Blocks[0].nxTex = -1;
        Blocks[0].nyTex = -1;
        Blocks[0].nzTex = -1;

        LoadTaskPlanet loadTaskPlanet;
        loadTaskPlanet.load();

        openglManager.InitializeFrameBuffer();

        _state = ScreenState::CHANGE_NEXT;
        loadedTextures = true;
    }
}
void LoadScreen::draw(const GameTime& gameTime) {
    const GameWindow* w = &_game->getWindow();

    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _sb->begin();
    for (ui32 i = 0; i < _loadTasks.size(); i++) {
        _loadBars[i].draw(_sb, _sf, 0, 0.8f);
    }

    _sb->end(SpriteSortMode::BACK_TO_FRONT);

    _sb->renderBatch(f32v2(w->getWidth(), w->getHeight()), &SamplerState::LINEAR_WRAP, &DepthState::NONE, &RasterizerState::CULL_NONE);
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
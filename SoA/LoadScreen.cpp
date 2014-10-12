#include "stdafx.h"
#include "LoadScreen.h"

#include "BlockData.h"
#include "DebugRenderer.h"
#include "FileSystem.h"
#include "FrameBuffer.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Inputs.h"
#include "LoadBar.h"
#include "LoadTaskGameManager.h"
#include "LoadTaskInput.h"
#include "Player.h"
#include "SamplerState.h"
#include "shader.h"
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
}

i32 LoadScreen::getNextScreen() const {
    return -1;
}
i32 LoadScreen::getPreviousScreen() const {
    return -1;
}

void LoadScreen::build() {
}
void LoadScreen::destroy(const GameTime& gameTime) {
}

void LoadScreen::onEntry(const GameTime& gameTime) {
    // Make LoadBar Resources
    glGenTextures(1, &_texID);
    glBindTexture(GL_TEXTURE_2D, _texID);
    ui32 pix[4] = { 0xffffffff, 0xff0000ff, 0xff0000ff, 0xffffffff };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);
    glBindTexture(GL_TEXTURE_2D, 0);
    _sb = new SpriteBatch(false);
    _sb->init();
    _sf = new SpriteFont("Fonts/orbitron_bold-webfont.ttf", 32);


    // Add Tasks Here
    _loadTasks.push_back(new LoadTaskGameManager);
    _monitor.addTask("GameManager", _loadTasks.back());

    _loadTasks.push_back(new LoadTaskInput);
    _monitor.addTask("InputManager", _loadTasks.back());
    _monitor.setDep("InputManager", "GameManager");

    _monitor.start();

    // Make LoadBars
    LoadBarCommonProperties lbcp(f32v2(500, 0), f32v2(500, 60), 800.0f, f32v2(10, 10), 40.0f);
    _loadBars = new LoadBar[_loadTasks.size()];
    for (i32 i = 0; i < _loadTasks.size(); i++) {
        _loadBars[i].setCommonProperties(lbcp);
        _loadBars[i].setStartPosition(f32v2(-lbcp.offsetLength, 30 + i * lbcp.size.y));
        _loadBars[i].expand();
        _loadBars[i].setColor(color::Black, color::Maroon);
    }

    // Put Text For The Load Bars
    {
        i32 i = 0;
        _loadBars[i++].setText("Game Manager");
        _loadBars[i++].setText("Input Manager");
    }


    // Clear State For The Screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
}
void LoadScreen::onExit(const GameTime& gameTime) {
    // Delete LoadBar Resources
    _sf->dispose();
    delete _sf;
    _sf = nullptr;

    _sb->dispose();
    delete _sb;
    _sb = nullptr;

    glDeleteTextures(1, &_texID);
    _texID = 0;

    delete[] _loadBars;
    _loadBars = nullptr;
}

void LoadScreen::onEvent(const SDL_Event& e) {
    switch (e.type) {
    case SDL_KEYDOWN:
        switch (e.key.keysym.sym) {
        case SDLK_a:
            for (i32 i = 0; i < 4; i++) {
                _loadBars[i].retract();
            }
            break;
        case SDLK_d:
            for (i32 i = 0; i < 4; i++) {
                _loadBars[i].expand();
            }
            break;
        default:
            break;
        }
    default:
        break;
    }
}
void LoadScreen::update(const GameTime& gameTime) {
    for (i32 i = 0; i < _loadTasks.size(); i++) {
        if (_loadTasks[i] != nullptr && _loadTasks[i]->isFinished()) {
            _loadBars[i].setColor(color::Black, color::Teal);
            _loadBars[i].retract();
            _loadTasks[i] = nullptr;
        }
        _loadBars[i].update((f32)gameTime.elapsed);
    }
}
void LoadScreen::draw(const GameTime& gameTime) {
    GameDisplayMode gdm;
    _game->getDisplayMode(&gdm);

    glViewport(0, 0, gdm.screenWidth, gdm.screenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _sb->begin();
    for (i32 i = 0; i < _loadTasks.size(); i++) {
        _loadBars[i].draw(_sb, _sf, 0, 0.8f);
    }
    _sb->end(SpriteSortMode::BACK_TO_FRONT);

    _sb->renderBatch(f32v2(gdm.screenWidth, gdm.screenHeight), &SamplerState::LINEAR_WRAP, &DepthState::NONE, &RasterizerState::CULL_NONE);
}
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
#include "Player.h"
#include "SamplerState.h"
#include "shader.h"
#include "SpriteFont.h"
#include "SpriteBatch.h";
#include "RasterizerState.h"
#include "DepthState.h"

const color8 LOAD_COLOR_TEXT(205, 205, 205, 255);
const color8 LOAD_COLOR_BG_LOADING(105, 5, 5, 255);
const color8 LOAD_COLOR_BG_FINISHED(25, 105, 5, 255);

LoadScreen::LoadScreen() : IGameScreen(),
rand(Random()),
_sf(nullptr),
_sb(nullptr),
_ffm(std::map<nString, nString>()),
_frameBuffer(nullptr) {}

i32 LoadScreen::getNextScreen() const {
    return -1;
}
i32 LoadScreen::getPreviousScreen() const {
    return -1;
}

void LoadScreen::build() {}
void LoadScreen::destroy(const GameTime& gameTime) {}

void LoadScreen::onEntry(const GameTime& gameTime) {
    // TODO: Put This In A Loading Thread
    initializeOld();

    // Make LoadBar Resources
    glGenTextures(1, &_texID);
    glBindTexture(GL_TEXTURE_2D, _texID);
    ui32 pix[4] = { 0xffffffff, 0xff0000ff, 0xff0000ff, 0xffffffff };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);
    glBindTexture(GL_TEXTURE_2D, 0);
    _sb = new SpriteBatch(false);
    _sb->init();
    _sf = new SpriteFont("Fonts/orbitron_bold-webfont.ttf", 32);

    // Make LoadBars
    LoadBarCommonProperties lbcp(f32v2(500, 0), f32v2(500, 60), 800.0f, f32v2(10, 10), 40.0f);
    _loadBars = new LoadBar[4];
    for (i32 i = 0; i < 4; i++) {
        _loadBars[i].setCommonProperties(lbcp);
        _loadBars[i].setStartPosition(f32v2(-lbcp.offsetLength, 30 + i * lbcp.size.y));
    }

#define LOAD_FUNC_START(NUM, TEXT) \
    scr->_loadBars[NUM].setText(TEXT); \
    scr->_loadBars[NUM].setColor(LOAD_COLOR_TEXT, LOAD_COLOR_BG_LOADING); \
    scr->_loadBars[NUM].expand(); \
    while (scr->_loadBars[NUM].getIsExpanding()) SDL_Delay(100)
#define LOAD_FUNC_SHARED_GL_CONTEXT \
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1); \
    SDL_GL_CreateContext(scr->_game->getWindowHandle())
#define LOAD_FUNC_END(NUM) \
    scr->_loadBars[NUM].setColor(LOAD_COLOR_TEXT, LOAD_COLOR_BG_FINISHED); \
    scr->_loadBars[NUM].retract(); \
    return 0

    _loadThreads[0].setFunction([] (LoadScreen* scr) {
        LOAD_FUNC_START(0, "Creating FrameBuffer");
        LOAD_FUNC_SHARED_GL_CONTEXT;
        scr->createFrameBuffer();
        LOAD_FUNC_END(0);
    });
    _loadThreads[1].setFunction([] (LoadScreen* scr) {
        LOAD_FUNC_START(1, "Loading Shaders");
        LOAD_FUNC_SHARED_GL_CONTEXT;
        scr->loadShaders();
        LOAD_FUNC_END(1);
    });
    _loadThreads[2].setFunction([] (LoadScreen* scr) {
        LOAD_FUNC_START(2, "Loading Inputs");
        LOAD_FUNC_SHARED_GL_CONTEXT;
        scr->loadInputs();
        LOAD_FUNC_END(2);
    });
    _loadThreads[3].setFunction([] (LoadScreen* scr) {
        LOAD_FUNC_START(3, "Loading Block Data");
        LOAD_FUNC_SHARED_GL_CONTEXT;
        while (!scr->_loadThreads[0].getIsFinished() ||
            !scr->_loadThreads[0].getIsFinished() ||
            !scr->_loadThreads[0].getIsFinished()) {
            SDL_Delay(100);
        }
        scr->loadBlockData();
        LOAD_FUNC_END(3);
    });

    for (i32 i = 0; i < 4; i++) {
        _loadThreads[i].start(this);
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
    for (i32 i = 0; i < 4; i++) {
        _loadBars[i].update((f32)gameTime.elapsed);
    }
}
void LoadScreen::draw(const GameTime& gameTime) {
    GameDisplayMode gdm;
    _game->getDisplayMode(&gdm);

    glViewport(0, 0, gdm.screenWidth, gdm.screenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _sb->begin();
    for (i32 i = 0; i < 4; i++) {
        _loadBars[i].draw(_sb, _sf, _texID, 0.8f);
    }
    //_sb->draw(_texID, f32v2(0), f32v2(100), color8(255, 255, 255, 255));
    //_sb->drawString(_sf, "Hello There", f32v2(10, 10), 30, 1, color8(255, 0, 0, 255));
    _sb->end(SpriteSortMode::BACK_TO_FRONT);

    _sb->renderBatch(f32v2(gdm.screenWidth, gdm.screenHeight), &SamplerState::LINEAR_WRAP, &DepthState::NONE, &RasterizerState::CULL_NONE);
}

void LoadScreen::checkSystemRequirements() {
    const GraphicsDeviceProperties gdProps = GraphicsDevice::getCurrent()->getProperties();
    if (gdProps.glVersionMajor < 3) {
        char buffer[2048];
        sprintf(buffer, "Your graphics card driver does not support at least OpenGL 3.3. Your OpenGL version is \"%s\". The game will most likely not work.\n\nEither your graphics card drivers are not up to date, or your computer is using an integrated graphics card instead of your gaming card.\nYou should be able to switch to your main graphics card by right clicking your desktop and going to graphics properties.", gdProps.glVersion);
        pError(buffer);
    }
    if (!GLEW_VERSION_2_1) {
        pError("Machine does not support 2.1 GLEW API.");
        _state = ScreenState::EXIT_APPLICATION;
    }
    if (gdProps.maxTextureUnits < 8) {
        showMessage("Your graphics card does not support at least 8 texture units! It only supports " + std::to_string(gdProps.maxTextureUnits));
        _state = ScreenState::EXIT_APPLICATION;
    }

#ifdef DEBUG
    printf("System Met Minimum Requirements\n");
#endif // DEBUG
}
void LoadScreen::initializeOld() {
    initializeOptions();
    GameManager::inputManager = new InputManager();
    GameManager::inputManager->loadAxes();

    player = new Player();
    player->isFlying = 0;
    player->setMoveSpeed(0.0095f, 0.166f); //Average Human Running Speed = .12, 17 is a good speed

    loadOptions();

    GameManager::initializeSystems();
}

void LoadScreen::createFrameBuffer() {
    const GraphicsDeviceProperties gdProps = GraphicsDevice::getCurrent()->getProperties();
    i32 msaa = max(gdProps.maxColorSamples, gdProps.maxDepthSamples);

    GameDisplayMode gdm;
    _game->getDisplayMode(&gdm);

    if (msaa > 0) {
        glEnable(GL_MULTISAMPLE);
        _frameBuffer = new FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT, gdm.screenWidth, gdm.screenHeight, msaa);
    } else {
        glDisable(GL_MULTISAMPLE);
        _frameBuffer = new FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT, gdm.screenWidth, gdm.screenHeight);
    }
}

void LoadScreen::changeFont() {
    if (_sf) {
        _sf->dispose();
        delete _sf;
    }

    auto ffmIter = _ffm.begin();
    i32 fi = (i32)(rand.genMT() * _ffm.size());
    for (i32 i = 0; i < fi; i++) ffmIter++;
    printf("Using Font: %s\n", ffmIter->first.c_str());
    _sf = new SpriteFont(ffmIter->second.c_str(), 64);
}

void LoadScreen::loadShaders() {
    textureShader.DeleteShader();
    textureShader.Initialize();
    basicColorShader.DeleteShader();
    basicColorShader.Initialize();
    simplexNoiseShader.DeleteShader();
    //simplexNoiseShader.Initialize();
    hdrShader.DeleteShader();
    motionBlurShader.DeleteShader();
    motionBlurShader.Initialize();
    hdrShader.Initialize();
    blockShader.DeleteShader();
    blockShader.Initialize();
    cutoutShader.Initialize();
    transparentShading.Initialize();
    atmosphereToSkyShader.DeleteShader();
    atmosphereToSkyShader.Initialize();
    atmosphereToGroundShader.DeleteShader();
    atmosphereToGroundShader.Initialize();
    spaceToSkyShader.DeleteShader();
    spaceToSkyShader.Initialize();
    spaceToGroundShader.DeleteShader();
    spaceToGroundShader.Initialize();
    waterShader.DeleteShader();
    waterShader.Initialize();
    billboardShader.DeleteShader();
    billboardShader.Initialize();
    fixedSizeBillboardShader.DeleteShader();
    fixedSizeBillboardShader.Initialize();
    sonarShader.DeleteShader();
    sonarShader.Initialize();
    physicsBlockShader.DeleteShader();
    physicsBlockShader.Initialize();
    texture2Dshader.DeleteShader();
    texture2Dshader.Initialize();
    treeShader.DeleteShader();
    treeShader.Initialize();
}
void LoadScreen::loadInputs() {
    initInputs();
}
void LoadScreen::loadBlockData() {
    initConnectedTextures();
    if (!(fileManager.loadBlocks("Data/BlockData.ini"))) exit(123432);
    fileManager.saveBlocks("Data/test.ini");
}


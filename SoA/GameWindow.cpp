#include "stdafx.h"
#include "GameWindow.h"

#include "IOManager.h"

KEG_ENUM_INIT_BEGIN(GameSwapInterval, GameSwapInterval, etype)
using namespace Keg;
etype->addValue("Unlimited", GameSwapInterval::UNLIMITED_FPS);
etype->addValue("VSync", GameSwapInterval::V_SYNC);
etype->addValue("LowSync", GameSwapInterval::LOW_SYNC);
etype->addValue("PowerSaver", GameSwapInterval::POWER_SAVER);
etype->addValue("ValueCap", GameSwapInterval::USE_VALUE_CAP);
KEG_ENUM_INIT_END

KEG_TYPE_INIT_BEGIN(GameDisplayMode, GameDisplayMode, type)
using namespace Keg;
type->addValue("ScreenWidth", Value::basic(BasicType::I32, offsetof(GameDisplayMode, screenWidth)));
type->addValue("ScreenHeight", Value::basic(BasicType::I32, offsetof(GameDisplayMode, screenHeight)));
type->addValue("IsFullscreen", Value::basic(BasicType::BOOL, offsetof(GameDisplayMode, isFullscreen)));
type->addValue("IsBorderless", Value::basic(BasicType::BOOL, offsetof(GameDisplayMode, isBorderless)));
type->addValue("SwapInterval", Value::custom("GameSwapInterval", offsetof(GameDisplayMode, swapInterval), true));
type->addValue("MaxFPS", Value::basic(BasicType::F32, offsetof(GameDisplayMode, maxFPS)));
KEG_TYPE_INIT_END

// For Comparing Display Modes When Saving Data
static bool operator==(const GameDisplayMode& m1, const GameDisplayMode& m2) {
    return
        m1.screenWidth == m2.screenWidth &&
        m1.screenHeight == m2.screenHeight &&
        m1.isFullscreen == m2.isFullscreen &&
        m1.isBorderless == m2.isBorderless &&
        m1.swapInterval == m2.swapInterval &&
        m1.maxFPS == m2.maxFPS
        ;
}
static bool operator!=(const GameDisplayMode& m1, const GameDisplayMode& m2) {
    return !(m1 == m2);
}

GameWindow::GameWindow() :
_window(nullptr),
_glc(nullptr) {
    setDefaultSettings(&_displayMode);
}

bool GameWindow::init() {
    // Attempt To Read Custom Settings
    readSettings();

    // Create The Window
    _window = SDL_CreateWindow(DEFAULT_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _displayMode.screenWidth, _displayMode.screenHeight, DEFAULT_WINDOW_FLAGS);
    if (_window == nullptr) {
        printf("Window Creation Failed\r\n");
        return false;
    }

    // Initialize OpenGL
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#ifdef GL_CORE
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    _glc = SDL_GL_CreateContext(_window);
    if (_glc == nullptr) {
        printf("Could Not Create OpenGL Context");
        return false;
    }
    SDL_GL_MakeCurrent(_window, _glc);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        printf("Glew failed to initialize. Your graphics card is probably WAY too old. Or you forgot to extract the .zip. It might be time for an upgrade :)");
        return false;
    }

    // Set More Display Settings
    setFullscreen(_displayMode.isFullscreen, true);
    setBorderless(_displayMode.isBorderless, true);
    setSwapInterval(_displayMode.swapInterval, true);

    // Make sure default clear depth is 1.0f
    glClearDepth(1.0f);

    return true;
}
void GameWindow::dispose() {
    saveSettings();

    if (_glc) {
        SDL_GL_DeleteContext(_glc);
        _glc = nullptr;
    }
    if (_window) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
}

void GameWindow::setDefaultSettings(GameDisplayMode* mode) {
    mode->screenWidth = DEFAULT_WINDOW_WIDTH;
    mode->screenHeight = DEFAULT_WINDOW_HEIGHT;
    mode->isBorderless = true;
    mode->isFullscreen = false;
    mode->maxFPS = DEFAULT_MAX_FPS;
    mode->swapInterval = DEFAULT_SWAP_INTERVAL;
}

void GameWindow::readSettings() {
    IOManager iom;
    nString data;
    iom.readFileToString(DEFAULT_APP_CONFIG_FILE, data);
    if (data.length()) {
        Keg::parse(&_displayMode, data.c_str(), "GameDisplayMode");
    } else {
        // If there is no app.config, save a default one.
        saveSettings();
    }
}

void GameWindow::saveSettings() const {
    GameDisplayMode modeBasis = {};
    setDefaultSettings(&modeBasis);

    if (_displayMode != modeBasis) {
        nString data = Keg::write(&_displayMode, "GameDisplayMode", nullptr);
        std::ofstream file(DEFAULT_APP_CONFIG_FILE);
        file << data << std::endl;
        file.flush();
        file.close();
    }
}

void GameWindow::setScreenSize(const i32& w, const i32& h, const bool& overrideCheck /*= false*/) {
    // Apply A Minimal State Change
    if (overrideCheck || _displayMode.screenWidth != w || _displayMode.screenHeight != h) {
        _displayMode.screenWidth = w;
        _displayMode.screenHeight = h;
        SDL_SetWindowSize(_window, _displayMode.screenWidth, _displayMode.screenHeight);
    }
}
void GameWindow::setFullscreen(const bool& useFullscreen, const bool& overrideCheck /*= false*/) {
    if (overrideCheck || _displayMode.isFullscreen != useFullscreen) {
        _displayMode.isFullscreen = useFullscreen;
        SDL_SetWindowFullscreen(_window, _displayMode.isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }
}
void GameWindow::setBorderless(const bool& useBorderless, const bool& overrideCheck /*= false*/) {
    if (overrideCheck || _displayMode.isBorderless != useBorderless) {
        _displayMode.isBorderless = useBorderless;
        SDL_SetWindowBordered(_window, _displayMode.isBorderless ? SDL_FALSE : SDL_TRUE);
    }
}
void GameWindow::setSwapInterval(const GameSwapInterval& mode, const bool& overrideCheck /*= false*/) {
    if (overrideCheck || _displayMode.swapInterval != mode) {
        _displayMode.swapInterval = mode;
        switch (_displayMode.swapInterval) {
        case GameSwapInterval::UNLIMITED_FPS:
        case GameSwapInterval::USE_VALUE_CAP:
            SDL_GL_SetSwapInterval(0);
            break;
        default:
            SDL_GL_SetSwapInterval(static_cast<i32>(DEFAULT_SWAP_INTERVAL));
            break;
        }
    }
}
void GameWindow::setMaxFPS(const f32& fpsLimit) {
    _displayMode.maxFPS = fpsLimit;
}
void GameWindow::setTitle(const cString title) const {
    if (title) SDL_SetWindowTitle(_window, title);
    else SDL_SetWindowTitle(_window, DEFAULT_TITLE);
}

void GameWindow::sync(ui32 frameTime) {
    SDL_GL_SwapWindow(_window);

    // Limit FPS
    if (_displayMode.swapInterval == GameSwapInterval::USE_VALUE_CAP) {
        f32 desiredFPS = 1000.0f / (f32)_displayMode.maxFPS;
        ui32 sleepTime = (ui32)(desiredFPS - frameTime);
        if (desiredFPS > frameTime && sleepTime > 0) SDL_Delay(sleepTime);
    }
}

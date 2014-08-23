#include "stdafx.h"
#include "MainGame.h"

#include <thread>

#include <TTF\SDL_ttf.h>

#include "DepthState.h"
#include "FrameBuffer.h"
#include "IGameScreen.h"
#include "GraphicsDevice.h"
#include "RasterizerState.h"
#include "SamplerState.h"
#include "ScreenList.h"
#include "utils.h"

MainGame::MainGame() {
    _displayMode = {};
}
MainGame::~MainGame() {}

void MainGame::getDisplayMode(GameDisplayMode* displayMode) {
    // Get Display Info From SDL
    SDL_GetWindowSize(_window, &displayMode->screenWidth, &displayMode->screenHeight);
    ui32 flags = SDL_GetWindowFlags(_window);
    displayMode->isBorderless = flags & SDL_WINDOW_BORDERLESS;
    displayMode->isFullscreen = flags & SDL_WINDOW_FULLSCREEN_DESKTOP;
    displayMode->swapInterval = static_cast<GameSwapInterval>(SDL_GL_GetSwapInterval());

    // Also Update Our Display Mode
    _displayMode = *displayMode;
}
void MainGame::setDisplayMode(const GameDisplayMode& displayMode) {
    // Apply A Minimal State Change
    if (_displayMode.screenWidth != displayMode.screenWidth || _displayMode.screenHeight != displayMode.screenHeight) {
        _displayMode.screenWidth = displayMode.screenWidth;
        _displayMode.screenHeight = displayMode.screenHeight;
        SDL_SetWindowSize(_window, _displayMode.screenWidth, _displayMode.screenHeight);
    }
    if (_displayMode.isBorderless != displayMode.isBorderless) {
        _displayMode.isBorderless = displayMode.isBorderless;
        SDL_SetWindowBordered(_window, _displayMode.isBorderless ? SDL_FALSE : SDL_TRUE);
    }
    if (_displayMode.isFullscreen != displayMode.isFullscreen) {
        _displayMode.isFullscreen = displayMode.isFullscreen;
        SDL_SetWindowFullscreen(_window, _displayMode.isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }
    if (_displayMode.swapInterval != displayMode.swapInterval) {
        _displayMode.swapInterval = displayMode.swapInterval;
        SDL_GL_SetSwapInterval(static_cast<i32>(displayMode.swapInterval));
    }
}

void MainGame::setWindowTitle(const cString title) {
    if (title) SDL_SetWindowTitle(_window, title);
    else SDL_SetWindowTitle(_window, DEFAULT_TITLE);
}

void MainGame::initSystems() {
    // Create The Window
    _window = SDL_CreateWindow(DEFAULT_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_FLAGS);
    if (_window == NULL) exit(343);

    // Initialize OpenGL
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#ifdef GL_CORE
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    _glc = SDL_GL_CreateContext(_window);
    if (_glc == nullptr) {
        pError("Could not make openGL context!");
        SDL_Quit();
    }
    SDL_GL_MakeCurrent(_window, _glc);
    SDL_GL_SetSwapInterval(static_cast<i32>(DEFAULT_SWAP_INTERVAL));
#if defined _WIN32 || defined _WIN64
    _hndGLRC = wglGetCurrentContext();
    if (_hndGLRC == nullptr) {
        pError("Call to wglGetCurrentContext failed in main thread!");
    }
#endif

    
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        pError("Glew failed to initialize. Your graphics card is probably WAY too old. Or you forgot to extract the .zip. It might be time for an upgrade :)");
        exit(133);
    }

    // Get The Machine's Graphics Capabilities
    _gDevice = new GraphicsDevice;
    _gDevice->refreshInformation();

    // Set A Default OpenGL State
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    // TODO: Replace With BlendState
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    DepthState::FULL.set();
    RasterizerState::CULL_COUNTER_CLOCKWISE.set();
    SamplerState::initPredefined();

    // Initialize Frame Buffer
    getDisplayMode(&_displayMode);
    glViewport(0, 0, _displayMode.screenWidth, _displayMode.screenHeight);

    // Initialize Fonts Library
    if (TTF_Init() == -1) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        pError("TTF COULD NOT INIT!");
        exit(331);
    }
}

void MainGame::run() {
    SDL_Init(SDL_INIT_EVERYTHING);
    init();

    _isRunning = true;
    while (_isRunning) {
        // Refresh Time Information
        refreshElapsedTime();

        // Main Game Logic
        checkInput();
        if (!_isRunning) break;
        onUpdateFrame();
        onRenderFrame();

        // Draw Rendered Objects
        SDL_GL_SwapWindow(_window);

        // Limit FPS
        if (graphicsOptions.maxFPS < 165) {
            f32 desiredFPS = 1000.0f / (f32)graphicsOptions.maxFPS;
            ui32 diff = SDL_GetTicks() - _lastMS;
            if (desiredFPS > diff) Sleep((ui32)(desiredFPS - diff));
        }
    }

    SDL_Quit();
}
void MainGame::exitGame() {
    if (_screen) {
        _screen->onExit(_lastTime);
    }
    if (_screenList) {
        _screenList->destroy(_lastTime);
    }
    _isRunning = false;
}

void MainGame::init() {
    // This Is Vital
    initSystems();
    setWindowTitle(nullptr);

    // Initialize Logic And Screens
    _screenList = new ScreenList(this);
    onInit();
    addScreens();

    // Try To Get A Screen
    _screen = _screenList->getCurrent();
    if (_screen == nullptr) {
        exitGame();
        return;
    }

    // Run The First Game Screen
    _screen->setRunning();
    _lastTime = {};
    _curTime = {};
    _screen->onEntry(_lastTime);
    _lastMS = SDL_GetTicks();
}
void MainGame::refreshElapsedTime() {
    ui32 ct = SDL_GetTicks();
    f64 et = (ct - _lastMS) / 1000.0;
    _lastMS = ct;

    _lastTime = _curTime;
    _curTime.elapsed = et;
    _curTime.total += et;
}
void MainGame::checkInput() {
    SDL_Event e;
    if (_screen) {
        while (SDL_PollEvent(&e) != 0) {
            _screen->onEvent(e);
            switch (e.type) {
            case SDL_QUIT:
                exitGame();
                break;
            }
        }
    } else {
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
            case SDL_QUIT:
                exitGame();
                break;
            }
        }
    }
}
void MainGame::onUpdateFrame() {
    if (_screen != nullptr) {
        switch (_screen->getState()) {
        case ScreenState::RUNNING:
            _screen->update(_curTime);
            break;
        case ScreenState::CHANGE_NEXT:
            _screen->onExit(_curTime);
            _screen = _screenList->moveNext();
            if (_screen != nullptr) {
                _screen->setRunning();
                _screen->onEntry(_curTime);
            }
            break;
        case ScreenState::CHANGE_PREVIOUS:
            _screen->onExit(_curTime);
            _screen = _screenList->movePrevious();
            if (_screen != nullptr) {
                _screen->setRunning();
                _screen->onEntry(_curTime);
            }
            break;
        case ScreenState::EXIT_APPLICATION:
            exitGame();
            return;
        }
    } else {
        exitGame();
        return;
    }
}
void MainGame::onRenderFrame() {
    glViewport(0, 0, _displayMode.screenWidth, _displayMode.screenHeight);
    if (_screen != nullptr && _screen->getState() == ScreenState::RUNNING) {
        _screen->draw(_curTime);
    }
}

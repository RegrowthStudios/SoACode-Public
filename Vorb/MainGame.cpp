#include "stdafx.h"
#include "MainGame.h"

#include <thread>

#include <TTF\SDL_ttf.h>

#include "DepthState.h"
#include "IGameScreen.h"
#include "GraphicsDevice.h"
#include "RasterizerState.h"
#include "SamplerState.h"
#include "ScreenList.h"
#include "utils.h"
#include "Timing.h"


MainGame::MainGame() : _fps(0) {
    // Empty
}
MainGame::~MainGame() {
    // Empty
}

bool MainGame::initSystems() {
    // Create The Window
    if (!_window.init()) return false;

    // Get The Machine's Graphics Capabilities
    _gDevice = new GraphicsDevice(_window);
    _gDevice->refreshInformation();

    // Set A Default OpenGL State
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    // TODO: Replace With BlendState
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    DepthState::FULL.set();
    RasterizerState::CULL_CLOCKWISE.set();
    SamplerState::initPredefined();

    // Initialize Frame Buffer
    glViewport(0, 0, _window.getWidth(), _window.getHeight());

    // Initialize Fonts Library
    if (TTF_Init() == -1) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void MainGame::run() {
  
    // Initialize everything except SDL audio and SDL haptic feedback.
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK);

    // Make sure we are using hardware acceleration
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // For counting the fps
    FpsCounter fpsCounter;

    // Game Loop
    if (init()) {
        _isRunning = true;
        while (_isRunning) {
            // Start the FPS counter
            fpsCounter.beginFrame();
            // Refresh Time Information
            refreshElapsedTime();

            // Main Game Logic
            checkInput();
            if (!_isRunning) break;
            onUpdateFrame();
            onRenderFrame();

            _window.sync(SDL_GetTicks() - _lastMS);
            // Get the FPS
            _fps = fpsCounter.endFrame();
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
    _window.dispose();
    _isRunning = false;
}

bool MainGame::init() {
    // This Is Vital
    if (!initSystems()) return false;
    _window.setTitle(nullptr);

    // Initialize Logic And Screens
    _screenList = new ScreenList(this);
    onInit();
    addScreens();

    // Try To Get A Screen
    _screen = _screenList->getCurrent();
    if (_screen == nullptr) {
        exitGame();
        return false;
    }

    // Run The First Game Screen
    _screen->setRunning();
    _lastTime = {};
    _curTime = {};
    _screen->onEntry(_lastTime);
    _lastMS = SDL_GetTicks();

    return true;
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
    // TODO: Investigate Removing This
    glViewport(0, 0, _window.getWidth(), _window.getHeight());
    if (_screen != nullptr && _screen->getState() == ScreenState::RUNNING) {
        _screen->draw(_curTime);
    }
}

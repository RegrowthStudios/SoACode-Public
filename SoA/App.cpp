#include "stdafx.h"
#include "App.h"

#include <InputDispatcher.h>

#include "InitScreen.h"
#include "LoadScreen.h"
#include "MainMenuScreen.h"
#include "GamePlayScreen.h"
#include "ScreenList.h"
#include "SpriteBatch.h"
#include "MeshManager.h"
#include "Options.h"

void App::addScreens() {
    scrInit = new InitScreen(this);
    scrLoad = new LoadScreen(this);
    scrMainMenu = new MainMenuScreen(this);
    scrGamePlay = new GamePlayScreen(this);

    _screenList->addScreen(scrInit);
    _screenList->addScreen(scrLoad);
    _screenList->addScreen(scrMainMenu);
    _screenList->addScreen(scrGamePlay);

    _screenList->setScreen(scrInit->getIndex());
}

void App::onInit() {
    
    // Load the graphical options
    initializeOptions();
    loadOptions();

    SamplerState::initPredefined();

    // Allocate resources
    meshManager = new MeshManager;

    vui::InputDispatcher::key.onEvent += createDelegate<const vui::KeyEvent&>([=] (void* sender, const vui::KeyEvent& e) {
        std::cout << "Received keyboard event" << std::endl;
    });
    vui::InputDispatcher::mouse.onEvent += createDelegate<const vui::MouseEvent&>([=] (void* sender, const vui::MouseEvent& e) {
        std::cout << "Received mouse event" << std::endl;
    });
    vui::InputDispatcher::mouse.onButtonDown += createDelegate<const vui::MouseButtonEvent&>([=] (void* sender, const vui::MouseButtonEvent& e) {
        if (e.clicks > 1) std::cout << "Received double-click mouse event" << std::endl;
    });
    vui::InputDispatcher::window.onFile += createDelegate<const vui::WindowFileEvent&>([=] (void* sender, const vui::WindowFileEvent& e) {
        std::cout << "Received file: " << e.file << std::endl;
    });
    vui::InputDispatcher::window.onResize += createDelegate<const vui::WindowResizeEvent&>([=] (void* sender, const vui::WindowResizeEvent& e) {
        std::cout << "Window resized: " << e.w << " x " << e.h <<std::endl;
    });
}

void App::onExit() {
    // Delete cache if it exists

    SpriteBatch::disposeProgram();
}

App::~App() {
    if (scrInit) {
        delete scrInit;
        scrInit = nullptr;
    }

    if (scrLoad) {
        delete scrLoad;
        scrLoad = nullptr;
    }

    delete meshManager;
}
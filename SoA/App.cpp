#include "stdafx.h"
#include "App.h"

#include <InputDispatcher.h>
#include <ScreenList.h>
#include <SpriteBatch.h>

#include "DevScreen.h"
#include "InitScreen.h"
#include "LoadScreen.h"
#include "MainMenuScreen.h"
#include "GamePlayScreen.h"
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

    // Add development screen
    scrDev = new DevScreen;
    scrDev->addScreen(SDLK_RETURN, scrInit);
    scrDev->addScreen(SDLK_SPACE, scrInit);
    _screenList->addScreen(scrDev);

    _screenList->setScreen(scrDev->getIndex());
}

void App::onInit() {
    
    // Load the graphical options
    initializeOptions();
    loadOptions();

    SamplerState::initPredefined();

    // Allocate resources
    meshManager = new MeshManager;

    // Showcase various information
    vui::InputDispatcher::window.onFile += createDelegate<const vui::WindowFileEvent&>([=] (void* sender, const vui::WindowFileEvent& e) {
        std::cout << "Received file: " << e.file << std::endl;
    });
    vui::InputDispatcher::window.onResize += createDelegate<const vui::WindowResizeEvent&>([=] (void* sender, const vui::WindowResizeEvent& e) {
        std::cout << "Window resized: " << e.w << " x " << e.h <<std::endl;
    });
    vui::InputDispatcher::window.onClose += createDelegate<>([=] (void* sender) {
        std::cout << "Window requested close" << std::endl;
    });
    vui::InputDispatcher::onQuit += createDelegate<>([=] (void* sender) {
        std::cout << "App requested quit" << std::endl;
    });
    vui::InputDispatcher::key.onFocusGained += createDelegate<>([=] (void* sender) {
        std::cout << "Keyboard gained focus" << std::endl;
    });
    vui::InputDispatcher::key.onFocusLost += createDelegate<>([=] (void* sender) {
        std::cout << "Keyboard lost focus" << std::endl;
    });
    vui::InputDispatcher::mouse.onFocusGained += createDelegate<const vui::MouseEvent&>([=] (void* sender, const vui::MouseEvent& e) {
        std::cout << "Mouse gained focus at:" << e.x << "," << e.y << std::endl;
    });
    vui::InputDispatcher::mouse.onFocusLost += createDelegate<const vui::MouseEvent&>([=] (void* sender, const vui::MouseEvent& e) {
        std::cout << "Mouse lost focus at:" << e.x << "," << e.y << std::endl;
    });
    vui::InputDispatcher::key.onKeyDown += createDelegate<const vui::KeyEvent&>([=](void* sender, const vui::KeyEvent& e) {
        std::cout << "Key Event: RC=" << e.repeatCount << " Num:" << e.mod.num << std::endl;
    });
}

void App::onExit() {
    // Delete cache if it exists

    SpriteBatch::disposeProgram();
}

App::~App() {
#define COND_DEL(SCR) if (SCR) { delete SCR; SCR = nullptr; }

    COND_DEL(scrInit)
    COND_DEL(scrLoad)
    // TODO: Why do these break
    //COND_DEL(scrMainMenu)
    //COND_DEL(scrGamePlay)
    COND_DEL(scrDev)

    delete meshManager;
}
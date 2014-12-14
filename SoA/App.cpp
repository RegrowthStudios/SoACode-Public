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
#include "TestConsoleScreen.h"

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

    // Add test screens
    scrTests.push_back(new TestConsoleScreen);
    _screenList->addScreen(scrTests.back());
    scrDev->addScreen(SDLK_c, scrTests.back());

    // Start from dev screen for convenience
    _screenList->setScreen(scrDev->getIndex());
}

void App::onInit() {
    
    // Load the graphical options
    initializeOptions();
    loadOptions();

    SamplerState::initPredefined();

    // Allocate resources
    meshManager = new MeshManager;
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
#include "stdafx.h"
#include "App.h"

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
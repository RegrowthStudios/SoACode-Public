#include "stdafx.h"
#include "App.h"

#include "GamePlayScreen.h"
#include "InitScreen.h"
#include "LoadScreen.h"
#include "MainMenuScreen.h"
#include "MeshManager.h"
#include "Options.h"
#include "ScreenList.h"
#include "SpaceSystem.h"
#include "SpriteBatch.h"

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
    loadOptions("Data/Options.yml");

    SamplerState::initPredefined();

    // Allocate resources
    spaceSystem = new SpaceSystem;
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
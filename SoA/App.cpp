#include "stdafx.h"
#include "App.h"

#include "GamePlayScreen.h"

#include <Vorb/InputDispatcher.h>
#include <Vorb/ScreenList.h>
#include <Vorb/SpriteBatch.h>
#include <Vorb/InputDispatcher.h>
#include <Vorb/ScreenList.h>
#include <Vorb/SpriteBatch.h>

#include "DevScreen.h"
#include "GameManager.h"
#include "GamePlayScreen.h"
#include "InitScreen.h"
#include "LoadScreen.h"
#include "MainMenuScreen.h"
#include "MeshManager.h"
#include "Options.h"
#include "InputManager.h"

#include "SpaceSystem.h"
#include "StarSystemScreen.h"
#include "TestBlockViewScreen.h"
#include "TestConsoleScreen.h"
#include "TestDeferredScreen.h"
#include "TestMappingScreen.h"


void App::addScreens() {
    scrInit = new InitScreen(this);
    scrLoad = new LoadScreen(this);
    scrMainMenu = new MainMenuScreen(this);
    scrGamePlay = new GamePlayScreen(this);
    scrStarSystem = new StarSystemScreen(this);

    _screenList->addScreen(scrInit);
    _screenList->addScreen(scrLoad);
    _screenList->addScreen(scrMainMenu);
    _screenList->addScreen(scrGamePlay);
    _screenList->addScreen(scrStarSystem);

    // Add development screen
    scrDev = new DevScreen;
    scrDev->addScreen(VKEY_RETURN, scrInit);
    scrDev->addScreen(VKEY_SPACE, scrInit);
    scrDev->addScreen(VKEY_S, scrStarSystem);
    _screenList->addScreen(scrDev);

    // Add test screens
    scrTests.push_back(new TestConsoleScreen);
    _screenList->addScreen(scrTests.back());
    scrDev->addScreen(VKEY_C, scrTests.back());
    scrTests.push_back(new TestMappingScreen);
    _screenList->addScreen(scrTests.back());
    scrDev->addScreen(VKEY_M, scrTests.back());
    scrTests.push_back(new TestDeferredScreen);
    _screenList->addScreen(scrTests.back());
    scrDev->addScreen(VKEY_D, scrTests.back());
    scrTests.push_back(new TestBlockView);
    _screenList->addScreen(scrTests.back());
    scrDev->addScreen(VKEY_B, scrTests.back());


    // Start from dev screen for convenience
    _screenList->setScreen(scrInit->getIndex());
}

void App::onInit() {
    
    // Load the graphical options
    loadOptions("Data/Options.yml");

    SamplerState::initPredefined();

    // Allocate resources
    meshManager = new MeshManager;
    spaceSystem = new SpaceSystem(this);
    spaceSystem->init(GameManager::glProgramManager);
    saveFileIom = new IOManager;
    inputManager = new InputManager;
}

void App::onExit() {
    // Delete cache if it exists

    SpriteBatch::disposeProgram();
}

App::~App() {

    delete scrInit;
    delete scrLoad;
    delete scrMainMenu;
    delete scrGamePlay;
    delete scrDev;
    delete scrStarSystem; 
    delete inputManager;

    delete meshManager;
}
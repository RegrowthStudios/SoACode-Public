#include "stdafx.h"
#include "App.h"

#include "InitScreen.h"
#include "LoadScreen.h"
#include "MainMenuScreen.h"
#include "ScreenList.h"

void App::addScreens() {
    scrInit = new InitScreen(this);
    scrLoad = new LoadScreen(this);
    scrMainMenu = new MainMenuScreen(this);

    _screenList->addScreen(scrInit);
    _screenList->addScreen(scrLoad);
    _screenList->addScreen(scrMainMenu);

    _screenList->setScreen(scrInit->getIndex());
}
void App::onInit() {
    // Empty
}
void App::onExit() {
    // Empty
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
}

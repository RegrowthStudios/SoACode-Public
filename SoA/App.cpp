#include "stdafx.h"
#include "App.h"

#include "InitScreen.h"
#include "LoadScreen.h"
#include "ScreenList.h"
#include "TestScreen.h"

void App::addScreens() {
    scrInit = new InitScreen(this);
    scrLoad = new LoadScreen(this);

    _screenList->addScreen(scrInit);
    _screenList->addScreen(scrLoad);
    _screenList->setScreen(scrInit->getIndex());
}
void App::onInit() {
    // empty
}
void App::onExit() {
    // empty
}
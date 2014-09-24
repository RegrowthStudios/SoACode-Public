#include "stdafx.h"
#include "App.h"

#include "ScreenList.h"
#include "TestScreen.h"


void App::addScreens() {
    _screenList->addScreen(new TestScreen);
    _screenList->setScreen(0);
}


void App::onInit() {
}


void App::onExit() {
}
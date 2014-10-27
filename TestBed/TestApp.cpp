#include "stdafx.h"
#include "TestApp.h"

#include "HSLScreen.h"
#include "ScreenList.h"

void TestApp::onInit() {

}

void TestApp::addScreens() {
    _screenList->addScreen(new HSLScreen());
    _screenList->setScreen(0);
}

void TestApp::onExit() {

}

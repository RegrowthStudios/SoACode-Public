#include "stdafx.h"
#include "MainMenuScreen.h"
#include "GameManager.h"

CTOR_APP_SCREEN_DEF(MainMenuScreen, App) ,
_isInitialized(false) {
    // Empty
}

void MainMenuScreen::onEntry(const GameTime& gameTime) {
    if (!_isInitialized) {
        GameManager::physicsThread = new thread(physicsThread);

        _isInitialized = true;
    }
}
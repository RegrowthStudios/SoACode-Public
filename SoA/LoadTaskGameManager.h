#pragma once
#include <SDL/SDL.h>

#include "GameManager.h"
#include "InputManager.h"
#include "LoadMonitor.h"

// Sample Dependency Task
class LoadTaskGameManager : public ILoadTask {
    virtual void load() {
        GameManager::gameState = GameStates::MAINMENU;
        GameManager::initializeSystems();
        GameManager::registerTexturesForLoad();
    }
};
#pragma once
#include <SDL/SDL.h>

#include "Player.h"
#include "GameManager.h"
#include "InputManager.h"
#include "LoadMonitor.h"

// Sample Dependency Task
class LoadTaskGameManager : public ILoadTask {
    virtual void load() {
        GameManager::inputManager = new InputManager;
        initInputs();
        GameManager::inputManager->loadAxes();
        GameManager::gameState = GameStates::MAINMENU;
        GameManager::initializeSystems();
        GameManager::player = new Player();
        #define SPEEDMOD 0.0095f
        GameManager::player->setMoveSpeed(SPEEDMOD, 0.166f);
        GameManager::registerTexturesForLoad();
    }
};
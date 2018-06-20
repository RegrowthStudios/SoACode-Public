#pragma once
#include <SDL2/SDL.h>

#include "GameManager.h"
#include "InputMapper.h"
#include "LoadMonitor.h"

// Sample Dependency Task
class LoadTaskGameManager : public ILoadTask {
    virtual void load() {
        GameManager::initializeSystems();
        GameManager::registerTexturesForLoad();
    }
};
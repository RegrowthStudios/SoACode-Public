#pragma once
#include "LoadMonitor.h"
#include "SpaceSystem.h"

#include "SoaEngine.h"

// Sample Dependency Task
class LoadTaskStarSystem : public ILoadTask {
    friend class MainMenuLoadScreen;

    LoadTaskStarSystem(const nString& filePath, SoaState* state) :
        soaState(state),
        filePath(filePath) {
        // Empty
    }
    virtual void load() {
        SoaEngine::loadSpaceSystem(soaState, filePath);
    }

    nString filePath;
    SoaState* soaState;
};
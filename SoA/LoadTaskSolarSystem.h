#pragma once
#include "LoadMonitor.h"
#include "SpaceSystem.h"

#include "SoaEngine.h"

// Sample Dependency Task
class LoadTaskSolarSystem : public ILoadTask {
    friend class LoadScreen;

    LoadTaskSolarSystem(const nString& filePath, SoaState* state) :
        soaState(state) {
        loadData.filePath = filePath;
    }
    virtual void load() {
        SoaEngine::loadSpaceSystem(soaState, loadData);
    }
    
    SoaEngine::SpaceSystemLoadData loadData;
    SoaState* soaState;
};
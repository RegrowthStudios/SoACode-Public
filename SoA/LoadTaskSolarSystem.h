#pragma once
#include "LoadMonitor.h"
#include "SpaceSystem.h"

// Sample Dependency Task
class LoadTaskSolarSystem : public ILoadTask {
    friend class LoadScreen;

    LoadTaskSolarSystem(nString FilePath, SpaceSystem* SpaceSystem) :
        filePath(FilePath),
        spaceSystem(SpaceSystem) {
        // Empty
    }
    virtual void load() {
        spaceSystem->addSolarSystem(filePath);
    }
    
    nString filePath;
    SpaceSystem* spaceSystem;
};
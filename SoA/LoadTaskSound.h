#pragma once
#include "GameManager.h"
#include "LoadMonitor.h"

// Sample Dependency Task
class LoadTaskSound : public ILoadTask {
    virtual void load() {
        GameManager::initializeSound();
    }
};
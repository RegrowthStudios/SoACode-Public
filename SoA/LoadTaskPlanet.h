#pragma once
#include "LoadMonitor.h"
#include "GameManager.h"
#include "Camera.h"
#include "Planet.h"

class LoadTaskPlanet : public ILoadTask {
    friend class LoadScreen;
    virtual void load() {
        GameManager::loadPlanet("SolarSystems/Hyperion/Planets/Aldrin.yml");
    }
};
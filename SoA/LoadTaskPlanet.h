#pragma once
#include "LoadMonitor.h"
#include "GameManager.h"
#include "Camera.h"
#include "Planet.h"

class LoadTaskPlanet : public ILoadTask {
    friend class LoadScreen;
    virtual void load() {
        GameManager::loadPlanet("Worlds/Aldrin/");

        GameManager::initializePlanet(glm::dvec3(0.0, 0.0, 1000000000)); 
    }
};
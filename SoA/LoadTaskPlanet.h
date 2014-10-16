#pragma once
#include "LoadMonitor.h"
#include "GameManager.h"
#include "Camera.h"
#include "Planet.h"
#include "OpenglManager.h"

class LoadTaskPlanet : public ILoadTask {
    virtual void load() {
        GameManager::loadPlanet("Worlds/Aldrin/");

        mainMenuCamera.setPosition(glm::dvec3(0.0, 0.0, 1000000000));
        mainMenuCamera.setDirection(glm::vec3(0.0, 0.0, -1.0));
        mainMenuCamera.setRight(glm::vec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0));
        mainMenuCamera.setUp(glm::cross(mainMenuCamera.right(), mainMenuCamera.direction()));
        mainMenuCamera.setClippingPlane(1000000.0f, 30000000.0f);

        GameManager::initializePlanet(mainMenuCamera.position());

        mainMenuCamera.zoomTo(glm::dvec3(0.0, 0.0, GameManager::planet->radius * 1.35), 3.0, glm::dvec3(0.0, 0.0, -1.0), glm::dvec3(cos(GameManager::planet->axialZTilt), sin(GameManager::planet->axialZTilt), 0.0), glm::dvec3(0.0), GameManager::planet->radius, 0.0);
        
    }
};
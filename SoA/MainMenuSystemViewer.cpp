#include "stdafx.h"
#include "MainMenuSystemViewer.h"


MainMenuSystemViewer::MainMenuSystemViewer(CinematicCamera* camera, SpaceSystem* spaceSystem, InputManager* inputManager) :
        m_camera(camera),
        m_spaceSystem(spaceSystem),
        m_inputManager(inputManager) {
    
    // Initialize the camera
    m_camera->setPosition(glm::dvec3(0.0, 200000.0, 0.0));
    m_camera->setDirection(glm::vec3(0.0, -1.0, 0.0));
    m_camera->setUp(glm::cross(m_camera.getRight(), m_camera.getDirection()));
    m_camera->setClippingPlane(10000.0f, 3000000000000.0f);
    m_camera->setTarget(glm::dvec3(0.0, 0.0, 0.0), f32v3(1.0f, 0.0f, 0.0f), f32v3(0.0f, 0.0f, 1.0f), 20000.0);

    m_spaceSystem->targetBody("Aldrin");

}

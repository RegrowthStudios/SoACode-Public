#include "stdafx.h"
#include "MainMenuSystemViewer.h"

#include <Vorb/InputDispatcher.h>
#include <Vorb/utils.h>

#include "Camera.h"
#include "SpaceSystem.h"

MainMenuSystemViewer::MainMenuSystemViewer(ui32v2 viewport, CinematicCamera* camera,
                                           SpaceSystem* spaceSystem, InputManager* inputManager) :
        m_viewport(viewport),
        m_camera(camera),
        m_spaceSystem(spaceSystem),
        m_inputManager(inputManager) {
    
    // Initialize the camera
    m_camera->setPosition(glm::dvec3(0.0, 200000.0, 0.0));
    m_camera->setDirection(glm::vec3(0.0, -1.0, 0.0));
    m_camera->setUp(glm::cross(m_camera->getRight(), m_camera->getDirection()));
    m_camera->setClippingPlane(10000.0f, 3000000000000.0f);
    m_camera->setTarget(glm::dvec3(0.0, 0.0, 0.0), f32v3(1.0f, 0.0f, 0.0f), f32v3(0.0f, 0.0f, 1.0f), 20000.0);

    m_spaceSystem->targetBody("Aldrin");

    // Register events
    vui::InputDispatcher::mouse.onButtonDown.addFunctor(([=](void* s, const vui::MouseButtonEvent& e) { onMouseButtonDown(s, e); }));
    vui::InputDispatcher::mouse.onButtonUp.addFunctor(([=](void* s, const vui::MouseButtonEvent& e) { onMouseButtonUp(s, e); }));
    vui::InputDispatcher::mouse.onMotion.addFunctor(([=](void* s, const vui::MouseMotionEvent& e) { onMouseMotion(s, e); }));
    vui::InputDispatcher::mouse.onWheel.addFunctor(([=](void* s, const vui::MouseWheelEvent& e) { onMouseWheel(s, e); }));
}

MainMenuSystemViewer::~MainMenuSystemViewer() {
    // Empty
}

void MainMenuSystemViewer::update() {
    const float HOVER_SPEED = 0.08f;
    const float HOVER_SIZE_INC = 7.0f;
    const float ROTATION_FACTOR = M_PI * 2.0f + M_PI / 4;
    const float MIN_SELECTOR_SIZE = 12.0f;
    const float MAX_SELECTOR_SIZE = 160.0f;

    // Render all bodies
    for (auto& it : m_spaceSystem->m_namePositionCT) {
        vcore::ComponentID componentID;

        f64v3 relativePos = it.second.position - m_camera->getPosition();
        f64 distance = glm::length(relativePos);
        float radiusPixels;
        float radius;

        BodyArData& data = bodyArData[it.first];
        float hoverTime = data.hoverTime;

        if (m_camera->pointInFrustum(f32v3(relativePos))) {
            data.inFrustum = true;
            // Get screen position 
            f32v3 screenCoords = m_camera->worldToScreenPoint(relativePos);
            f32v2 xyScreenCoords(screenCoords.x * m_viewport.x, screenCoords.y * m_viewport.y);

            // Get a smooth interpolator with hermite
            float interpolator = hermite(hoverTime);

            // See if it has a radius
            componentID = m_spaceSystem->m_sphericalGravityCT.getComponentID(it.first);
            if (componentID) {
                // Get radius of projected sphere
                radius = m_spaceSystem->m_sphericalGravityCT.get(componentID).radius;
                radiusPixels = (radius /
                                (tan(m_camera->getFieldOfView() / 2) * distance)) *
                                (m_viewport.y / 2.0f);
            } else {
                radius = 1000.0f;
                radiusPixels = (radius /
                                (tan(m_camera->getFieldOfView() / 2) * distance)) *
                                (m_viewport.y / 2.0f);
            }

            float selectorSize = radiusPixels * 2.0f + 3.0f;
            if (selectorSize < MIN_SELECTOR_SIZE) selectorSize = MIN_SELECTOR_SIZE;

            // Interpolate size
            selectorSize += interpolator * HOVER_SIZE_INC;

            // Detect mouse hover
            if (glm::length(m_mouseCoords - xyScreenCoords) <= selectorSize / 2.0f) {
                hoverTime += HOVER_SPEED;
                if (hoverTime > 1.0f) hoverTime = 1.0f;
            } else {
                hoverTime -= HOVER_SPEED;
                if (hoverTime < 0.0f) hoverTime = 0.0f;
            }

            data.hoverTime = hoverTime;
            data.selectorSize = selectorSize;
        } else {
            data.inFrustum = false;
        }
    }
}

void MainMenuSystemViewer::onMouseButtonDown(void* sender, const vui::MouseButtonEvent& e) {

}

void MainMenuSystemViewer::onMouseButtonUp(void* sender, const vui::MouseButtonEvent& e) {

}

void MainMenuSystemViewer::onMouseWheel(void* sender, const vui::MouseWheelEvent& e) {

}

void MainMenuSystemViewer::onMouseMotion(void* sender, const vui::MouseMotionEvent& e) {

}

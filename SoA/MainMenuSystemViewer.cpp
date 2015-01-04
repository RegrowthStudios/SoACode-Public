#include "stdafx.h"
#include "MainMenuSystemViewer.h"

#include <Vorb/InputDispatcher.h>
#include <Vorb/utils.h>

#include "Camera.h"
#include "SpaceSystem.h"

const float MainMenuSystemViewer::MIN_SELECTOR_SIZE = 12.0f;
const float MainMenuSystemViewer::MAX_SELECTOR_SIZE = 160.0f;

MainMenuSystemViewer::MainMenuSystemViewer(ui32v2 viewport, CinematicCamera* camera,
                                           SpaceSystem* spaceSystem, InputManager* inputManager) :
        m_viewport(viewport),
        m_camera(camera),
        m_spaceSystem(spaceSystem),
        m_inputManager(inputManager) {
    
    mouseButtons[0] = false;
    mouseButtons[1] = false;

    // Initialize the camera
    m_camera->setPosition(glm::dvec3(0.0, 200000.0, 0.0));

    // Initialize the camera
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
                data.isHovering = true;
                data.hoverEntity = it.first;
                hoverTime += HOVER_SPEED;
                if (hoverTime > 1.0f) hoverTime = 1.0f;
            } else {
                data.isHovering = false;
                hoverTime -= HOVER_SPEED;
                if (hoverTime < 0.0f) hoverTime = 0.0f;
            }

            data.hoverTime = hoverTime;
            data.selectorSize = selectorSize;
        } else {
            data.isHovering = false;
            data.inFrustum = false;
        }
    }

    // Connect camera to target planet
    float length = m_camera->getFocalLength() / 10.0;
    if (length == 0) length = 0.1;
    m_camera->setClippingPlane(length, m_camera->getFarClip());
    // Target closest point on sphere
    m_camera->setTargetFocalPoint(m_spaceSystem->getTargetPosition() -
                                 f64v3(glm::normalize(m_camera->getDirection())) * m_spaceSystem->getTargetRadius());

}

void MainMenuSystemViewer::onMouseButtonDown(void* sender, const vui::MouseButtonEvent& e) {
    m_mouseCoords.x = e.x;
    m_mouseCoords.y = e.y;
    if (e.button == vui::MouseButton::LEFT) {
        mouseButtons[0] = true;
        // Target a body if we clicked on one
        for (auto& it : bodyArData) {
            if (it.second.isHovering) {
                pickStartLocation(it.second.hoverEntity);
                m_spaceSystem->targetBody(it.first);
                break;
            }
        }
    } else {
        mouseButtons[1] = true;
    }
}

void MainMenuSystemViewer::onMouseButtonUp(void* sender, const vui::MouseButtonEvent& e) {
    m_mouseCoords.x = e.x;
    m_mouseCoords.y = e.y;
    if (e.button == vui::MouseButton::LEFT) {
        mouseButtons[0] = false;
    } else {
        mouseButtons[1] = false;
    }
}

void MainMenuSystemViewer::onMouseWheel(void* sender, const vui::MouseWheelEvent& e) {
#define SCROLL_SPEED 0.1f
    m_camera->offsetTargetFocalLength(m_camera->getTargetFocalLength() * SCROLL_SPEED * -e.dy);
}

void MainMenuSystemViewer::onMouseMotion(void* sender, const vui::MouseMotionEvent& e) {
    m_mouseCoords.x = e.x;
    m_mouseCoords.y = e.y;

#define MOUSE_SPEED 0.1f
    if (mouseButtons[0]) {
        m_camera->rotateFromMouse((float)-e.dx, (float)-e.dy, MOUSE_SPEED);
    }
    if (mouseButtons[1]) {
        m_camera->yawFromMouse((float)e.dx, MOUSE_SPEED);
    }
}

inline float sum(const f32v3& v) {
    return v.x + v.y + v.z;
}

inline bool intersect(const f32v3& raydir, const f32v3& rayorig, const f32v3& pos,
                      const float& rad, f32v3& hitpoint, float& distance, f32v3& normal) {
    float a = sum(raydir*raydir);
    float b = sum(raydir * (2.0f * (rayorig - pos)));
    float c = sum(pos*pos) + sum(rayorig*rayorig) - 2.0f*sum(rayorig*pos) - rad*rad;
    float D = b*b + (-4.0f)*a*c;

    // If ray can not intersect then stop
    if (D < 0) {
        return false;
    }
    D = sqrtf(D);

    // Ray can intersect the sphere, solve the closer hitpoint
    float t = (-0.5f)*(b + D) / a;
    if (t > 0.0f) {
        distance = sqrtf(a)*t;
        hitpoint = rayorig + t*raydir;
        normal = (hitpoint - pos) / rad;
    } else {
        return false;
    }
    return true;
}

void MainMenuSystemViewer::pickStartLocation(vcore::EntityID eid) {
    f32v2 ndc = f32v2((m_mouseCoords.x / m_viewport.x) * 2.0f - 1.0f,
        1.0f - (m_mouseCoords.y / m_viewport.y) * 2.0f);
    f32v3 pickRay = m_camera->getPickRay(ndc);

    vcore::ComponentID cid = m_spaceSystem->m_namePositionCT.getComponentID(eid);
    if (!cid) return;
    f64v3 centerPos = m_spaceSystem->m_namePositionCT.get(cid).position;
    f32v3 pos = f32v3(centerPos - m_camera->getPosition());

    cid = m_spaceSystem->m_sphericalGravityCT.getComponentID(eid);
    if (!cid) return;
    float radius = m_spaceSystem->m_sphericalGravityCT.get(cid).radius;

    // Compute the intersection
    f32v3 normal, hitpoint;
    float distance;
    if (intersect(pickRay, f32v3(0.0f), pos, radius, hitpoint, distance, normal)) {
        auto& data = bodyArData[eid];
        data.selectedPos = hitpoint - pos;
        data.isLandSelected = true;
    }
}
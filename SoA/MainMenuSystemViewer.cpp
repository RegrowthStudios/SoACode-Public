#include "stdafx.h"
#include "MainMenuSystemViewer.h"

#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/utils.h>

#include "Camera.h"
#include "SpaceSystem.h"
#include "TerrainPatch.h"
#include "SphericalHeightmapGenerator.h"

const f32 MainMenuSystemViewer::MIN_SELECTOR_SIZE = 12.0f;
const f32 MainMenuSystemViewer::MAX_SELECTOR_SIZE = 160.0f;

void MainMenuSystemViewer::init(ui32v2 viewport, CinematicCamera* camera,
                                SpaceSystem* spaceSystem, InputMapper* inputManager) {  
    m_viewport = viewport;
    m_camera = camera;
    m_spaceSystem = spaceSystem;
    m_inputManager = inputManager;

    mouseButtons[0] = false;
    mouseButtons[1] = false;

    // Initialize the camera
    m_camera->setPosition(f64v3(0.0, 200000.0, 0.0));

    // Initialize the camera
    m_camera->setClippingPlane(10000.0f, 30000000000000.0f);
    m_camera->setTarget(f64v3(0.0, 0.0, 0.0), f32v3(1.0f, 0.0f, 0.0f), f32v3(0.0f, 0.0f, 1.0f), 20000.0);

    targetBody("Aldrin");
    // Force target focal point
    m_camera->setFocalPoint(getTargetPosition() -
                            f64v3(vmath::normalize(m_camera->getDirection())) * getTargetRadius());

    // Register events
    startInput();
}

void MainMenuSystemViewer::dispose() {
    stopInput();
}

void MainMenuSystemViewer::update() {

    const f32 HOVER_SPEED = 0.08f;
    const f32 HOVER_SIZE_INC = 7.0f;

    m_camera->setClippingPlane((f32)(0.1 * KM_PER_M), m_camera->getFarClip());
    // Target closest point on sphere
    m_camera->setTargetFocalPoint(getTargetPosition() -
                                  f64v3(vmath::normalize(m_camera->getDirection())) * getTargetRadius());

    m_camera->update();

    for (auto& it : m_spaceSystem->namePosition) {
        vecs::ComponentID componentID;

        f64v3 relativePos = it.second.position - m_camera->getPosition();
        f64 distance = vmath::length(relativePos);
        f64 radiusPixels;
        f64 radius;

        BodyArData& data = bodyArData[it.first];
        f32 hoverTime = data.hoverTime;

        if (m_camera->pointInFrustum(f32v3(relativePos))) {
            data.inFrustum = true;
            // Get screen position 
            f32v3 screenCoords = m_camera->worldToScreenPoint(relativePos);
            f32v2 xyScreenCoords(screenCoords.x * m_viewport.x, screenCoords.y * m_viewport.y);

            // Get a smooth interpolator with hermite
            f32 interpolator = hermite(hoverTime);

            // See if it has a radius
            componentID = m_spaceSystem->sphericalGravity.getComponentID(it.first);
            if (componentID) {
                // Get radius of projected sphere
                radius = m_spaceSystem->sphericalGravity.get(componentID).radius;
                radiusPixels = (radius /
                                (tan((f64)m_camera->getFieldOfView() / 2.0) * distance)) *
                                ((f64)m_viewport.y / 2.0);
            } else {
                radius = 1000.0;
                radiusPixels = (radius /
                                (tan((f64)m_camera->getFieldOfView() / 2.0) * distance)) *
                                ((f64)m_viewport.y / 2.0);
            }

            f32 selectorSize = (f32)(radiusPixels * 2.0 + 3.0);
            if (selectorSize < MIN_SELECTOR_SIZE) selectorSize = MIN_SELECTOR_SIZE;

            // Interpolate size
            selectorSize += interpolator * HOVER_SIZE_INC;

            // Detect mouse hover
            if (vmath::length(m_mouseCoords - xyScreenCoords) <= selectorSize / 2.0f) {
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
}

void MainMenuSystemViewer::startInput() {
    stopInput();
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [=](Sender s, const vui::MouseButtonEvent& e) { onMouseButtonDown(s, e); });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [=](Sender s, const vui::MouseButtonEvent& e) { onMouseButtonUp(s, e); });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [=](Sender s, const vui::MouseMotionEvent& e) { onMouseMotion(s, e); });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onWheel, [=](Sender s, const vui::MouseWheelEvent& e) { onMouseWheel(s, e); });
}

void MainMenuSystemViewer::stopInput() {
    m_hooks.dispose();
}

void MainMenuSystemViewer::targetBody(const nString& name) {
    for (auto& it : m_spaceSystem->namePosition) {
        if (it.second.name == name) {
            targetBody(it.first);
            return;
        }
    }
}

void MainMenuSystemViewer::targetBody(vecs::EntityID eid) {
    if (m_targetEntity != eid) {
        TargetChange(eid);
        m_targetEntity = eid;
        m_targetComponent = m_spaceSystem->namePosition.getComponentID(m_targetEntity);
    }
}

f64v3 MainMenuSystemViewer::getTargetPosition() {
    return m_spaceSystem->namePosition.get(m_targetComponent).position;
}

f64 MainMenuSystemViewer::getTargetRadius() {
    return m_spaceSystem->sphericalGravity.getFromEntity(m_targetEntity).radius;
}

nString MainMenuSystemViewer::getTargetName() {
    return m_spaceSystem->namePosition.get(m_targetComponent).name;
}

void MainMenuSystemViewer::onMouseButtonDown(Sender sender, const vui::MouseButtonEvent& e) {
    m_mouseCoords.x = (f32)e.x;
    m_mouseCoords.y = (f32)e.y;
    if (e.button == vui::MouseButton::LEFT) {
        mouseButtons[0] = true;
        // Target a body if we clicked on one
        f64 closestDist = 99999999999999999999999999999.0;
        vecs::EntityID closestEntity = 0;
        for (auto& it : bodyArData) {
            if (it.second.isHovering) {

                // Check distance so we pick only the closest one
                f64v3 pos = m_spaceSystem->namePosition.getFromEntity(it.first).position;
                f64 dist = vmath::length(pos - m_camera->getPosition());
                if (dist < closestDist) {
                    closestDist = dist;
                    closestEntity = it.first;
                } else {
                    it.second.isLandSelected = false;
                }
            }
        }

        // If we selected an entity, then do the target picking
        if (closestEntity) {
            targetBody(closestEntity);
            pickStartLocation(closestEntity);
        }
    } else {
        mouseButtons[1] = true;
    }
}

void MainMenuSystemViewer::onMouseButtonUp(Sender sender, const vui::MouseButtonEvent& e) {
    m_mouseCoords.x = (f32)e.x;
    m_mouseCoords.y = (f32)e.y;
    if (e.button == vui::MouseButton::LEFT) {
        mouseButtons[0] = false;
    } else {
        mouseButtons[1] = false;
    }
}

void MainMenuSystemViewer::onMouseWheel(Sender sender, const vui::MouseWheelEvent& e) {
#define SCROLL_SPEED 0.1f
    m_camera->offsetTargetFocalLength((f32)m_camera->getTargetFocalLength() * SCROLL_SPEED * -e.dy);
    if (m_camera->getTargetFocalLength() < 0.1f) {
        m_camera->setTargetFocalLength(0.1f);
    }
}

void MainMenuSystemViewer::onMouseMotion(Sender sender, const vui::MouseMotionEvent& e) {
    m_mouseCoords.x = (f32)e.x;
    m_mouseCoords.y = (f32)e.y;

#define MOUSE_SPEED 0.001f
    if (mouseButtons[0]) {
        m_camera->rotateFromMouse((f32)-e.dx, (f32)-e.dy, MOUSE_SPEED);
    }
    if (mouseButtons[1]) {
        m_camera->rollFromMouse((f32)e.dx, MOUSE_SPEED);
    }
}

void MainMenuSystemViewer::pickStartLocation(vecs::EntityID eid) {
    // Check to see if it even has terrain by checking if it has a generator
    if (!m_spaceSystem->sphericalTerrain.getFromEntity(m_targetEntity).cpuGenerator) return;
    // Select the planet
    m_selectedPlanet = eid;

    f32v2 ndc = f32v2((m_mouseCoords.x / m_viewport.x) * 2.0f - 1.0f,
        1.0f - (m_mouseCoords.y / m_viewport.y) * 2.0f);
    f64v3 pickRay(m_camera->getPickRay(ndc));

    vecs::ComponentID cid = m_spaceSystem->namePosition.getComponentID(eid);
    if (!cid) return;
    f64v3 centerPos = m_spaceSystem->namePosition.get(cid).position;
    f64v3 pos = f64v3(centerPos - m_camera->getPosition());

    cid = m_spaceSystem->sphericalGravity.getComponentID(eid);
    if (!cid) return;
    f64 radius = m_spaceSystem->sphericalGravity.get(cid).radius;

    // Compute the intersection
    f64v3 normal, hitpoint;
    f64 distance;
    if (IntersectionUtils::sphereIntersect(pickRay, f64v3(0.0f), pos, radius, hitpoint, distance, normal)) {
        hitpoint -= pos;
        cid = m_spaceSystem->axisRotation.getComponentID(eid);
        if (cid) {
            f64q rot = m_spaceSystem->axisRotation.get(cid).currentOrientation;
            hitpoint = vmath::inverse(rot) * hitpoint;
        }

        // Compute face and grid position
        PlanetHeightData heightData;
        m_spaceSystem->sphericalTerrain.getFromEntity(m_targetEntity).cpuGenerator->generateHeightData(heightData, vmath::normalize(hitpoint));
        f64 height = heightData.height * KM_PER_VOXEL;

        m_clickPos = f64v3(hitpoint + vmath::normalize(hitpoint) * height);

        auto& data = bodyArData[eid];
        data.selectedPos = hitpoint;
        data.isLandSelected = true;
    }
}

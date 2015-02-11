///
/// MainMenuSystemViewer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 3 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Controls the camera and state when viewing/selecting
/// the star system in the main menu.
///

#pragma once

#ifndef MainMenuSystemViewer_h__
#define MainMenuSystemViewer_h__

#include <Vorb/Vorb.h>
// Temporary
#include <Vorb/ui/MouseInputDispatcher.h>
#include <Vorb/ecs/ECS.h>

#include "VoxelCoordinateSpaces.h"

class CinematicCamera;
class InputManager;
class SpaceSystem;

class MainMenuSystemViewer {
public:
    MainMenuSystemViewer(ui32v2 viewport, CinematicCamera* camera, SpaceSystem* spaceSystem, InputManager* inputManager);
    ~MainMenuSystemViewer();

    void setViewport(const ui32v2& viewPort) { m_viewport = viewPort; }

    void update();

    struct BodyArData {
        f32 hoverTime = 0.0f;
        f32 selectorSize = 0.0f;
        bool inFrustum = false;
        vcore::EntityID hoverEntity = 0;
        bool isHovering = false;
        bool isLandSelected = false;
        f32v3 selectedPos;
    };
    const BodyArData* finBodyAr(vcore::EntityID eid) const {
        auto& it = bodyArData.find(eid);
        if (it == bodyArData.end()) return nullptr;
        return &it->second;
    }

    /// Targets a named body
    /// @param name: Name of the body
    void targetBody(const nString& name);
    /// Targets an entity
    /// @param eid: Entity ID
    void targetBody(vcore::EntityID eid);

    /// Getters
    const f32v3& getSelectedGridPos() const { return m_selectedGridPos; }
    const int& getSelectedCubeFace() const { return m_selectedCubeFace; }
    vcore::EntityID getSelectedPlanet() const { return m_selectedPlanet; }
    const f64v3& getClickPos() const { return m_clickPos; }

    /// Gets the position of the targeted entity
    /// @return position
    f64v3 getTargetPosition();

    /// Gets the position of the targeted entity
    /// @return radius
    f64 getTargetRadius();

    /// Gets the name of the targeted component
    /// @return position
    nString getTargetName();

    static const f32 MIN_SELECTOR_SIZE;
    static const f32 MAX_SELECTOR_SIZE;

private:
    // Events
    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    void onMouseButtonDown(Sender sender, const vui::MouseButtonEvent& e);
    void onMouseButtonUp(Sender sender, const vui::MouseButtonEvent& e);
    void onMouseWheel(Sender sender, const vui::MouseWheelEvent& e);
    void onMouseMotion(Sender sender, const vui::MouseMotionEvent& e);

    void pickStartLocation(vcore::EntityID eid);
    void computeGridPosition(const f32v3& hitpoint, f32 radius, OUT f32& height);

    nString currentBody = "";

    std::map <vcore::EntityID, BodyArData> bodyArData;

    vcore::EntityID m_targetEntity = 1; ///< Current entity we are focusing on
    vcore::ComponentID m_targetComponent = 1; ///< namePositionComponent of the targetEntity

    bool mouseButtons[2];
    f32v2 m_mouseCoords = f32v2(-1.0f);
    f32v2 m_viewport;
    f64v3 m_clickPos = f64v3(0.0);

    f32v3 m_selectedGridPos = f32v3(0.0f);
    WorldCubeFace m_selectedCubeFace = WorldCubeFace::FACE_NONE;

    vcore::EntityID m_selectedPlanet = 0;

    CinematicCamera* m_camera = nullptr;
    SpaceSystem* m_spaceSystem = nullptr;
    InputManager* m_inputManager = nullptr;
};

#endif // MainMenuSystemViewer_h__
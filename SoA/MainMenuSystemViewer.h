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
#include <Vorb/MouseInputDispatcher.h>
#include <Vorb/ECS.h>

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
        float hoverTime = 0.0f;
        float selectorSize = 0.0f;
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

    /// Getters
    const f32v2& getSelectedGridPos() const { return m_selectedGridPos; }
    const int& getSelectedCubeFace() const { return m_selectedCubeFace; }

    static const float MIN_SELECTOR_SIZE;
    static const float MAX_SELECTOR_SIZE;

private:
    // Events
    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    void onMouseButtonDown(Sender sender, const vui::MouseButtonEvent& e);
    void onMouseButtonUp(Sender sender, const vui::MouseButtonEvent& e);
    void onMouseWheel(Sender sender, const vui::MouseWheelEvent& e);
    void onMouseMotion(Sender sender, const vui::MouseMotionEvent& e);

    void pickStartLocation(vcore::EntityID eid);
    void computeGridPosition(const f32v3& hitpoint, float radius);

    nString currentBody = "";

    std::map <vcore::EntityID, BodyArData> bodyArData;

    bool mouseButtons[2];
    f32v2 m_mouseCoords = f32v2(-1.0f);
    f32v2 m_viewport;

    f32v2 m_selectedGridPos = f32v2(0.0f);
    int m_selectedCubeFace = -1;

    CinematicCamera* m_camera = nullptr;
    SpaceSystem* m_spaceSystem = nullptr;
    InputManager* m_inputManager = nullptr;
};

#endif // MainMenuSystemViewer_h__
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
    };
    const BodyArData* finBodyAr(vcore::EntityID eid) const {
        auto& it = bodyArData.find(eid);
        if (it == bodyArData.end()) return nullptr;
        return &it->second;
    }

private:
    // Events
    void onMouseButtonDown(void* sender, const vui::MouseButtonEvent& e);
    void onMouseButtonUp(void* sender, const vui::MouseButtonEvent& e);
    void onMouseWheel(void* sender, const vui::MouseWheelEvent& e);
    void onMouseMotion(void* sender, const vui::MouseMotionEvent& e);

    nString currentBody = "";

    std::map <vcore::EntityID, BodyArData> bodyArData;


    f32v2 m_mouseCoords = f32v2(-1.0f);
    f32v2 m_viewport;

    CinematicCamera* m_camera = nullptr;
    SpaceSystem* m_spaceSystem = nullptr;
    InputManager* m_inputManager = nullptr;
};

#endif // MainMenuSystemViewer_h__
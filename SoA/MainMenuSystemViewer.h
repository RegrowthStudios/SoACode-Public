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

class CinematicCamera;
class InputManager;
class SpaceSystem;

class MainMenuSystemViewer {
public:
    MainMenuSystemViewer(CinematicCamera* camera, SpaceSystem* spaceSystem, InputManager* inputManager);

    void update();

private:
    nString currentBody = "";

    CinematicCamera* m_camera = nullptr;
    SpaceSystem* m_spaceSystem = nullptr;
    InputManager* m_inputManager = nullptr;
};

#endif // MainMenuSystemViewer_h__
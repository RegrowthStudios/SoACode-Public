///
/// PauseMenu.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Pause menu implementation for use in GamePlayScreen
///

#pragma once

#ifndef PauseMenu_h__
#define PauseMenu_h__

#include <Vorb/VorbPreDecl.inl>

DECL_VG(class GLProgram);
class GameplayScreen;


class PauseMenu {
public:
    PauseMenu();
    ~PauseMenu();

    /// Initializes the Pause Menu
    /// @param ownerScreen: The screen that owns this pause menu
    void init(GameplayScreen* ownerScreen);

    /// Opens the Pause Menu
    void open();

    /// Closes the Pause Menu
    void close();

    /// Updates the Pause Menu
    void update();

    /// Draws the Pause Menu
    void draw() const;

    /// Handles an event
    /// @param e: The event to handle
    //void onEvent(const SDL_Event& e);

    /// Frees all resources
    void destroy();

    /// Returns true if the Pause Menu is open
    const bool& isOpen() const { return _isOpen; }
private:
    vg::GLProgram* m_program = nullptr;
    bool _isOpen = false;
};

#endif // PauseMenu_h__
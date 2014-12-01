///
/// PauseMenuAwesomiumAPI.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Implements the awesomium API for the pause menu
///

#pragma once

#ifndef PauseMenuAwesomiumAPI_h__
#define PauseMenuAwesomiumAPI_h__

#include "IAwesomiumAPI.h"

class GamePlayScreen;

class PauseMenuAwesomiumAPI : public vui::IAwesomiumAPI<PauseMenuAwesomiumAPI> {
public:
    /// Initializes the API and hooks up all functions
    /// @oaram interfaceObject: The object that the API will talk to
    /// @param ownerScreen: The GamePlayScreen that owns this interface
    void init(Awesomium::JSObject* interfaceObject, IGameScreen* ownerScreen) override;

    /// Sets the owner screen. Should be a GamePlayScreen type
    /// @param ownerScreen: The screen
    void setOwnerScreen(IGameScreen* ownerScreen) override;
private:
    /// Continues the game
    /// @param args: Empty argument list
    void continueGame(const Awesomium::JSArray& args);

    /// Exits the game
    /// @param args: Empty argument list
    void exitGame(const Awesomium::JSArray& args);

    GamePlayScreen* _ownerScreen; ///< Handle to the main menu screen
};

#endif // PauseMenuAwesomiumAPI_h__
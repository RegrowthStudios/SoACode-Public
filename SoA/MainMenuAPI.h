// 
//  MainMenuAPI.h
//  Seed Of Andromeda
//
//  Created by Ben Arnold on 18 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides the function call API for
//  the main menu UI.
//

#pragma once

#ifndef MAINMENUAPI_H_
#define MAINMENUAPI_H_

#include "IAwesomiumAPI.h"

class IGameScreen;
class MainMenuScreen;

/// Awesomium API for MainMenuScreen
class MainMenuAPI : public IAwesomiumAPI<MainMenuAPI>
{
public:
    /// Initializes the API and hooks up all functions
    /// @oaram interfaceObject: The object that the API will talk to
    /// @param ownerScreen: The MainMenuScreen that owns this interface
    void init(Awesomium::JSObject* interfaceObject, IGameScreen* ownerScreen);

    // Sets the owner screen. Should be a MainMenuScreen type
    /// @param ownerScreen: The screen
    void setOwnerScreen(IGameScreen* ownerScreen);

private:
    /// Gets the camera position
    /// @param args: Empty arguments.
    /// @return float[3] position
    Awesomium::JSValue getCameraPosition(const Awesomium::JSArray& args);

    /// Gets the current planet radius
    /// @param args: Empty arguments.
    /// @return float radius
    Awesomium::JSValue getPlanetRadius(const Awesomium::JSArray& args);

    /// Gets a list of all save games
    /// @param args: Empty arguments.
    /// @return array of pairs specified as:
    /// pair<string filename, string timestamp>
    Awesomium::JSValue getSaveFiles(const Awesomium::JSArray& args);

    /// Sets the camera focal length
    /// @param args: Argument should be float.
    void setCameraFocalLength(const Awesomium::JSArray& args);

    /// Sets the camera position length
    /// @param args: Argument should be float[3].
    void setCameraPosition(const Awesomium::JSArray& args);

    /// Sets the camera position length
    /// @param args: Arguments should be 
    /// float[3]: camera pos,
    /// float: move time
    /// float: focal length
    /// float[3]: target dir vector
    /// float[3]: target right vector
    void setCameraTarget(const Awesomium::JSArray& args);

    /// Loads a save game and begins playing
    /// @param args: Argument should be the string name
    /// provided by getSaveFiles
    void loadSaveGame(const Awesomium::JSArray& args);

    /// Creates a new save game if name is valid and save
    /// doesn't already exist.
    /// @param args: Argument should be the string name
    void newSaveGame(const Awesomium::JSArray& args);

    MainMenuScreen* _ownerScreen; ///< Handle to the main menu screen
};

#endif // MAINMENUAPI_H_
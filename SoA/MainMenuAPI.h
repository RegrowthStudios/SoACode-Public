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
class MainMenuAPI : public vui::IAwesomiumAPI<MainMenuAPI>
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
    Awesomium::JSValue getCameraPosition(const Awesomium::JSArray& args);
    Awesomium::JSValue getPlanetRadius(const Awesomium::JSArray& args);

    void setCameraFocalLength(const Awesomium::JSArray& args);
    void setCameraPosition(const Awesomium::JSArray& args);
    void setCameraTarget(const Awesomium::JSArray& args);

    MainMenuScreen* _ownerScreen;
};

#endif // MAINMENUAPI_H_
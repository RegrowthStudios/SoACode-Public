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
    void init(Awesomium::WebView* webView, vui::CustomJSMethodHandler<MainMenuAPI>* methodHandler,
              vui::IGameScreen* ownerScreen) override;

    // Sets the owner screen. Should be a MainMenuScreen type
    /// @param ownerScreen: The screen
    void setOwnerScreen(vui::IGameScreen* ownerScreen) override;

private:
    Awesomium::JSValue initMainMenu();
    Awesomium::JSValue initVideoOptionsMenu();
    Awesomium::JSValue initControlsMenu();

    Awesomium::JSArray generateClickable(const cString name, const Awesomium::JSArray& linkData,
                                         const cString category, const cString description,
                                         int id, const cString updateCallback);
    Awesomium::JSArray generateText(const cString name, const cString text,
                                    const cString category, const cString description);
    Awesomium::JSArray generateToggle(const cString name, bool isToggled,
                                      const cString category, const cString description,
                                      int id, const cString updateCallback,
                                      bool updateInRealTime);
    Awesomium::JSArray generateSlider(const cString name, Awesomium::JSValue min,
                                      Awesomium::JSValue max, Awesomium::JSValue initialVal,
                                      Awesomium::JSValue intervalRes,
                                      const cString category, const cString description,
                                      int id, const cString updateCallback,
                                      bool updateInRealTime);
    Awesomium::JSArray generateDiscrete(const cString name, Awesomium::JSArray vals,
                                        Awesomium::JSValue initialVal,
                                        const cString category, const cString description,
                                        int id, const cString updateCallback,
                                        bool updateInRealTime);
    Awesomium::JSArray generateTextArea(const cString name,
                                        const cString defaultVal,
                                        int maxLength,
                                        const cString category, const cString description,
                                        int id);
    Awesomium::JSArray generateSubList(const cString name, Awesomium::JSArray subItems,
                                        const cString category, const cString description);
    Awesomium::JSArray generateCombo(const cString name, Awesomium::JSArray vals,
                                     Awesomium::JSValue initialVal,
                                     const cString category, const cString description,
                                     int id, const cString updateCallback,
                                     bool updateInRealTime);
    /// Gets the camera position
    /// @param args: Empty arguments.
    /// @return float[3] position
    Awesomium::JSValue getCameraPosition(const Awesomium::JSArray& args);

    /// Gets a list of all save games
    /// @param args: Empty arguments.
    /// @return array of pairs specified as:
    /// pair<string filename, string timestamp>
    Awesomium::JSValue getSaveFiles(const Awesomium::JSArray& args);

    Awesomium::JSValue getControls(const Awesomium::JSArray& args);

    Awesomium::JSValue getPageProperties(const Awesomium::JSArray& args);

    void setPage(const Awesomium::JSArray& args);

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

    // Updates an option
    void optionUpdate(const Awesomium::JSArray& args);

    /// Exits the game
    void quit(const Awesomium::JSArray& args);


    enum class MENU_PAGE {
        NONE,
        MAIN_MENU,
        VIDEO_OPTIONS_MENU,
        CONTROLS_MENU
    };
    MENU_PAGE m_currentPage = MENU_PAGE::MAIN_MENU;

    MainMenuScreen* _ownerScreen; ///< Handle to the main menu screen
};

#endif // MAINMENUAPI_H_
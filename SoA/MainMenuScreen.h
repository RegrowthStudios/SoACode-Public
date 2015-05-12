// 
//  MainMenuScreen.h
//  Seed Of Andromeda
//
//  Created by Ben Arnold on 17 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides the main menu screen
//  implementation. This screen encompasses the
//  gamestate for the main menu, as well as interaction
//  with the user interface.
//

#pragma once

#ifndef MAINMENUSCREEN_H_
#define MAINMENUSCREEN_H_

#include <Vorb/Random.h>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/io/IOManager.h>
#include <Vorb/ui/IGameScreen.h>

#include "LoadMonitor.h"
#include "MainMenuRenderPipeline.h"
#include "MainMenuForm.h"
#include "Camera.h"
#include <Vorb/ui/FormScriptEnvironment.h>

class App;

class InputMapper;
class LoadScreen;
class MainMenuSystemViewer;
class SoaState;
class SpaceSystemUpdater;

DECL_VSOUND(class Engine)
class AmbienceLibrary;
class AmbiencePlayer;

class MainMenuScreen : public vui::IAppScreen<App>
{
    friend class MainMenuAPI; ///< MainMenuAPI needs to talk directly to the MainMenuScreen
public:
    MainMenuScreen(const App* app, const LoadScreen* loadScreen);
    ~MainMenuScreen();

    virtual i32 getNextScreen() const;
    virtual i32 getPreviousScreen() const;

    virtual void build();
    virtual void destroy(const vui::GameTime& gameTime);

    virtual void onEntry(const vui::GameTime& gameTime);
    virtual void onExit(const vui::GameTime& gameTime);

    virtual void update(const vui::GameTime& gameTime);
    virtual void draw(const vui::GameTime& gameTime);

    // Getters
    CinematicCamera& getCamera() { return m_camera; }
    vio::IOManager& getIOManager() { return m_ioManager; }
    SoaState* getSoAState() const { return m_soaState; }

private:

    /// Initializes event delegates and InputManager
    void initInput();

    /// Initializes the rendering
    void initRenderPipeline();

    /// Initializes user interface
    void initUI();

    /// Loads a save file and prepares to play the game
    /// @param fileName: The name of the save file
    void loadGame(const nString& fileName);

    /// Makes a new save file and prepares to play the game
    /// @param fileName: The name of the save file
    void newGame(const nString& fileName);

    /// The function that runs on the update thread. It handles
    /// loading the planet in the background.
    void updateThreadFunc();

    /// Sets up iomanager and makes save file directories if they don't exist
    void initSaveIomanager(const vio::Path& savePath);

    /// Reloads the user interface
    void reloadUI();

    // --------------- Event handlers ---------------
    void onReloadSystem(Sender s, ui32 a);
    void onReloadShaders(Sender s, ui32 a);
    void onQuit(Sender s, ui32 a);
    // ----------------------------------------------

    const LoadScreen* m_loadScreen = nullptr;
    SoaState* m_soaState = nullptr;

    vio::IOManager m_ioManager; ///< Helper class for IO operations

    InputMapper* m_inputMapper = nullptr;

    CinematicCamera m_camera; ///< The camera that looks at the planet from space

    std::unique_ptr<MainMenuSystemViewer> m_mainMenuSystemViewer = nullptr;

    std::unique_ptr<SpaceSystemUpdater> m_spaceSystemUpdater = nullptr;

    std::thread* m_updateThread = nullptr; ///< The thread that updates the planet. Runs updateThreadFunc()
    volatile bool m_threadRunning; ///< True when the thread should be running

    MainMenuRenderPipeline m_renderPipeline; ///< This handles all rendering for the main menu
    MainMenuForm m_form; ///< The UI form
    vui::FormScriptEnvironment m_formEnv; ///< Form scripts

    // TODO: Remove to a client state
    vsound::Engine* m_engine;
    AmbienceLibrary* m_ambLibrary;
    AmbiencePlayer* m_ambPlayer;

    bool m_shouldReloadUI = false;
};

#endif // MAINMENUSCREEN_H_

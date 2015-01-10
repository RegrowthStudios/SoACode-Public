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

#include <Vorb/AwesomiumInterface.h>
#include <Vorb/IGameScreen.h>
#include <Vorb/Random.h>

#include "LoadMonitor.h"
#include "MainMenuAPI.h"
#include "MainMenuRenderPipeline.h"

class App;
class InputManager;
class LoadScreen;
class MainMenuSystemViewer;
class SoaState;
class TerrainMeshMessage;

class MainMenuScreen : public IAppScreen<App>
{
    friend class OnMainMenuReloadShadersKeyDown;
    friend class MainMenuAPI; ///< MainMenuAPI needs to talk directly to the MainMenuScreen
public:
    MainMenuScreen(const App* app, const LoadScreen* loadScreen);
    ~MainMenuScreen();

    virtual i32 getNextScreen() const;
    virtual i32 getPreviousScreen() const;

    virtual void build();
    virtual void destroy(const GameTime& gameTime);

    virtual void onEntry(const GameTime& gameTime);
    virtual void onExit(const GameTime& gameTime);

    virtual void onEvent(const SDL_Event& e);
    virtual void update(const GameTime& gameTime);
    virtual void draw(const GameTime& gameTime);

    // Getters
    CinematicCamera& getCamera() { return m_camera; }
    vio::IOManager& getIOManager() { return m_ioManager; }
    SoaState* getSoAState() const { return m_soaState; }

private:

    /// Initializes the rendering
    void initRenderPipeline();

    /// Loads a save file and prepares to play the game
    /// @param fileName: The name of the save file
    void loadGame(const nString& fileName);

    /// Makes a new save file and prepares to play the game
    /// @param fileName: The name of the save file
    void newGame(const nString& fileName);

    /// The function that runs on the update thread. It handles
    /// loading the planet in the background.
    void updateThreadFunc();

    /// Updates the dynamic clipping plane for the world camera
    void updateWorldCameraClip();

    const LoadScreen* m_loadScreen = nullptr;
    SoaState* m_soaState = nullptr;

    vui::AwesomiumInterface<MainMenuAPI> m_awesomiumInterface; ///< The user interface
    
    vio::IOManager m_ioManager; ///< Helper class for IO operations

    InputManager* m_inputManager = nullptr;

    CinematicCamera m_camera; ///< The camera that looks at the planet from space

    std::unique_ptr<MainMenuSystemViewer> m_mainMenuSystemViewer;

    std::thread* m_updateThread = nullptr; ///< The thread that updates the planet. Runs updateThreadFunc()
    volatile bool m_threadRunning; ///< True when the thread should be running

    IDelegate<ui32>* m_onReloadShadersKeyDown = nullptr;

    MainMenuRenderPipeline m_renderPipeline; ///< This handles all rendering for the main menu
};

#endif // MAINMENUSCREEN_H_
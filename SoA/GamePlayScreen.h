// 
//  GamePlayScreen.h
//  Seed Of Andromeda
//
//  Created by Ben Arnold on 17 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides the implementation for the main
//  gameplay screen. This will encompass all in game
//  behavior.
//

#pragma once

#ifndef GAMEPLAYSCREEN_H_
#define GAMEPLAYSCREEN_H_

#include <Vorb/Random.h>
#include <Vorb/ui/IGameScreen.h>

#include "AwesomiumInterface.h"
#include "GamePlayRenderPipeline.h"
#include "LoadMonitor.h"
#include "MessageManager.h"
#include "PDA.h"
#include "PauseMenu.h"
#include "SoaController.h"

class App;
class GameStartState;
class InputManager;
class MainMenuScreen;
class SoaState;
class SpriteBatch;
class SpriteFont;
class TerrainMeshMessage;
class GameSystem;
class GameSystemUpdater;

template<typename... Params>
class IDelegate;
class GamePlayScreenDelegate;
class OnPauseKeyDown;
class OnFlyKeyDown;
class OnGridKeyDown;
class OnReloadTexturesKeyDown;
class OnReloadShadersKeyDown;
class OnInventoryKeyDown;
class OnReloadUIKeyDown;
class OnHUDKeyDown;

// Each mode includes the previous mode
enum DevUiModes {
    DEVUIMODE_NONE,
    DEVUIMODE_CROSSHAIR,
    DEVUIMODE_HANDS,
    DEVUIMODE_FPS,
    DEVUIMODE_ALL
};

class GamePlayScreen : public IAppScreen<App> {
    friend class PdaAwesomiumAPI;
    friend class OnPauseKeyDown;
    friend class OnFlyKeyDown;
    friend class OnInventoryKeyDown;
    friend class OnReloadUIKeyDown;
    friend class OnReloadShadersKeyDown;
    friend class OnHUDKeyDown;
    friend class OnGridKeyDown;
    friend class PauseMenuAwesomiumAPI;
public:
    GamePlayScreen(const App* app, const MainMenuScreen* mainMenuScreen);
    ~GamePlayScreen();

    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;

    virtual void build() override;
    virtual void destroy(const GameTime& gameTime) override;

    virtual void onEntry(const GameTime& gameTime) override;
    virtual void onExit(const GameTime& gameTime) override;

    virtual void onEvent(const SDL_Event& e) override;
    virtual void update(const GameTime& gameTime) override;
    virtual void draw(const GameTime& gameTime) override;

    void unPause();

    // Getters
    i32 getWindowWidth() const;
    i32 getWindowHeight() const;

    bool isInGame() const {
        return (!m_pda.isOpen() && !m_pauseMenu.isOpen());
    }

private:

    /// Initializes the voxel world
    void initVoxels();

    /// Initializes the rendering
    void initRenderPipeline();

    /// Handles updating state based on input
    void handleInput();

    /// The function that runs on the update thread. It handles
    /// loading the planet in the background.
    void updateThreadFunc();

    /// Processes messages from the update->render thread
    void processMessages();

    /// Updates the dynamic clipping plane for the world camera
    void updateWorldCameraClip();

    /// Loads the player save file
    bool loadPlayerFile(Player* player);

    const MainMenuScreen* m_mainMenuScreen = nullptr;
    SoaState* m_soaState = nullptr;

    InputManager* m_inputManager = nullptr;

    PDA m_pda; ///< The PDA

    PauseMenu m_pauseMenu; ///< The Pause Menu

    bool m_inFocus; ///< true when the window is in focus

    SoaController controller;
    std::unique_ptr<GameSystemUpdater> m_gameSystemUpdater = nullptr;

    ChunkManager* m_chunkManager = nullptr;

    std::thread* m_updateThread = nullptr; ///< The thread that updates the planet. Runs updateThreadFunc()
    volatile bool m_threadRunning; ///< True when the thread should be running

    /// Delegates
    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    IDelegate<ui32>* m_onPauseKeyDown = nullptr;
    IDelegate<ui32>* m_onFlyKeyDown = nullptr;
    IDelegate<ui32>* m_onGridKeyDown = nullptr;
    IDelegate<ui32>* m_onReloadTexturesKeyDown = nullptr;
    IDelegate<ui32>* m_onReloadShadersKeyDown = nullptr;
    IDelegate<ui32>* m_onInventoryKeyDown = nullptr;
    IDelegate<ui32>* m_onReloadUIKeyDown = nullptr;
    IDelegate<ui32>* m_onHUDKeyDown = nullptr;
    IDelegate<ui32>* m_onNightVisionToggle = nullptr;
    IDelegate<ui32>* m_onNightVisionReload = nullptr;
    IDelegate<ui32>* m_onDrawMode = nullptr;
    GamePlayRenderPipeline m_renderPipeline; ///< This handles all rendering for the screen

    #define MESSAGES_PER_FRAME 300
    Message messageBuffer[MESSAGES_PER_FRAME];
};

#endif // GAMEPLAYSCREEN_H_
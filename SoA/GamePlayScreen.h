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

#include <Vorb/AwesomiumInterface.h>
#include <Vorb/Random.h>
#include <Vorb/IGameScreen.h>

#include "GamePlayRenderPipeline.h"
#include "LoadMonitor.h"
#include "MainMenuAPI.h"
#include "MessageManager.h"
#include "PDA.h"
#include "PauseMenu.h"

class App;
class SpriteBatch;
class SpriteFont;
struct TerrainMeshMessage;

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
    CTOR_APP_SCREEN_DECL(GamePlayScreen, App);

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
        return (!_pda.isOpen() && !_pauseMenu.isOpen());
    }


private:

    /// Initializes the rendering
    void initRenderPipeline();

    /// Handles updating state based on input
    void handleInput();

    /// Updates the player
    void updatePlayer();

    /// The function that runs on the update thread. It handles
    /// loading the planet in the background.
    void updateThreadFunc();

    /// Processes messages from the update->render thread
    void processMessages();

    /// Updates the dynamic clipping plane for the world camera
    void updateWorldCameraClip();

    Player* _player; ///< The current player

    PDA _pda; ///< The PDA

    PauseMenu _pauseMenu; ///< The Pause Menu

    bool _inFocus; ///< true when the window is in focus

    // TODO(Ben): Should they be stored here?
    //Camera _voxelCamera; ///< The camera for rendering the voxels
    //Camera _planetCamera; ///< The camera for rendering the planet

    std::thread* _updateThread; ///< The thread that updates the planet. Runs updateThreadFunc()
    volatile bool _threadRunning; ///< True when the thread should be running

    /// Delegates
    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    IDelegate<ui32>* _onPauseKeyDown;
    IDelegate<ui32>* _onFlyKeyDown;
    IDelegate<ui32>* _onGridKeyDown;
    IDelegate<ui32>* _onReloadTexturesKeyDown;
    IDelegate<ui32>* _onReloadShadersKeyDown;
    IDelegate<ui32>* _onInventoryKeyDown;
    IDelegate<ui32>* _onReloadUIKeyDown;
    IDelegate<ui32>* _onHUDKeyDown;
    IDelegate<ui32>* _onNightVisionToggle;
    IDelegate<ui32>* _onNightVisionReload;
    IDelegate<ui32>* m_onDrawMode;
    GamePlayRenderPipeline _renderPipeline; ///< This handles all rendering for the screen

    #define MESSAGES_PER_FRAME 300
    Message messageBuffer[MESSAGES_PER_FRAME];
};

#endif // GAMEPLAYSCREEN_H_
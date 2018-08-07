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

#include "GameplayRenderer.h"
#include "LoadMonitor.h"
#include "MTRenderStateManager.h"
#include "PDA.h"
#include "PauseMenu.h"
#include "SoaController.h"
#include "DevConsoleView.h"
#include "SpaceSystemUpdater.h"

class App;
class GameStartState;
class GameSystem;
class GameSystemUpdater;
class InputMapper;
class MainMenuScreen;
struct SoaState;
class SpriteBatch;
class SpriteFont;
class TerrainMeshMessage;

template<typename... Params>
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

class GameplayScreen : public vui::IAppScreen<App> {
    friend class GameplayRenderer;
    friend class GameplayLoadScreen;
public:
    GameplayScreen(const App* app, const MainMenuScreen* mainMenuScreen);
    ~GameplayScreen();

    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;

    virtual void build() override;
    virtual void destroy(const vui::GameTime& gameTime) override;

    virtual void onEntry(const vui::GameTime& gameTime) override;
    virtual void onExit(const vui::GameTime& gameTime) override;

    virtual void update(const vui::GameTime& gameTime) override;
    virtual void draw(const vui::GameTime& gameTime) override;

    void unPause();

    // Getters
    i32 getWindowWidth() const;
    i32 getWindowHeight() const;

    bool isInGame() const {
        return (!m_pda.isOpen() && !m_pauseMenu.isOpen());
    }

private:
    /// Initializes event delegates and InputManager
    void initInput();

    /// Initializes dev console events
    void initConsole();

    /// Initializes the rendering
    void initRenderPipeline();

    /// The function that runs on the update thread. It handles
    /// loading the planet in the background.
    void updateThreadFunc();
    
    /// Updates the Entity component system
    void updateECS();

    /// Updates multi-threaded render state
    void updateMTRenderState();

    // --------------- Event handlers ---------------
    void onReloadShaders(Sender s, ui32 a);
    void onReloadTarget(Sender s, ui32 a);
    void onQuit(Sender s, ui32 a);
    void onToggleWireframe(Sender s, ui32 i);
    void onWindowClose(Sender s);
    // ----------------------------------------------

    const MainMenuScreen* m_mainMenuScreen;
    SoaState* m_soaState;

    InputMapper* m_inputMapper;

    PDA m_pda; ///< The PDA

    PauseMenu m_pauseMenu; ///< The Pause Menu

    DevConsoleView m_devConsoleView;

    SoaController controller;
    std::unique_ptr<SpaceSystemUpdater> m_spaceSystemUpdater;
    std::unique_ptr<GameSystemUpdater> m_gameSystemUpdater;

    std::thread* m_updateThread; ///< The thread that updates the planet. Runs updateThreadFunc()
    volatile bool m_threadRunning; ///< True when the thread should be running

    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    GameplayRenderer m_renderer; ///< This handles all rendering for the screen

    MTRenderStateManager m_renderStateManager; ///< Manages the triple buffered render state
    const MTRenderState* m_prevRenderState; ///< Render state use for previous draw

    std::mutex m_reloadLock;
    bool m_shouldReloadTarget;
    bool m_shouldReloadShaders;
    bool m_shouldToggleDevConsole;
};

#endif // GAMEPLAYSCREEN_H_

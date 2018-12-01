// 
//  MainMenuScreen.h
//  Seed Of Andromeda
//
//  Created by Ben Arnold on 17 Oct 2014
//  Copyright 2014 Regrowth Studios
//  MIT License
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
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/ui/IGameScreen.h>

#include "LoadMonitor.h"
#include "MainMenuRenderer.h"
#include "MainMenuScriptedUI.h"
#include "Camera.h"
#include "SpaceSystemUpdater.h"

class App;

class InputMapper;
class MainMenuLoadScreen;
class MainMenuSystemViewer;
struct CommonState;
struct SoaState;

DECL_VSOUND(class Engine)
DECL_VUI(struct WindowResizeEvent);
class AmbienceLibrary;
class AmbiencePlayer;

class MainMenuScreen : public vui::IAppScreen<App> {
    friend class MainMenuScriptedUI;
    friend class MainMenuRenderer;
    friend class MainMenuLoadScreen; // So it can load our assets
    friend class GameplayLoadScreen; // So it can use our renderer
public:
    MainMenuScreen(const App* app, CommonState* state);
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

    /// Sets up iomanager and makes save file directories if they don't exist
    void initSaveIomanager(const vio::Path& savePath);

    /// Reloads the user interface
    void reloadUI();

    /// Cycles the draw mode for wireframe
    void cycleDrawMode();

    // --------------- Event handlers ---------------
    void onReloadSystem(Sender s, ui32 a);
    void onReloadShaders(Sender s, ui32 a);
    void onQuit(Sender s, ui32 a);
    void onWindowResize(Sender s, const vui::WindowResizeEvent& e);
    void onWindowClose(Sender s);
    void onOptionsChange(Sender s);
    void onToggleUI(Sender s, ui32 i);
    void onToggleWireframe(Sender s, ui32 i);
    // ----------------------------------------------

    CommonState* m_commonState = nullptr;
    SoaState* m_soaState = nullptr;

    vio::IOManager m_ioManager; ///< Helper class for IO operations

    InputMapper* m_inputMapper = nullptr;

    std::unique_ptr<SpaceSystemUpdater> m_spaceSystemUpdater = nullptr;

    std::thread* m_updateThread = nullptr; ///< The thread that updates the planet. Runs updateThreadFunc()
    volatile bool m_threadRunning; ///< True when the thread should be running

    MainMenuRenderer m_renderer; ///< This handles all rendering for the main menu
    MainMenuScriptedUI m_ui; ///< The UI form
    vg::SpriteFont m_formFont; ///< The UI font

    MainMenuSystemViewer* m_mainMenuSystemViewer = nullptr;

    AmbienceLibrary* m_ambLibrary = nullptr;
    AmbiencePlayer* m_ambPlayer = nullptr;
    vui::GameWindow* m_window = nullptr;
    bool m_uiEnabled = true;

    bool m_isFullscreen = false;
    bool m_isBorderless = false;

    bool m_newGameClicked = false;
    bool m_shouldReloadUI = false;
};

#endif // MAINMENUSCREEN_H_

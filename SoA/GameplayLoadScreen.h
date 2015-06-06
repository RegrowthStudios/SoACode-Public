///
/// GameplayLoadScreen.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 6 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Load screen for going into a game world
///

#pragma once

#ifndef GameplayLoadScreen_h__
#define GameplayLoadScreen_h__

#include <Vorb/RPC.h>
#include <Vorb/Random.h>
#include <Vorb/graphics/Texture.h>
#include <Vorb/script/Environment.h>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/ui/InputDispatcher.h>

#include "LoadMonitor.h"

class App;
class MainMenuScreen;
class GameplayScreen;
struct CommonState;

#define VORB_NUM_TEXTURES 7
#define REGROWTH_NUM_TEXTURES 2

class GameplayLoadScreen : public vui::IAppScreen < App > {
public:
    GameplayLoadScreen(const App* app, CommonState* state, MainMenuScreen* mainMenuScreen, GameplayScreen* gameplayScreen);

    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;

    virtual void build() override;
    virtual void destroy(const vui::GameTime& gameTime) override;

    virtual void onEntry(const vui::GameTime& gameTime) override;
    virtual void onExit(const vui::GameTime& gameTime) override;

    virtual void update(const vui::GameTime& gameTime) override;
    virtual void draw(const vui::GameTime& gameTime) override;
private:
    void addLoadTask(const nString& name, ILoadTask* task);
    // Game state
    CommonState* m_commonState = nullptr;
    MainMenuScreen* m_mainMenuScreen = nullptr;
    GameplayScreen* m_gameplayScreen = nullptr;

    // Loading Tasks
    LoadMonitor m_monitor;
    std::vector<ILoadTask*> m_loadTasks;

    vcore::RPCManager m_glrpc; ///< Handles cross-thread OpenGL calls
};

#endif // GameplayLoadScreen_h__
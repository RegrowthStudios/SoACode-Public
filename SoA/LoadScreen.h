#pragma once

#include <Vorb/ui/IGameScreen.h>
#include <Vorb/Random.h>
#include <Vorb/RPC.h>
#include <Vorb/VorbPreDecl.inl>

#include "LoadMonitor.h"
#include "LoadBar.h"

class App;
class MainMenuScreen;
struct CommonState;
DECL_VG(class SpriteBatch; class SpriteFont);

class LoadScreen : public vui::IAppScreen<App> {
public:
    LoadScreen(const App* app, CommonState* state, MainMenuScreen* mainMenuScreen);

    virtual i32 getNextScreen() const;
    virtual i32 getPreviousScreen() const;

    virtual void build();
    virtual void destroy(const vui::GameTime& gameTime);

    virtual void onEntry(const vui::GameTime& gameTime);
    virtual void onExit(const vui::GameTime& gameTime);

    virtual void update(const vui::GameTime& gameTime);
    virtual void draw(const vui::GameTime& gameTime);

private:
    void addLoadTask(const nString& name, const cString loadText, ILoadTask* task);

    // Game state
    CommonState* m_commonState = nullptr;
    MainMenuScreen* m_mainMenuScreen = nullptr;

    // Visualization Of Loading Tasks
    std::vector<LoadBar> m_loadBars;
    vg::SpriteBatch* m_sb;
    vg::SpriteFont* m_sf;

    // Loading Tasks
    LoadMonitor m_monitor;
    std::vector<ILoadTask*> m_loadTasks;

    vcore::RPCManager m_glrpc; ///< Handles cross-thread OpenGL calls
};
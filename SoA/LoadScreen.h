#pragma once

#include <Vorb/RPC.h>
#include <Vorb/Random.h>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/Texture.h>
#include <Vorb/script/Environment.h>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/ui/InputDispatcher.h>

#include "LoadMonitor.h"
#include "LoadBar.h"

class App;
class MainMenuScreen;
struct CommonState;
DECL_VG(class SpriteBatch; class SpriteFont);

#define VORB_NUM_TEXTURES 7

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
    void onKeyPress(Sender, const vui::KeyEvent& e);
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

    // Vars for vorb logo
    f64 m_timer, m_screenDuration;
    bool m_isSkipDetected;
    vg::Texture m_textures[VORB_NUM_TEXTURES]; ///< 7 textures for the letters and cube
    vscript::Environment m_env;
    vscript::RFunction<f32v2> m_fUpdatePosition; ///< f32v2 (f64 totalTime, nString texture)
    vscript::RFunction<f32v4> m_fUpdateColor; ///< f32v4 (f64 totalTime, nString texture)

    vcore::RPCManager m_glrpc; ///< Handles cross-thread OpenGL calls
};
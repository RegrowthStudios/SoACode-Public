#pragma once

#include <Vorb/vorb_rpc.h>
#include <Vorb/Random.h>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/Texture.h>
// #include <Vorb/script/Environment.h>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/ui/InputDispatcher.h>

#include "LoadMonitor.h"
#include "LoadBar.h"

class App;
class MainMenuScreen;
struct CommonState;
DECL_VG(class SpriteBatch; class SpriteFont);

#define VORB_NUM_TEXTURES 7
#define REGROWTH_NUM_TEXTURES 2

class MainMenuLoadScreen : public vui::IAppScreen<App> {
public:
    MainMenuLoadScreen(const App* app, CommonState* state, MainMenuScreen* mainMenuScreen);

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
    vg::SpriteBatch* m_sb = nullptr;
    vg::SpriteFont* m_sf = nullptr;

    // Loading Tasks
    LoadMonitor m_monitor;
    std::vector<ILoadTask*> m_loadTasks;

    // Vars for logos
    f64 m_timer, m_vorbScreenDuration, m_regrowthScreenDuration;
    f32 m_regrowthScale;
    bool m_isSkipDetected;
    vg::Texture m_vorbTextures[VORB_NUM_TEXTURES]; ///< 7 textures for the letters and cube
    vg::Texture m_regrowthTextures[REGROWTH_NUM_TEXTURES]; ///< 2 Textures for Regrowth Studios
    // vscript::Environment m_env;
    // vscript::RFunction<f32v2> m_fUpdateVorbPosition; ///< f32v2 (f64 totalTime, nString texture)
    // vscript::RFunction<f32v4> m_fUpdateVorbColor; ///< f32v4 (f64 totalTime, nString texture)
    // vscript::RFunction<f32v4> m_fUpdateVorbBackColor;///< f32v4 (f64 totalTime)
    // vscript::RFunction<f32v2> m_fUpdateRegrowthPosition; ///< f32v2 (f64 totalTime, nString texture)
    // vscript::RFunction<f32v4> m_fUpdateRegrowthColor; ///< f32v4 (f64 totalTime, nString texture)
    // vscript::RFunction<f32v4> m_fUpdateRegrowthBackColor;///< f32v4 (f64 totalTime)

    bool m_isOnVorb = true;
};
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/ui/IGameScreen.h>

#include <iostream>

#include <Vorb/Event.hpp>
#include <Vorb/script/lua/Environment.h>
#include <Vorb/ui/ScriptedUI.h>

#include "CommonState.h"

class InputMapper;

class TestScriptScreen : public vui::IAppScreen<App> {
public:
    TestScriptScreen(const App* app, CommonState* state);

    i32 getNextScreen() const override;
    i32 getPreviousScreen() const override;

    void build() override;
    void destroy(const vui::GameTime&) override;

    void onEntry(const vui::GameTime&) override;
    void onExit(const vui::GameTime&) override;

    void update(const vui::GameTime&) override;
    void draw(const vui::GameTime&) override;

    Event<nString> onMessage;

    static void printMessage(nString message) {
        std::cout << message << std::endl;
    }

    static i32 add(i32 a, i32 b) {
        return a + b;
    }
private:
    CommonState*               m_commonState;
    vscript::lua::Environment  m_env;
    vui::ScriptedUI<vscript::lua::Environment> m_ui;
    vg::SpriteBatch m_sb;
    vg::SpriteFont m_font;
    vio::IOManager m_iom;
};

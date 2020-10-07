#include "stdafx.h"
#include "TestScriptScreen.h"

#include <Vorb/graphics/script/Graphics.hpp>
#include <Vorb/io/File.h>
#include <Vorb/io/Path.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/ui/script/ViewScriptContext.hpp>

#include "App.h"
#include "InputMapper.h"
#include "Inputs.h"

TestScriptScreen::TestScriptScreen(const App* app, CommonState* state) : IAppScreen<App>(app),
    m_commonState(state)
{
    // Empty
}

i32 TestScriptScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestScriptScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestScriptScreen::build() {
    // Empty
}

void TestScriptScreen::destroy(const vui::GameTime&) {
    // Empty
}

void TestScriptScreen::onEntry(const vui::GameTime&) {
    m_textureCache.init(&m_iom);
    m_fontCache.init(&m_iom);

    m_sb.init();
    m_font.init("Fonts/orbitron_bold-webfont.ttf", 32);

    auto builder = makeFunctor([&](vscript::lua::Environment* env) {
        env->setNamespaces("onMessage");
        env->addCDelegate("subscribe", makeFunctor([&, env](nString name) {
            onMessage.add(env->template getScriptDelegate<void, Sender, nString>(name), true);
        }));
        env->addCDelegate("unsubscribe", makeFunctor([&, env](nString name) {
            onMessage.remove(env->template getScriptDelegate<void, Sender, nString>(name, false));
        }));
        env->setNamespaces();

        env->addCDelegate("C_Print", makeDelegate(&TestScriptScreen::printMessage));
        env->addCDelegate("C_Add",   makeDelegate(&TestScriptScreen::add));

        vg::GraphicsScriptContext::injectInto(env, &m_fontCache, &m_textureCache);

        vui::UIScriptContext::injectInto(env);

        vui::ViewScriptContext::injectInto(env, m_commonState->window, &m_textureCache, m_widgets);
    });
    m_commonState->scriptEnvRegistry->createGroup("test", &builder);

    m_env = m_commonState->scriptEnvRegistry->getScriptEnv("test");

    m_env->run(vio::Path("test.lua"));

    auto del = m_env->getScriptDelegate<void, Sender, nString>("doPrint");

    onMessage("Hello, World!");
    onMessage("Hello, World!");

    del.invoke(nullptr, "Hello, Waldo!");

    m_ui.init(this, m_commonState->window, &m_iom, &m_textureCache, &m_fontCache, &m_sb, m_commonState->scriptEnvRegistry, "test");

    auto view = m_ui.makeViewFromScript("TestView", 1, "ui_test.lua");

    view->enable();
}

void TestScriptScreen::onExit(const vui::GameTime&) {
    // Empty
}

void TestScriptScreen::update(const vui::GameTime& dt) {
    m_ui.update((f32)dt.elapsed);
}

void TestScriptScreen::draw(const vui::GameTime&) {
    glClear(GL_COLOR_BUFFER_BIT);

    m_ui.draw();
}
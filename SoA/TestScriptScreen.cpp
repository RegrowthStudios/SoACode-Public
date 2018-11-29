#include "stdafx.h"
#include "TestScriptScreen.h"

#include <Vorb/io/File.h>
#include <Vorb/io/Path.h>
#include <Vorb/ui/InputDispatcher.h>

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
    m_env.init();

    m_env.registerEvent("onMessage", &onMessage);

    m_env.addCDelegate("C_Print", makeDelegate(&TestScriptScreen::printMessage));
    m_env.addCDelegate("C_Add",   makeDelegate(&TestScriptScreen::add));

    m_env.run(vio::Path("test.lua"));

    auto del = m_env.getScriptDelegate<void, void*, std::string>("bleh");

    del->invoke(nullptr, "Hello, Waldo!");

    onMessage("Hello, World!");

    m_sb.init();
    m_font.init("Fonts/orbitron_bold-webfont.ttf", 32);

    m_ui.init(m_commonState->window, nullptr, &m_font, &m_sb);

    auto view = m_ui.makeView("TestView", 1);
    view.scriptEnv->getEnv()->addCDelegate("C_Print", makeDelegate(&TestScriptScreen::printMessage));
    view.scriptEnv->run("ui_test.lua");
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
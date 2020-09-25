#include "stdafx.h"
#include "TestModScreen.h"

#include <Vorb/mod/ModEnvironment.h>
#include <Vorb/script/lua/Environment.h>

#include "App.h"

TestModScreen::TestModScreen(const App* app, CommonState* state) : IAppScreen<App>(app),
    m_commonState(state)
{
    // Empty
}

i32 TestModScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestModScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestModScreen::build() {
    // Empty
}

void TestModScreen::destroy(const vui::GameTime&) {
    // Empty
}

void TestModScreen::onEntry(const vui::GameTime&) {
    m_modEnv = new vmod::ModEnvironment<vscript::lua::Environment>();

    m_modEnv->init("Mods", "LoadOrders");

    const vmod::ModBase* mod = m_modEnv->getActiveMod("test");

    std::cout << mod->getModMetadata().author << std::endl;
}

void TestModScreen::onExit(const vui::GameTime&) {
    // Empty
}

void TestModScreen::update(const vui::GameTime& dt) {
    // Empty
}

void TestModScreen::draw(const vui::GameTime&) {
    glClear(GL_COLOR_BUFFER_BIT);
}
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

    auto profiles = m_modEnv->getLoadOrderManager().getAllLoadOrderProfiles();
    
    std::cout << "Load Order Profiles:" << std::endl;
    for (auto& profile : profiles) {
        std::cout << "    - name:     " << profile.name << std::endl;
        std::cout << "    - created:  " << profile.createdTimestamp << std::endl;
        std::cout << "    - modified: " << profile.lastModifiedTimestamp << std::endl;
            std::cout << "    - mods: " << std::endl;
        for (auto& mod : profile.mods) {
            std::cout << "        - name: " << mod << std::endl;
        }
    }

    vmod::ModBasePtrs mods = m_modEnv->getMods();

    std::cout << "All Mods:" << std::endl;
    for (auto& mod : mods) {
        std::cout << "    - name:   " << mod->getModMetadata().name << std::endl;
        std::cout << "      author: " << mod->getModMetadata().author << std::endl;
    }

    vmod::ModBaseConstPtrs activeMods = m_modEnv->getActiveMods();

    std::cout << "Active Mods:" << std::endl;
    for (auto& mod : activeMods) {
        std::cout << "    - name:   " << mod->getModMetadata().name << std::endl;
        std::cout << "      author: " << mod->getModMetadata().author << std::endl;
    }
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
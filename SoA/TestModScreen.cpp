#include "stdafx.h"
#include "TestModScreen.h"

#include <Vorb/mod/DataAssetIOManager.h>
#include <Vorb/mod/Merge.h>
#include <Vorb/mod/ModEnvironment.h>
#include <Vorb/script/lua/Environment.h>

#include "App.h"

class TestMerge : public vmod::MergeBase {
protected:
    /*!
        * \brief The merge strategy with which to merge the data points together.
        *
        * \param newData: The new data point to merge.
        * \param existingData: The data so far merged.
        *
        * \return True if the merge completes, false otherwise.
        */
    virtual bool performMerge(const nString& newData, void* existingData) override {
        nString* existing = static_cast<nString*>(existingData);

        *existing += "\n" + newData;

        return true;
    }
};

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
    vmod::DataAssetIOManager::setVanillaDataDir("Data");
    vmod::DataAssetIOManager::setGlobalModDirectory("Mods");

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

    vmod::DataAssetIOManager iom;
    iom.setModEnvironment(m_modEnv);

    nString message;
    if (iom.readFileToString("message.txt", message)) {
        std::cout << message << std::endl;
    }
    if (iom.readModFileToString("all.txt", message, m_modEnv->getMod("test"))) {
        std::cout << message << std::endl;
    }
    if (iom.readVanillaFileToString("options.ini", message)) {
        std::cout << message << std::endl;
    }

    std::vector<nString> messages;
    if (iom.readEachFileToString("message.txt", messages)) {
        for (auto& msg : messages) {
            std::cout << msg << std::endl;
        }
    }

    TestMerge myMerger;
    myMerger.init(&iom);

    nString merged;
    myMerger.mergeFiles("message.txt", (void*)&merged);

    std::cout << "Result of merging 'message.txt':" << std::endl << merged << std::endl;
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
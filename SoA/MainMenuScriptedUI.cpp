#include "stdafx.h"
#include "MainMenuScriptedUI.h"
#include "SoaEngine.h"
#include "MainMenuScreen.h"

#include <Vorb/ui/FormScriptEnvironment.h>
#include <Vorb/script/Environment.h>
#include <Vorb/Events.hpp>
#include <Vorb/ui/Form.h>
#include <Vorb/ui/KeyStrings.h>

MainMenuScriptedUI::MainMenuScriptedUI() {
    // Empty
}

MainMenuScriptedUI::~MainMenuScriptedUI() {
    // Empty
}

void MainMenuScriptedUI::init(const nString& startFormPath, vui::IGameScreen* ownerScreen, const vui::GameWindow* window, const f32v4& destRect, vg::SpriteFont* defaultFont /*= nullptr*/) {
    m_inputMapper = ((MainMenuScreen*)ownerScreen)->m_inputMapper; 
    ScriptedUI::init(startFormPath, ownerScreen, window, destRect, defaultFont); 
}


void MainMenuScriptedUI::registerScriptValues(vui::FormScriptEnvironment* newFormEnv) {
    vui::ScriptedUI::registerScriptValues(newFormEnv);
    vscript::Environment* env = newFormEnv->getEnv();

    SoaEngine::optionsController.registerScripting(env);

    // Controls menu stuff
    env->setNamespaces("Controls");
    env->addCRDelegate("size", makeRDelegate(*this, &MainMenuScriptedUI::getNumInputs));
    env->addCRDelegate("getInput", makeRDelegate(*this, &MainMenuScriptedUI::getInput));
    env->addCRDelegate("getKey", makeRDelegate(*this, &MainMenuScriptedUI::getKey));
    env->addCRDelegate("getDefaultKey", makeRDelegate(*this, &MainMenuScriptedUI::getDefaultKey));
    env->addCRDelegate("getKeyString", makeRDelegate(*this, &MainMenuScriptedUI::getKeyString));
    env->addCRDelegate("getDefaultKeyString", makeRDelegate(*this, &MainMenuScriptedUI::getDefaultKeyString));
    env->addCRDelegate("getName", makeRDelegate(*this, &MainMenuScriptedUI::getName));
    env->setNamespaces();
}

size_t MainMenuScriptedUI::getNumInputs() {
    return m_inputMapper->getInputLookup().size();
}

InputMapper::InputID MainMenuScriptedUI::getInput(int index) {
    // This is slow, but that is ok.
    auto& it = m_inputMapper->getInputLookup().begin();
    std::advance(it, index);
    return it->second;
}

VirtualKey MainMenuScriptedUI::getKey(InputMapper::InputID id) {
    return m_inputMapper->getKey(id);
}

VirtualKey MainMenuScriptedUI::getDefaultKey(InputMapper::InputID id) {
    return m_inputMapper->get(id).defaultKey;
}

nString MainMenuScriptedUI::getKeyString(InputMapper::InputID id) {
    return nString(VirtualKeyStrings[m_inputMapper->getKey(id)]);
}

nString MainMenuScriptedUI::getDefaultKeyString(InputMapper::InputID id) {
    return nString(VirtualKeyStrings[m_inputMapper->get(id).defaultKey]);
}

nString MainMenuScriptedUI::getName(InputMapper::InputID id) {
    return m_inputMapper->get(id).name;
}


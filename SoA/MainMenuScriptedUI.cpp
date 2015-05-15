#include "stdafx.h"
#include "MainMenuScriptedUI.h"
#include "SoaEngine.h"

#include <Vorb/ui/FormScriptEnvironment.h>
#include <Vorb/script/Environment.h>
#include <Vorb/Events.hpp>
#include <Vorb/ui/Form.h>

MainMenuScriptedUI::MainMenuScriptedUI() {
    // Empty
}

MainMenuScriptedUI::~MainMenuScriptedUI() {
    // Empty
}

void MainMenuScriptedUI::registerScriptValues(vui::FormScriptEnvironment* newFormEnv) {
    vui::ScriptedUI::registerScriptValues(newFormEnv);
    vscript::Environment* env = newFormEnv->getEnv();

    SoaEngine::optionsController.registerScripting(env);
}

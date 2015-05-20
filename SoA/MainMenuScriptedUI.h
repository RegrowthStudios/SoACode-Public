///
/// MainMenuScriptedUI.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 13 May 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Custom main menu UI
///

#pragma once

#ifndef MainMenuScriptedUI_h__
#define MainMenuScriptedUI_h__

#include <Vorb/ui/ScriptedUI.h>
#include "InputMapper.h"

class MainMenuScreen;

class MainMenuScriptedUI : public vui::ScriptedUI {
public:
    MainMenuScriptedUI();
    ~MainMenuScriptedUI();

    void init(const nString& startFormPath, vui::IGameScreen* ownerScreen,
              const vui::GameWindow* window, const f32v4& destRect,
              vg::SpriteFont* defaultFont = nullptr) override;

protected:
    virtual void registerScriptValues(vui::FormScriptEnvironment* newFormEnv) override;
    // LUA funcs for controls
    size_t getNumInputs();
    InputMapper::InputID getInput(int index);
    VirtualKey getKey(InputMapper::InputID id);
    VirtualKey getDefaultKey(InputMapper::InputID id);
    nString getKeyString(InputMapper::InputID id);
    nString getDefaultKeyString(InputMapper::InputID id);
    nString getName(InputMapper::InputID id);
    void onExit(int code);

    InputMapper* m_inputMapper = nullptr;
};

#endif // MainMenuScriptedUI_h__

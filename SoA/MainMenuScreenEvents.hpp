///
/// MainMenuScreenEvents.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 14 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// This file provides the event delegates for the MainMenuScreen
///

#pragma once

#ifndef MainMenuScreenEvents_h__
#define MainMenuScreenEvents_h__

#include <Vorb/Events.hpp>

#include "MainMenuScreen.h"
#include "LoadTaskShaders.h"

class MainMenuScreenDelegate : IDelegate<ui32> {
public:
    MainMenuScreenDelegate() {}
    MainMenuScreenDelegate(MainMenuScreen* screen) : _screen(screen) {}

    virtual void invoke(Sender sender, ui32 key) override = 0;
protected:
    MainMenuScreen* _screen;
};

/// Delegate to handle when the Reload Shaders key down event
class OnMainMenuReloadShadersKeyDown : MainMenuScreenDelegate {
public:
    OnMainMenuReloadShadersKeyDown() {}
    OnMainMenuReloadShadersKeyDown(MainMenuScreen* screen) : MainMenuScreenDelegate(screen) {}

    virtual void invoke(Sender sender, ui32 key) override {
     /*   GameManager::glProgramManager->destroy();
        LoadTaskShaders shaderTask(nullptr);
        shaderTask.load();
        _screen->m_renderPipeline.destroy();
        _screen->initRenderPipeline();*/
    }
};

#endif // MainMenuScreenEvents_h__
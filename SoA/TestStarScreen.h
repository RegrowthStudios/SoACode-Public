///
/// TestStarScreen.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 9 Apr 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Test screen for star renderer
///

#pragma once

#ifndef TestStarScreen_h__
#define TestStarScreen_h__

#include "StarComponentRenderer.h"
#include "SpaceSystemComponents.h"
#include <Vorb/Events.hpp>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>

class TestStarScreen : public vui::IGameScreen {
public:
    /************************************************************************/
    /* IGameScreen functionality                                            */
    /************************************************************************/
    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;
    virtual void build() override;
    virtual void destroy(const vui::GameTime& gameTime) override;
    virtual void onEntry(const vui::GameTime& gameTime) override;
    virtual void onExit(const vui::GameTime& gameTime) override;
    virtual void update(const vui::GameTime& gameTime) override;
    virtual void draw(const vui::GameTime& gameTime) override;

private:
    StarComponentRenderer m_starRenderer;
    f32v3 m_eyePos;
    StarComponent m_sCmp;

    AutoDelegatePool m_hooks;
};

#endif // TestStarScreen_h__

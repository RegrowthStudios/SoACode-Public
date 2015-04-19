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
#include "Camera.h"
#include <Vorb/Events.hpp>
#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/ui/IGameScreen.h>

class HdrRenderStage;
class SoaState;
class App;

class TestStarScreen : public vui::IAppScreen<App> {
public:
    TestStarScreen(const App* app, const SoaState* state);
    ~TestStarScreen();
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
    const f64 STAR_RADIUS = 1693000.0 / 2.0;

    StarComponentRenderer* m_starRenderer = nullptr;
    f64v3 m_eyePos;
    f64 m_eyeDist = STAR_RADIUS;
    StarComponent m_sCmp;
    bool m_isUpDown = false;
    bool m_isDownDown = false;
    AutoDelegatePool m_hooks;
    vg::SpriteBatch m_spriteBatch;
    vg::SpriteFont m_spriteFont;
    HdrRenderStage* m_hdr = nullptr;
    vg::FullQuadVBO m_quad;
    Camera m_camera;
    vg::GLRenderTarget* m_hdrFrameBuffer = nullptr;
    const SoaState* m_soaState = nullptr;
    bool m_isHDR = true;
    bool m_isGlow = true;
    bool m_is1Pressed = false;
    bool m_is2Pressed = false;
    bool m_is3Pressed = false;
    bool m_is4Pressed = false;
};

#endif // TestStarScreen_h__

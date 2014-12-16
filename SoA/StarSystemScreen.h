///
/// StarSystemScreen.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 16 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef StarSystemScreen_h__
#define StarSystemScreen_h__

#include "IGameScreen.h"
#include <GLProgram.h>

#include "FullQuadVBO.h"
#include "RTSwapChain.hpp"
#include "GLRenderTarget.h"
#include "HdrRenderStage.h"
#include "Camera.h"
#include "MouseInputDispatcher.h"
#include "KeyboardEventDispatcher.h"

class App;
class SkyboxRenderStage;
class SpaceSystemRenderStage;

class StarSystemScreen : public IAppScreen<App> {
public:
    CTOR_APP_SCREEN_INL(StarSystemScreen, App) {
    }

    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;
    virtual void build() override;
    virtual void destroy(const GameTime& gameTime) override;
    virtual void onEntry(const GameTime& gameTime) override;
    virtual void onExit(const GameTime& gameTime) override;
    virtual void onEvent(const SDL_Event& e) override;
    virtual void update(const GameTime& gameTime) override;
    virtual void draw(const GameTime& gameTime) override;
private:
    void onMouseButtonDown(void* sender, const vui::MouseButtonEvent& e);
    void onMouseButtonUp(void* sender, const vui::MouseButtonEvent& e);
    void onMouseWheel(void* sender, const vui::MouseWheelEvent& e);
    void onMouseMotion(void* sender, const vui::MouseMotionEvent& e);
    void onKeyDown(void* sender, const vui::KeyEvent& e);

    bool mouseButtons[2];

    vg::GLProgram m_terrainProgram;
    CinematicCamera m_camera;
    vg::FullQuadVBO _quad; ///< Quad used for post-processing
    vg::RTSwapChain<2>* _swapChain = nullptr; ///< Swap chain of framebuffers used for post-processing

    vg::GLRenderTarget* _hdrFrameBuffer = nullptr; ///< Framebuffer needed for the HDR rendering
    HdrRenderStage* _hdrRenderStage = nullptr; ///< Renders HDR post-processing
    SkyboxRenderStage* _skyboxRenderStage = nullptr; ///< Renders the skybox
    SpaceSystemRenderStage* m_spaceSystemRenderStage = nullptr;

    ui32v4 _viewport; ///< Viewport to draw to
};

#endif // StarSystemScreen_h__

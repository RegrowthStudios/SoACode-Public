/// 
///  DevHudRenderStage.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 2 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file provides the render stage for
///  drawing the dev/debug Hud.
///

#pragma once

#ifndef DevHudRenderStage_h__
#define DevHudRenderStage_h__

#include <Vorb/graphics/IRenderStage.h>
#include <Vorb/VorbPreDecl.inl>

DECL_VG(class SpriteBatch;
        class SpriteFont)

class App;

class DevHudRenderStage : public vg::IRenderStage{
public:
    DevHudRenderStage(const cString fontPath, i32 fontSize,
                      const App* app, const f32v2& windowDims);
    ~DevHudRenderStage();

    /// Draws the render stage
    virtual void draw() override;

    /// Cycles the Hud mode
    /// @param offset: How much to offset the current mode
    void cycleMode(int offset = 1);

    // Each mode includes the previous mode
    enum class DevUiModes {
        NONE = 0,
        CROSSHAIR = 1,
        HANDS = 2,
        FPS = 3,
        POSITION = 4,
        LAST = POSITION // Make sure LAST is always last
    };

private:
    void drawCrosshair();
    void drawHands();
    void drawFps();
    void drawPosition();

    vg::SpriteBatch* _spriteBatch; ///< For rendering 2D sprites
    vg::SpriteFont* _spriteFont; ///< Font used by spritebatch
    DevUiModes _mode; ///< The mode for rendering
    f32v2 _windowDims; ///< Dimensions of the window
    const App* _app; ///< Handle to the app
    int _fontHeight; ///< Height of the spriteFont
    int _yOffset; ///< Y offset accumulator
};

#endif // DevHudRenderStage_h__
///
/// NightVisionRenderStage.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 6 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Night vision effect
///

#pragma once

#ifndef NightVisionRenderStage_h__
#define NightVisionRenderStage_h__

#define NIGHT_VISION_NOISE_QUALITY 512

#include "FullQuadVBO.h"
#include "GLProgram.h"
#include "IRenderStage.h"
#include "Texture.h"

class NightVisionRenderStage : public vg::IRenderStage {
public:
    /// Constructor which injects dependencies
    /// @param glProgram: The program used to render HDR
    /// @param quad: Quad used for rendering to screen
    NightVisionRenderStage(vg::GLProgram* glProgram, vg::FullQuadVBO* quad);
    virtual ~NightVisionRenderStage();

    /// Draws the render stage
    virtual void draw() override;
private:
    vg::GLProgram* _glProgram; ///< Stores the program we use to render
    vg::FullQuadVBO* _quad; ///< For use in processing through data
    vg::Texture _texNoise;
    float _et = 0.0f;
};

#endif // NightVisionRenderStage_h__

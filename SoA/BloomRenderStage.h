/// 
///  BloomRenderStage.h
///  Seed of Andromeda
///
///  Created by Isaque Dutra on 2 June 2015
///  Copyright 2015 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements a bloom render stage for
///  MainMenuRenderer.
///

#pragma once

#ifndef BloomRenderStage_h__
#define BloomRenderStage_h__

#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLProgram.h>
#include "ShaderLoader.h"
#include "LoadContext.h"

#include "IRenderStage.h"


typedef enum {
    BLOOM_RENDER_STAGE_LUMA,
    BLOOM_RENDER_STAGE_GAUSSIAN_FIRST,
    BLOOM_RENDER_STAGE_GAUSSIAN_SECOND
} BloomRenderStagePass;

class BloomRenderStage : public IRenderStage {
public:

    void init(vui::GameWindow* window, StaticLoadContext& context) override;

    void setParams(ui32 gaussianN = 20, float gaussian_variance = 36.0f, float luma_threshold = 0.75f);

    void load(StaticLoadContext& context) override;

    void hook(vg::FullQuadVBO* quad);

    void dispose(StaticLoadContext& context) override;

    /// Draws the render stage
    void render(const Camera* camera = nullptr) override;

    void setIntensity(float intensity = 1.0f);

private:
    float gauss(int i, float sigma2);
    void render(BloomRenderStagePass stage);

    vg::GLProgram m_program_luma, m_program_gaussian_first, m_program_gaussian_second;
    vg::FullQuadVBO* m_quad;    ///< For use in processing through data
    vg::GLRenderTarget m_fbo1, m_fbo2;
    ui32 m_gaussianN;           ///< Threshold for filtering image luma for bloom bluring
    float m_gaussian_variance;  ///< Radius number for gaussian blur. Must be less than 50.
    float m_luma_threshold;     ///< Gaussian variance for blur pass
};

#endif // BloomRenderStage_h__
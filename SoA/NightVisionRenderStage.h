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
#define NIGHT_VISION_DEFAULT_NOISE_POWER 0.001f
#define NIGHT_VISION_DEFAULT_NOISE_COLOR 0.2f
#define NIGHT_VISION_DEFAULT_LUMINANCE_TARE 0.5f
#define NIGHT_VISION_DEFAULT_LUMINANCE_EXPONENT 1.15f
#define NIGHT_VISION_DEFAULT_COLOR_AMPLIFICATION 5.0f
#define NIGHT_VISION_DEFAULT_NOISE_TIME_STEP 0.016667f
#define NIGHT_VISION_TEXTURE_SLOT_COLOR 0
#define NIGHT_VISION_TEXTURE_SLOT_NOISE 1
#define NIGHT_VISION_DEFAULT_VISION_COLOR f32v3(0.1f, 0.95f, 0.2f)

#include <Vorb/io/Keg.h>
#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/Texture.h>

#include "IRenderStage.h"

class NightVisionRenderParams {
public:
    static NightVisionRenderParams createDefault();

    f32v3 color;
    f32 luminanceExponent;
    f32 luminanceTare;
    f32 colorAmplification;
    f32 noisePower;
    f32 noiseColor;
};
KEG_TYPE_DECL(NightVisionRenderParams);

/// Renders a night vision post-process effect
class NightVisionRenderStage : public IRenderStage {
public:
    void hook(vg::FullQuadVBO* quad);

    void setParams(NightVisionRenderParams& params);

    /// Disposes and deletes the shader and turns off visibility
    /// If stage does lazy init, shader will reload at next draw
    virtual void dispose(StaticLoadContext& context) override;

    /// Draws the render stage
    virtual void render(const Camera* camera = nullptr) override;
private:
    vg::GLProgram m_program;
    vg::FullQuadVBO* m_quad; ///< For use in processing through data
    vg::Texture m_texNoise; ///< A noise texture for blurry static
    f32 m_et = 0.0f; ///< Counter for elapsed total time
    f32v3 m_visionColorHSL = NIGHT_VISION_DEFAULT_VISION_COLOR; ///< Color of night vision
};

#endif // NightVisionRenderStage_h__

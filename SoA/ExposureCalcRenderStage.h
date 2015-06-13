///
/// ExposureCalcRenderStage.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Apr 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Calculates log luminance of a scene for tone mapping
///

#pragma once

#ifndef ExposureCalcRenderStage_h__
#define ExposureCalcRenderStage_h__

#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/script/Function.h>
#include <Vorb/script/Environment.h>
#include <Vorb/graphics/GLProgram.h>

#include "IRenderStage.h"

DECL_VG(class GLProgram);
DECL_VG(class GLRenderTarget);

class ExposureCalcRenderStage : public IRenderStage {
public:
    ExposureCalcRenderStage();
    ~ExposureCalcRenderStage();

    /// resolution should be power of 2
    void hook(vg::FullQuadVBO* quad, vg::GLRenderTarget* hdrFrameBuffer,
              const ui32v4* viewPort, ui32 resolution);

    /// Disposes and deletes the shader and turns off visibility
    /// If stage does lazy init, shader will reload at next draw
    virtual void dispose(StaticLoadContext& context) override;

    /// Draws the render stage
    /// @pre no FBO is bound
    virtual void render(const Camera* camera = nullptr) override;

    void setFrameBuffer(vg::GLRenderTarget* hdrFrameBuffer) { m_hdrFrameBuffer = hdrFrameBuffer; }

    const f32& getExposure() const { return m_exposure; }

private:
    vg::GLProgram m_downsampleProgram;
    std::vector<vg::GLRenderTarget> m_renderTargets; ///< All render targets
    vg::FullQuadVBO* m_quad = nullptr;
    vg::GLRenderTarget* m_hdrFrameBuffer = nullptr;
    const ui32v4* m_restoreViewport;
    ui32 m_resolution;
    ui32 m_mipLevels = 1;
    int m_mipStep = -1;
    f32 m_exposure = 0.0005f;
    vg::GLProgram m_program;

    // Script for exposure calc
    bool m_needsScriptLoad = true;
    vscript::Environment* m_scripts = nullptr;
    vscript::RFunction<f32> m_calculateExposure;
};

#endif // ExposureCalcRenderStage_h__

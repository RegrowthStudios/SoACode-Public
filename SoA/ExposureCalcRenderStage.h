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
#include <Vorb/graphics/IRenderStage.h>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/script/Function.h>
#include <Vorb/script/Environment.h>

DECL_VG(class GLProgram);
DECL_VG(class GLRenderTarget);

class ExposureCalcRenderStage : public vg::IRenderStage {
public:
    ExposureCalcRenderStage();
    ~ExposureCalcRenderStage();

    /// resolution should be power of 2
    void init(vg::FullQuadVBO* quad, vg::GLRenderTarget* hdrFrameBuffer,
              const ui32v4* viewPort, ui32 resolution);

    /// Reloads the shader. By default, it simply
    /// disposes the shader and allows a lazy init at next draw
    virtual void reloadShader() override;

    /// Disposes and deletes the shader and turns off visibility
    /// If stage does lazy init, shader will reload at next draw
    virtual void dispose() override;

    /// Draws the render stage
    /// @pre no FBO is bound
    virtual void render() override;

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

    // Script for exposure calc
    bool m_needsScriptLoad = true;
    vscript::Environment* m_scripts = nullptr;
    vscript::RFunction<f32> m_calculateExposure;
};

#endif // ExposureCalcRenderStage_h__

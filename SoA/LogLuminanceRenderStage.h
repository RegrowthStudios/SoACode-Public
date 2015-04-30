///
/// LogLuminanceRenderStage.h
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

#ifndef LogLuminanceRenderStage_h__
#define LogLuminanceRenderStage_h__

#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/IRenderStage.h>
#include <Vorb/VorbPreDecl.inl>

DECL_VG(class GLProgram);
DECL_VG(class GLRenderTarget);

class LogLuminanceRenderStage : public vg::IRenderStage {
public:
    /// Constructor
    /// resolution should be power of 2
    LogLuminanceRenderStage(vg::FullQuadVBO* quad, vg::GLRenderTarget* hdrFrameBuffer,
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

private:  
    vg::GLRenderTarget* m_renderTarget = nullptr;
    vg::FullQuadVBO* m_quad = nullptr;
    vg::GLRenderTarget* m_hdrFrameBuffer = nullptr;
    const ui32v4* m_restoreViewport;
    ui32v2 m_viewportDims;
    ui32 m_mipLevels = 0;
    bool m_hasPrevFrame = false;
};

#endif // LogLuminanceRenderStage_h__

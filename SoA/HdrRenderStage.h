/// 
///  HdrRenderStage.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements the HDR render stage, which
///  does HDR post processing.
///

#pragma once

#ifndef HdrRenderStage_h__
#define HdrRenderStage_h__

#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/IRenderStage.h>

class Camera;

class HdrRenderStage : public vg::IRenderStage {
public:
    /// Constructor which injects dependencies
    /// @param quad: Quad used for rendering to screen
    /// @param camera: Camera used to render the scene
    HdrRenderStage(vg::FullQuadVBO* quad, const Camera* camera);

    /// Reloads the shader. By default, it simply
    /// disposes the shader and allows a lazy init at next draw
    virtual void reloadShader() override;

    /// Disposes and deletes the shader and turns off visibility
    /// If stage does lazy init, shader will reload at next draw
    virtual void dispose() override;

    /// Draws the render stage
    virtual void render() override;
private:
    vg::GLProgram* m_glProgramBlur = nullptr; ///< Motion blur enabled
    vg::GLProgram* m_glProgramDoFBlur = nullptr; ///< Motion blur and DoF enabled
    vg::FullQuadVBO* m_quad = nullptr; ///< For use in processing through data
    f32m4 m_oldVP; ///< ViewProjection of previous frame
};

#endif // HdrRenderStage_h__
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

#include <Vorb/FullQuadVBO.h>
#include <Vorb/GLProgram.h>
#include <Vorb/IRenderStage.h>

#include "GLProgramManager.h"

class Camera;

class HdrRenderStage : public vg::IRenderStage {
public:
    /// Constructor which injects dependencies
    /// @param glProgram: The program used to render HDR
    /// @param quad: Quad used for rendering to screen
    /// @param camera: Camera used to render the scene
    HdrRenderStage(const vg::GLProgramManager* glPM, vg::FullQuadVBO* quad, const Camera* camera);

    /// Draws the render stage
    virtual void draw() override;
private:
    vg::GLProgram* _glProgramDefault; ///< Stores the program we use to render
    vg::GLProgram* _glProgramBlur; ///< Motion blur enabled
    vg::GLProgram* _glProgramDoFBlur; ///< Motion blur and DoF enabled
    vg::FullQuadVBO* _quad; ///< For use in processing through data
    const Camera* _camera; ///< Used for motion blur
    f32m4 _oldVP; ///< ViewProjection of previous frame
};

#endif // HdrRenderStage_h__
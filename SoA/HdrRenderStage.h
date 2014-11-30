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

#include "FullQuadVBO.h"
#include "GLProgram.h"
#include "IRenderStage.h"

class HdrRenderStage : public vg::IRenderStage {
public:
    /// Conclassor which injects dependencies
    /// @param glProgram: The program used to render HDR
    /// @param quad: Quad used for rendering to screen
    HdrRenderStage(vg::GLProgram* glProgram, vg::FullQuadVBO* quad);

    /// Draws the render stage
    virtual void draw() override;
private:
    vg::GLProgram* _glProgram; ///< Stores the program we use to render
    vg::FullQuadVBO* _quad; ///< For use in processing through data
};

#endif // HdrRenderStage_h__
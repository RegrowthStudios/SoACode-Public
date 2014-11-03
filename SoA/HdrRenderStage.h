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

#include "IRenderStage.h"
#include "GLProgram.h"

class HdrRenderStage : public vg::IRenderStage {
public:
    /// Constructor which injects dependencies
    /// @param glProgram: The program used to render HDR
    /// @param destViewport: Viewport we want to draw to. (x,y,w,h)
    HdrRenderStage(vg::GLProgram* glProgram, const ui32v4& destViewport);

    /// Draws the render stage
    virtual void draw() override;
private:
    vg::GLProgram* _glProgram; ///< Stores the program we use to render
    const ui32v4 _destViewport; ///< Stores the viewport we want to draw to. (x,y,w,h)
};

#endif // HdrRenderStage_h__
/// 
///  FullRenderPipeline.h
///  Vorb Engine
///
///  Created by Ben Arnold on 28 Oct 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file provides the implementation for the
///  full render pipeline. The full pipeline has all
///  stages activated.
///

#pragma once

#ifndef FullRenderPipeline_h_
#define FullRenderPipeline_h_

#include "IRenderPipeline.h"

class FullRenderPipeline : public vg::IRenderPipeline
{
public:
    FullRenderPipeline();
    ~FullRenderPipeline();

    /// Renders the pipeline
    void render() override;

    /// Frees all resources
    void destroy() override;

};

#endif // FullRenderPipeline_h_
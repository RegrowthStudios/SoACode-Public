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
#include "GLProgramManager.h"

class SkyboxRenderer;
class Camera;
class GameRenderStage;

class FullRenderPipeline : public vg::IRenderPipeline
{
public:
    FullRenderPipeline();
    ~FullRenderPipeline();

    /// Initializes the pipeline
    /// @param glProgramManager: Program manager that holds needed programs
    /// @param textureCache: Cache of needed textures
    /// @param chunkCamera: Camera to render the chunks
    /// @param worldCamera: Camera to render the planets
    void init(vg::GLProgramManager* glProgramManager,
              vg::TextureCache* textureCache,
              Camera* chunkCamera,
              Camera* worldCamera);

    /// Renders the pipeline
    void render() override;

    /// Frees all resources
    void destroy() override;

private:
    SkyboxRenderer* _skyboxRenderer; ///< Renders the skybox

    GameRenderStage* _gameRenderStage; ///< Renders the game scene

    bool _isInitialized; ///< True when the pipeline was initialized
};

#endif // FullRenderPipeline_h_
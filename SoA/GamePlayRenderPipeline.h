/// 
///  GamePlayRenderPipeline.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements the render pipeline for the
///  GamePlayScreen.
///
#pragma once

#ifndef GamePlayRenderPipeline_h__
#define GamePlayRenderPipeline_h__

#include "IRenderPipeline.h"
#include "GLProgramManager.h"
#include "FrameBuffer.h"

/// Forward declarations
class SkyboxRenderStage;
class PlanetRenderStage;
class HdrRenderStage;
class Camera;
class IAwesomiumInterface;

class GamePlayRenderPipeline : public vg::IRenderPipeline {
public:
    GamePlayRenderPipeline();
    ~GamePlayRenderPipeline();

    void init(const ui32v4& viewport, Camera* chunkCamera, Camera* worldCamera, vg::GLProgramManager* glProgramManager);

    /// Renders the pipeline
    virtual void render() override;

    /// Frees all resources
    virtual void destroy() override;
private: 
    SkyboxRenderStage* _skyboxRenderStage; ///< Renders the skybox
    PlanetRenderStage* _planetRenderStage; ///< Renders the planets
    HdrRenderStage* _hdrRenderStage; ///< Renders HDR post-processing

    vg::FrameBuffer* _hdrFrameBuffer; ///< Framebuffer needed for the HDR rendering

    ui32v4 _viewport; ///< Viewport to draw to
    Camera* _chunkCamera;
    Camera* _worldCamera;
};

#endif // GamePlayRenderPipeline_h__
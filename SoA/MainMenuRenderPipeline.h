/// 
///  MainMenuRenderPipeline.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements the rendering pipeline for the main menu screen.
///

#pragma once

#ifndef MainMenuRenderPipeline_h__
#define MainMenuRenderPipeline_h__

#include "IRenderPipeline.h"
#include "GLProgramManager.h"
#include "FrameBuffer.h"

/// Forward declarations
class SkyboxRenderStage;
class PlanetRenderStage;
class AwesomiumRenderStage;
class HdrRenderStage;
class Camera;
class IAwesomiumInterface;

class MainMenuRenderPipeline : public vg::IRenderPipeline 
{
public:
    MainMenuRenderPipeline();
    ~MainMenuRenderPipeline();

    /// Initializes the pipeline and passes dependencies
    /// @param viewport: The viewport to draw to.
    /// @param camera: The camera used for rendering.
    /// @param awesomiumInterface: The user interface
    /// @param glProgramManager: The program cache that contains all needed
    /// GLPrograms
    void init(const ui32v4& viewport, Camera* camera, IAwesomiumInterface* awesomiumInterface, vg::GLProgramManager* glProgramManager);

    /// Renders the pipeline
    virtual void render() override;

    /// Frees all resources
    virtual void destroy() override;
private:
    SkyboxRenderStage* _skyboxRenderStage = nullptr; ///< Renders the skybox
    PlanetRenderStage* _planetRenderStage = nullptr; ///< Renders the planets
    AwesomiumRenderStage* _awesomiumRenderStage = nullptr; ///< Renders the UI
    HdrRenderStage* _hdrRenderStage = nullptr; ///< Renders HDR post-processing

    vg::FrameBuffer* _hdrFrameBuffer = nullptr; ///< Framebuffer needed for the HDR rendering

    ui32v4 _viewport; ///< Viewport to draw to
};

#endif // MainMenuRenderPipeline_h__

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

#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/IRenderPipeline.h>
#include <Vorb/graphics/RTSwapChain.hpp>

#include "GLProgramManager.h"

/// Forward declarations
class AwesomiumRenderStage;
class Camera;
class HdrRenderStage;
class IAwesomiumInterface;
class SkyboxRenderStage;
class SpaceSystem;
class SpaceSystemRenderStage;
class MainMenuSystemViewer;

class MainMenuRenderPipeline : public vg::IRenderPipeline 
{
public:
    MainMenuRenderPipeline();
    ~MainMenuRenderPipeline();

    /// Initializes the pipeline and passes dependencies
    /// @param viewport: The viewport to draw to.
    /// @param camera: The camera used for rendering.
    /// @param awesomiumInterface: The user interface
    /// @param spaceSystem: The space system for rendering
    /// @param systemViewer: System viewing controller for main menu
    /// @param glProgramManager: The program cache that contains all needed
    /// GLPrograms
    void init(const ui32v4& viewport, Camera* camera,
              IAwesomiumInterface* awesomiumInterface,
              SpaceSystem* spaceSystem,
              const MainMenuSystemViewer* systemViewer,
              const vg::GLProgramManager* glProgramManager);

    /// Renders the pipeline
    virtual void render() override;

    /// Frees all resources
    virtual void destroy() override;
private:
    SkyboxRenderStage* _skyboxRenderStage = nullptr; ///< Renders the skybox
    AwesomiumRenderStage* _awesomiumRenderStage = nullptr; ///< Renders the UI
    HdrRenderStage* _hdrRenderStage = nullptr; ///< Renders HDR post-processing
    SpaceSystemRenderStage* m_spaceSystemRenderStage = nullptr; ///< Renders space system

    vg::GLRenderTarget* _hdrFrameBuffer = nullptr; ///< Framebuffer needed for the HDR rendering
    vg::RTSwapChain<2>* _swapChain = nullptr; ///< Swap chain of framebuffers used for post-processing
    vg::FullQuadVBO _quad; ///< Quad used for post-processing

    ui32v4 _viewport; ///< Viewport to draw to
};

#endif // MainMenuRenderPipeline_h__

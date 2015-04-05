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
#include <Vorb/graphics/RenderPipeline.h>
#include <Vorb/graphics/RTSwapChain.hpp>

/// Forward declarations
class AwesomiumRenderStage;
class Camera;
class HdrRenderStage;
class IAwesomiumInterface;
class SkyboxRenderStage;
class SpaceSystem;
class SpaceSystemRenderStage;
class MainMenuSystemViewer;

class MainMenuRenderPipeline : public vg::RenderPipeline 
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
    /// GLPrograms
    void init(const ui32v4& viewport, Camera* camera,
              IAwesomiumInterface* awesomiumInterface,
              SpaceSystem* spaceSystem,
              const MainMenuSystemViewer* systemViewer);

    /// Renders the pipeline
    virtual void render() override;

    /// Frees all resources
    virtual void destroy(bool shouldDisposeStages) override;
private:
    SkyboxRenderStage* m_skyboxRenderStage = nullptr; ///< Renders the skybox
    AwesomiumRenderStage* m_awesomiumRenderStage = nullptr; ///< Renders the UI
    HdrRenderStage* m_hdrRenderStage = nullptr; ///< Renders HDR post-processing
    SpaceSystemRenderStage* m_spaceSystemRenderStage = nullptr; ///< Renders space system

    vg::GLRenderTarget* m_hdrFrameBuffer = nullptr; ///< Framebuffer needed for the HDR rendering
    vg::RTSwapChain<2>* m_swapChain = nullptr; ///< Swap chain of framebuffers used for post-processing
    vg::FullQuadVBO m_quad; ///< Quad used for post-processing

    ui32v4 m_viewport; ///< Viewport to draw to
    bool m_isInitialized = false;
};

#endif // MainMenuRenderPipeline_h__

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
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/Events.hpp>

#include "ExposureCalcRenderStage.h"
#include "SpaceSystemRenderStage.h"
#include "ColorFilterRenderStage.h"
#include "SkyboxRenderStage.h"
#include "HdrRenderStage.h"

/// Forward declarations
class Camera;
class MainMenuSystemViewer;
class SoaState;
class SpaceSystem;
class MainMenuScriptedUI;
DECL_VUI(struct WindowResizeEvent);

class MainMenuRenderPipeline : public vg::RenderPipeline 
{
public:
    MainMenuRenderPipeline();
    ~MainMenuRenderPipeline();

    /// Initializes the pipeline and passes dependencies
    void init(const SoaState* soaState, const ui32v4& viewport,
              MainMenuScriptedUI* mainMenuUI,
              Camera* camera,
              const MainMenuSystemViewer* systemViewer);

    /// Renders the pipeline
    virtual void render() override;

    /// Frees all resources
    virtual void destroy(bool shouldDisposeStages) override;

    void onWindowResize(Sender s, const vui::WindowResizeEvent& e);

    void takeScreenshot() { m_shouldScreenshot = true; }

    void toggleUI() { m_showUI = !m_showUI; }
    void toggleAR() { m_showAR = !m_showAR; }
    void toggleWireframe() { m_wireframe = !m_wireframe; }
    void cycleColorFilter() { m_colorFilter++; if (m_colorFilter > 3) m_colorFilter = 0; }

    struct {
        ColorFilterRenderStage colorFilter;
        SkyboxRenderStage skybox;
        HdrRenderStage hdr;
        SpaceSystemRenderStage spaceSystem;
        ExposureCalcRenderStage exposureCalc;
    } stages;

private:
    void initFramebuffer();
    void resize();
    void dumpScreenshot();

    vg::GLRenderTarget* m_hdrFrameBuffer = nullptr; ///< Framebuffer needed for the HDR rendering
    vg::RTSwapChain<2>* m_swapChain = nullptr; ///< Swap chain of framebuffers used for post-processing
    vg::FullQuadVBO m_quad; ///< Quad used for post-processing
    MainMenuScriptedUI* m_mainMenuUI; ///< The main menu UI

    ui32v4 m_viewport; ///< Viewport to draw to
    bool m_isInitialized = false;
    bool m_showUI = true;
    bool m_showAR = true;
    bool m_shouldScreenshot = false;
    bool m_shouldResize = false;
    bool m_wireframe = false;
    ui32v2 m_newDims;
    int m_colorFilter = 0;
};

#endif // MainMenuRenderPipeline_h__

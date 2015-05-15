#include "stdafx.h"
#include "MainMenuRenderPipeline.h"

#include <Vorb/graphics/TextureCache.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/io/FileOps.h>
#include <Vorb/utils.h>
#include <Vorb/ui/InputDispatcher.h>

#include "ColorFilterRenderStage.h"
#include "Errors.h"
#include "GameManager.h"
#include "HdrRenderStage.h"
#include "ExposureCalcRenderStage.h"
#include "SoaOptions.h"
#include "SkyboxRenderStage.h"
#include "SoaState.h"
#include "SpaceSystemRenderStage.h"
#include "soaUtils.h"
#include "MainMenuScriptedUI.h"


MainMenuRenderPipeline::MainMenuRenderPipeline() {
    // Empty
}


MainMenuRenderPipeline::~MainMenuRenderPipeline() {
    destroy(true);
}

void MainMenuRenderPipeline::init(const SoaState* soaState, const ui32v4& viewport,
                                  MainMenuScriptedUI* mainMenuUI,
                                  Camera* camera,
                                  SpaceSystem* spaceSystem,
                                  const MainMenuSystemViewer* systemViewer) {
    // Set the viewport
    m_viewport = viewport;
    m_mainMenuUI = mainMenuUI;

    vui::InputDispatcher::window.onResize += makeDelegate(*this, &MainMenuRenderPipeline::onWindowResize);

    // Check to make sure we don't double init
    if (m_isInitialized) {
        pError("Reinitializing MainMenuRenderPipeline without first calling destroy()!");
        return;
    } else {
        m_isInitialized = true;
    }

    initFramebuffer();

    m_quad.init();

    // Helpful macro to reduce code size
#define ADD_STAGE(type, ...) static_cast<type*>(addStage(std::make_shared<type>(__VA_ARGS__)))

    // Init render stages
    m_colorFilterRenderStage = ADD_STAGE(ColorFilterRenderStage, &m_quad);
    m_skyboxRenderStage = ADD_STAGE(SkyboxRenderStage, camera, &soaState->texturePathResolver);
    m_hdrRenderStage = ADD_STAGE(HdrRenderStage, &m_quad, camera);
    m_spaceSystemRenderStage = ADD_STAGE(SpaceSystemRenderStage, soaState, ui32v2(m_viewport.z, m_viewport.w),
                                                          spaceSystem, nullptr, systemViewer, camera, nullptr);
    m_logLuminanceRenderStage = ADD_STAGE(ExposureCalcRenderStage, &m_quad, m_hdrFrameBuffer, &m_viewport, 1024);
}

void MainMenuRenderPipeline::render() {
    
    // Check for window resize
    if (m_shouldResize) resize();

    // Bind the FBO
    m_hdrFrameBuffer->use();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // Main render passes
    m_skyboxRenderStage->render();
    m_spaceSystemRenderStage->setShowAR(m_showAR);
    m_spaceSystemRenderStage->render();

    f32v3 colorFilter(1.0);
    // Color filter rendering
    if (m_colorFilter != 0) {
        switch (m_colorFilter) {
            case 1:
                colorFilter = f32v3(0.66f);
                m_colorFilterRenderStage->setColor(f32v4(0.0, 0.0, 0.0, 0.33f)); break;
            case 2:
                colorFilter = f32v3(0.3f);
                m_colorFilterRenderStage->setColor(f32v4(0.0, 0.0, 0.0, 0.66f)); break;
            case 3:
                colorFilter = f32v3(0.0f);
                m_colorFilterRenderStage->setColor(f32v4(0.0, 0.0, 0.0, 0.9f)); break;
        }
        m_colorFilterRenderStage->render();
    }

    // Render last
    glBlendFunc(GL_ONE, GL_ONE);
    m_spaceSystemRenderStage->renderStarGlows(colorFilter);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Post processing
    m_swapChain->reset(0, m_hdrFrameBuffer->getID(), m_hdrFrameBuffer->getTextureID(), soaOptions.get(OPT_MSAA).value.i > 0, false);

    // TODO: More Effects?

    // Draw to backbuffer for the last effect
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    m_logLuminanceRenderStage->render();
    // Move exposure towards target
    static const f32 EXPOSURE_STEP = 0.005f;
    stepTowards(soaOptions.get(OPT_HDR_EXPOSURE).value.f, m_logLuminanceRenderStage->getExposure(), EXPOSURE_STEP);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(m_hdrFrameBuffer->getTextureTarget(), m_hdrFrameBuffer->getTextureID());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(m_hdrFrameBuffer->getTextureTarget(), m_hdrFrameBuffer->getTextureDepthID());
    m_hdrRenderStage->render();

    if (m_showUI) m_mainMenuUI->draw();

    if (m_shouldScreenshot) dumpScreenshot();

    // Check for errors, just in case
    checkGlError("MainMenuRenderPipeline::render()");
}

void MainMenuRenderPipeline::destroy(bool shouldDisposeStages) {
    if (!m_isInitialized) return;
    RenderPipeline::destroy(shouldDisposeStages);

    if (m_swapChain) {
        m_swapChain->dispose();
        delete m_swapChain;
        m_swapChain = nullptr;
    }

    vui::InputDispatcher::window.onResize -= makeDelegate(*this, &MainMenuRenderPipeline::onWindowResize);

    m_quad.dispose();

    m_isInitialized = false;
}

void MainMenuRenderPipeline::onWindowResize(Sender s, const vui::WindowResizeEvent& e) {
    m_newDims = ui32v2(e.w, e.h);
    m_shouldResize = true;
}

void MainMenuRenderPipeline::initFramebuffer() {
    // Construct framebuffer
    m_hdrFrameBuffer = new vg::GLRenderTarget(m_viewport.z, m_viewport.w);
    m_hdrFrameBuffer->init(vg::TextureInternalFormat::RGBA16F, (ui32)soaOptions.get(OPT_MSAA).value.i).initDepth();
    if (soaOptions.get(OPT_MSAA).value.i > 0) {
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }

    // Make swap chain
    m_swapChain = new vg::RTSwapChain<2>();
    m_swapChain->init(m_viewport.z, m_viewport.w, vg::TextureInternalFormat::RGBA8);
}

void MainMenuRenderPipeline::resize() {
    m_viewport.z = m_newDims.x;
    m_viewport.w = m_newDims.y;

    std::cout << "NewSize " << m_newDims.x << " " << m_newDims.y << std::endl;

    m_hdrFrameBuffer->dispose();
    delete m_hdrFrameBuffer;
    m_swapChain->dispose();
    delete m_swapChain;
    initFramebuffer();

    m_spaceSystemRenderStage->setViewport(m_newDims);
    m_logLuminanceRenderStage->setFrameBuffer(m_hdrFrameBuffer);

    m_mainMenuUI->setDimensions(m_newDims);

    m_shouldResize = false;
}

void MainMenuRenderPipeline::dumpScreenshot() {
    // Make screenshots directory
    vio::buildDirectoryTree("Screenshots");
    // Take screenshot
    dumpFramebufferImage("Screenshots/", m_viewport);
    m_shouldScreenshot = false;
}

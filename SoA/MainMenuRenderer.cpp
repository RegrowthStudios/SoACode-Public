#include "stdafx.h"
#include "MainMenuRenderer.h"

#include <Vorb/graphics/TextureCache.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/io/FileOps.h>
#include <Vorb/utils.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/ui/GameWindow.h>

#include "CommonState.h"
#include "Errors.h"
#include "GameManager.h"
#include "HdrRenderStage.h"
#include "MainMenuScreen.h"
#include "MainMenuScriptedUI.h"
#include "SoaOptions.h"
#include "SoAState.h"
#include "soaUtils.h"
#include "BloomRenderStage.h"

void MainMenuRenderer::init(vui::GameWindow* window, StaticLoadContext& context,
                            MainMenuScreen* mainMenuScreen, CommonState* commonState) {
    m_window = window;
    m_mainMenuScreen = mainMenuScreen;
    m_commonState = commonState;
    m_state = m_commonState->state;
    vui::InputDispatcher::window.onResize += makeDelegate(*this, &MainMenuRenderer::onWindowResize);

    // TODO(Ben): Dis is bad mkay
    m_viewport = f32v4(0, 0, m_window->getWidth(), m_window->getHeight());

    m_mainMenuUI = &m_mainMenuScreen->m_ui;
    // Add anticipated work
    context.addAnticipatedWork(3, 3);

    // Init render stages
    m_commonState->stages.skybox.init(window, context);
    m_commonState->stages.spaceSystem.init(window, context);
    m_commonState->stages.hdr.init(window, context);
    stages.colorFilter.init(window, context);
    stages.exposureCalc.init(window, context);

    stages.bloom.init(window, context);
    stages.bloom.setParams();
    stages.bloom.setActive(true);


    // TODO(Cristian): This needs to be upgraded.
    // Generate terrain patch indices
    TerrainPatchMesher::generateIndices();
}

void MainMenuRenderer::dispose(StaticLoadContext& context) {
    vui::InputDispatcher::window.onResize -= makeDelegate(*this, &MainMenuRenderer::onWindowResize);

    // Kill the builder
    if (m_loadThread) {
        delete m_loadThread;
        m_loadThread = nullptr;
    }
    // TODO(Ben): Dispose common stages
    stages.colorFilter.dispose(context);
    stages.exposureCalc.dispose(context);
    stages.bloom.dispose(context);

    // Dispose of persistent rendering resources
    m_hdrTarget.dispose();
    m_swapChain.dispose();
}

void MainMenuRenderer::load(StaticLoadContext& context) {
    m_isLoaded = false;



    m_loadThread = new std::thread([&]() {
        vcore::GLRPC so[4];
        // size_t i = 0;

        // Create the HDR target  
        // Create the HDR target     
        context.addTask([&](Sender, void*) {
            Array<vg::GBufferAttachment> attachments;
            vg::GBufferAttachment att[2];
            // TODO(Ben): Don't think this is right.
            // Color
            att[0].format = vg::TextureInternalFormat::RGBA16F;
            att[0].pixelFormat = vg::TextureFormat::RGBA;
            att[0].pixelType = vg::TexturePixelType::UNSIGNED_BYTE;
            att[0].number = 1;
            // Normals
            att[1].format = vg::TextureInternalFormat::RGBA16F;
            att[1].pixelFormat = vg::TextureFormat::RGBA;
            att[1].pixelType = vg::TexturePixelType::UNSIGNED_BYTE;
            att[1].number = 2;
            m_hdrTarget.setSize(m_window->getWidth(), m_window->getHeight());
            m_hdrTarget.init(Array<vg::GBufferAttachment>(att, 2), vg::TextureInternalFormat::RGBA32F).initDepth();

            if (soaOptions.get(OPT_MSAA).value.i > 0) {
                glEnable(GL_MULTISAMPLE);
            }
            else {
                glDisable(GL_MULTISAMPLE);
            }
            context.addWorkCompleted(1);
        }, false);

        // Create the swap chain for post process effects (HDR-capable)
        context.addTask([&](Sender, void*) { 
            m_swapChain.init(m_window->getWidth(), m_window->getHeight(), vg::TextureInternalFormat::RGBA32F);
            context.addWorkCompleted(1);
        }, false);

        // Create full-screen quad
        context.addTask([&](Sender, void*) {
            m_commonState->quad.init();
            context.addWorkCompleted(1);
        }, false);

        // Load all the stages
        m_commonState->stages.skybox.load(context);
        m_commonState->stages.spaceSystem.load(context);
        m_commonState->stages.hdr.load(context);
        stages.colorFilter.load(context);
        stages.exposureCalc.load(context);
        stages.bloom.load(context);

        context.blockUntilFinished();

        m_isLoaded = true;
    });
    m_loadThread->detach();
}

void MainMenuRenderer::hook() {
    m_commonState->stages.skybox.hook(m_state);
    m_commonState->stages.spaceSystem.hook(m_state, &m_state->clientState.spaceCamera, nullptr);
    m_commonState->stages.hdr.hook(&m_commonState->quad);
    stages.colorFilter.hook(&m_commonState->quad);
    stages.exposureCalc.hook(&m_commonState->quad, &m_hdrTarget, &m_viewport, 1024);
    stages.bloom.hook(&m_commonState->quad);
}

void MainMenuRenderer::render() {

    // Check for window resize
    if (m_shouldResize) resize();

    // Bind the FBO
    m_hdrTarget.useGeometry();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // Main render passes
    m_commonState->stages.skybox.render(&m_state->clientState.spaceCamera);

    // Check fore wireframe mode
    if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    m_commonState->stages.spaceSystem.setShowAR(m_showAR);
    m_commonState->stages.spaceSystem.render(&m_state->clientState.spaceCamera);

    // Restore fill
    if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    f32v3 colorFilter(1.0);
    // Color filter rendering
    if (m_colorFilter != 0) {
        switch (m_colorFilter) {
        case 1:
            colorFilter = f32v3(0.66f);
            stages.colorFilter.setColor(f32v4(0.0, 0.0, 0.0, 0.33f)); break;
        case 2:
            colorFilter = f32v3(0.3f);
            stages.colorFilter.setColor(f32v4(0.0, 0.0, 0.0, 0.66f)); break;
        case 3:
            colorFilter = f32v3(0.0f);
            stages.colorFilter.setColor(f32v4(0.0, 0.0, 0.0, 0.9f)); break;
        }
        stages.colorFilter.render();
    }

    stages.exposureCalc.render();
    // Move exposure towards target
    static const f32 EXPOSURE_STEP = 0.005f;
    stepTowards(soaOptions.get(OPT_HDR_EXPOSURE).value.f, stages.exposureCalc.getExposure(), EXPOSURE_STEP);

    // Post processing
    m_swapChain.reset(0, m_hdrTarget.getGeometryID(), m_hdrTarget.getGeometryTexture(0), soaOptions.get(OPT_MSAA).value.i > 0, false);

    // TODO: More Effects?

    // last effect should not swap swapChain
    if (stages.bloom.isActive()) {
        stages.bloom.render();
    }

    // Render last
    glBlendFunc(GL_ONE, GL_ONE);
    m_commonState->stages.spaceSystem.renderStarGlows(colorFilter);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_swapChain.swap();
    m_swapChain.bindPreviousTexture(0);

    // Draw to backbuffer for the last effect
    m_swapChain.unuse(m_window->getWidth(), m_window->getHeight());
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // original depth texture
    m_hdrTarget.bindDepthTexture(1);
    m_commonState->stages.hdr.render(&m_state->clientState.spaceCamera);

    if (m_showUI) m_mainMenuUI->draw();

    if (m_shouldScreenshot) dumpScreenshot();

    // Check for errors, just in case
    checkGlError("MainMenuRenderPipeline::render()");
}

void MainMenuRenderer::onWindowResize(Sender s VORB_MAYBE_UNUSED, const vui::WindowResizeEvent& e) {
    m_newDims = ui32v2(e.w, e.h);
    m_shouldResize = true;
}

void MainMenuRenderer::resize() {
    m_viewport.z = m_newDims.x;
    m_viewport.w = m_newDims.y;

    // TODO(Ben): Fix
    /* m_hdrFrameBuffer->dispose();
     delete m_hdrFrameBuffer;
     m_swapChain->dispose();
     delete m_swapChain;
     initFramebuffer();*/

    m_commonState->stages.spaceSystem.setViewport(m_newDims);
    stages.exposureCalc.setFrameBuffer(&m_hdrTarget);

    m_mainMenuUI->setDimensions(f32v2(m_newDims));

    m_shouldResize = false;
}

void MainMenuRenderer::dumpScreenshot() {
    // Make screenshots directory
    vio::buildDirectoryTree("Screenshots");
    // Take screenshot
    dumpFramebufferImage("Screenshots/", m_viewport);
    m_shouldScreenshot = false;
}

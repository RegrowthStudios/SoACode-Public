#include "stdafx.h"
#include "MainMenuRenderer.h"

#include <Vorb/graphics/TextureCache.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/io/FileOps.h>
#include <Vorb/utils.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/ui/GameWindow.h>

#include "Errors.h"
#include "GameManager.h"
#include "HdrRenderStage.h"
#include "MainMenuScreen.h"

#include "SoaOptions.h"
#include "SoaState.h"
#include "soaUtils.h"
#include "MainMenuScriptedUI.h"

void MainMenuRenderer::init(vui::GameWindow* window, LoadContext& context, MainMenuScreen* mainMenuScreen) {
    m_window = window;
    m_mainMenuScreen = mainMenuScreen;
    vui::InputDispatcher::window.onResize += makeDelegate(*this, &MainMenuRenderer::onWindowResize);

    m_mainMenuUI = &m_mainMenuScreen->m_ui;

    // Init render stages
    stages.skybox.init(window, context);
    stages.spaceSystem.init(window, context);
    stages.colorFilter.init(window, context);
    stages.exposureCalc.init(window, context);
    stages.hdr.init(window, context);
}

void MainMenuRenderer::dispose(LoadContext& context) {
    vui::InputDispatcher::window.onResize -= makeDelegate(*this, &MainMenuRenderer::onWindowResize);

    // Kill the builder
    if (m_loadThread) {
        delete m_loadThread;
        m_loadThread = nullptr;
    }

    stages.skybox.dispose(context);
    stages.spaceSystem.dispose(context);
    stages.colorFilter.dispose(context);
    stages.exposureCalc.dispose(context);
    stages.hdr.dispose(context);

    // Dispose of persistent rendering resources
    m_hdrTarget.dispose();
    m_swapChain.dispose();
    m_quad.dispose();

    m_isInitialized = false;
}

void MainMenuRenderer::load(LoadContext& context) {
    m_loaded = false;

    m_loadThread = new std::thread([&]() {
        vcore::GLRPC so[4];
        size_t i = 0;

        // Create the HDR target     
        so[i].set([&](Sender, void*) {
            m_hdrTarget.setSize(m_window->getWidth(), m_window->getHeight());
            m_hdrTarget.init(vg::TextureInternalFormat::RGBA16F, (ui32)soaOptions.get(OPT_MSAA).value.i).initDepth();
            if (soaOptions.get(OPT_MSAA).value.i > 0) {
                glEnable(GL_MULTISAMPLE);
            } else {
                glDisable(GL_MULTISAMPLE);
            }
        });
        m_glrpc.invoke(&so[i++], false);

        // Create the swap chain for post process effects (HDR-capable)
        so[i].set([&](Sender, void*) {
            m_swapChain.init(m_window->getWidth(), m_window->getHeight(), vg::TextureInternalFormat::RGBA16F);
        });
        m_glrpc.invoke(&so[i++], false);

        // Create full-screen quad
        so[i].set([&](Sender, void*) {
            m_quad.init();
        });
        m_glrpc.invoke(&so[i++], false);

        // Wait for the last command to complete
        so[i - 1].block();

        // Load all the stages
        stages.skybox.load(context, m_glrpc);
        stages.spaceSystem.load(context, m_glrpc);
        stages.colorFilter.load(context, m_glrpc);
        stages.exposureCalc.load(context, m_glrpc);
        stages.hdr.load(context, m_glrpc);

        m_loaded = true;
    });
    m_loadThread->detach();
}

void MainMenuRenderer::hook(SoaState* state) {
    m_state = state;
    stages.skybox.hook(state);
    stages.spaceSystem.hook(state, &state->spaceCamera);
    stages.colorFilter.hook(&m_quad);
    stages.exposureCalc.hook(&m_quad, &m_hdrTarget, &m_viewport, 1024);
    stages.hdr.hook(&m_quad);
}

void MainMenuRenderer::updateGL() {
    // TODO(Ben): Experiment with more requests
    m_glrpc.processRequests(1);
}

void MainMenuRenderer::render() {
    
    // Check for window resize
    if (m_shouldResize) resize();

    // Bind the FBO
    m_hdrTarget.use();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // Main render passes
    stages.skybox.render(&m_state->spaceCamera);

    // Check fore wireframe mode
    if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    stages.spaceSystem.setShowAR(m_showAR);
    stages.spaceSystem.render(&m_state->spaceCamera);

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

    // Render last
    glBlendFunc(GL_ONE, GL_ONE);
    stages.spaceSystem.renderStarGlows(colorFilter);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Post processing
    m_swapChain.reset(0, m_hdrTarget.getID(), m_hdrTarget.getTextureID(), soaOptions.get(OPT_MSAA).value.i > 0, false);

    // TODO: More Effects?

    // Draw to backbuffer for the last effect
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    stages.exposureCalc.render();
    // Move exposure towards target
    static const f32 EXPOSURE_STEP = 0.005f;
    stepTowards(soaOptions.get(OPT_HDR_EXPOSURE).value.f, stages.exposureCalc.getExposure(), EXPOSURE_STEP);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(m_hdrTarget.getTextureTarget(), m_hdrTarget.getTextureID());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(m_hdrTarget.getTextureTarget(), m_hdrTarget.getTextureDepthID());
    stages.hdr.render(&m_state->spaceCamera);

    if (m_showUI) m_mainMenuUI->draw();

    if (m_shouldScreenshot) dumpScreenshot();

    // Check for errors, just in case
    checkGlError("MainMenuRenderPipeline::render()");
}

void MainMenuRenderer::onWindowResize(Sender s, const vui::WindowResizeEvent& e) {
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

    stages.spaceSystem.setViewport(m_newDims);
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

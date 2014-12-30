#include "stdafx.h"
#include "MainMenuRenderPipeline.h"

#include "AwesomiumRenderStage.h"
#include "Errors.h"
#include "HdrRenderStage.h"
#include "Options.h"
#include "PlanetRenderStage.h"
#include "SkyboxRenderStage.h"
#include "SpaceSystemRenderStage.h"

MainMenuRenderPipeline::MainMenuRenderPipeline() {
    // Empty
}


MainMenuRenderPipeline::~MainMenuRenderPipeline() {
    destroy();
}

void MainMenuRenderPipeline::init(const ui32v4& viewport, Camera* camera,
                                  IAwesomiumInterface* awesomiumInterface,
                                  const SpaceSystem* spaceSystem,
                                  const vg::GLProgramManager* glProgramManager) {
    // Set the viewport
    _viewport = viewport;

    // Check to make sure we aren't leaking memory
    if (_skyboxRenderStage != nullptr) {
        pError("Reinitializing MainMenuRenderPipeline without first calling destroy()!");
    }

    // Conclass framebuffer
    _hdrFrameBuffer = new vg::GLRenderTarget(_viewport.z, _viewport.w);
    _hdrFrameBuffer->init(vg::TextureInternalFormat::RGBA16F, graphicsOptions.msaa).initDepth();
    if (graphicsOptions.msaa > 0) {
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }

    // Make swap chain
    _swapChain = new vg::RTSwapChain<2>(_viewport.z, _viewport.w);
    _swapChain->init(vg::TextureInternalFormat::RGBA8);
    _quad.init();

    // Init render stages
    _skyboxRenderStage = new SkyboxRenderStage(glProgramManager->getProgram("Texture"), camera);
    _planetRenderStage = new PlanetRenderStage(camera);
    _awesomiumRenderStage = new AwesomiumRenderStage(awesomiumInterface, glProgramManager->getProgram("Texture2D"));

    _hdrRenderStage = new HdrRenderStage(glProgramManager, &_quad, camera);
    m_spaceSystemRenderStage = new SpaceSystemRenderStage(spaceSystem, camera,
                                                          glProgramManager->getProgram("BasicColor"),
                                                          glProgramManager->getProgram("SphericalTerrain"),
                                                          glProgramManager->getProgram("SphericalWater"));

}

void MainMenuRenderPipeline::render() {
 
    // Bind the FBO
    _hdrFrameBuffer->use();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // Main render passes
    _skyboxRenderStage->draw();
    _planetRenderStage->draw();
    m_spaceSystemRenderStage->draw();
    _awesomiumRenderStage->draw();

    // Post processing
    _swapChain->reset(0, _hdrFrameBuffer, graphicsOptions.msaa > 0, false);

    // TODO: More Effects?

    // Draw to backbuffer for the last effect
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(_hdrFrameBuffer->getTextureTarget(), _hdrFrameBuffer->getTextureDepthID());
    _hdrRenderStage->draw();

    // Check for errors, just in case
    checkGlError("MainMenuRenderPipeline::render()");
}

void MainMenuRenderPipeline::destroy() {
    delete _skyboxRenderStage;
    _skyboxRenderStage = nullptr;

    delete _planetRenderStage;
    _planetRenderStage = nullptr;

    delete _awesomiumRenderStage;
    _awesomiumRenderStage = nullptr;

    delete _hdrRenderStage;
    _hdrRenderStage = nullptr;

    delete m_spaceSystemRenderStage;
    m_spaceSystemRenderStage = nullptr;

    _hdrFrameBuffer->dispose();
    delete _hdrFrameBuffer;
    _hdrFrameBuffer = nullptr;

    _swapChain->dispose();
    delete _swapChain;
    _swapChain = nullptr;

    _quad.dispose();
}

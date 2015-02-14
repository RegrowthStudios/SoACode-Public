#include "stdafx.h"
#include "MainMenuRenderPipeline.h"

#include <Vorb/graphics/TextureCache.h>

#include "AwesomiumRenderStage.h"
#include "Errors.h"
#include "HdrRenderStage.h"
#include "Options.h"
#include "SkyboxRenderStage.h"
#include "SpaceSystemRenderStage.h"
#include "GameManager.h"

MainMenuRenderPipeline::MainMenuRenderPipeline() {
    // Empty
}


MainMenuRenderPipeline::~MainMenuRenderPipeline() {
    destroy();
}

void MainMenuRenderPipeline::init(const ui32v4& viewport, Camera* camera,
                                  IAwesomiumInterface* awesomiumInterface,
                                  SpaceSystem* spaceSystem,
                                  const MainMenuSystemViewer* systemViewer,
                                  const vg::GLProgramManager* glProgramManager) {
    // Set the viewport
    _viewport = viewport;

    // Check to make sure we aren't leaking memory
    if (_skyboxRenderStage != nullptr) {
        pError("Reinitializing MainMenuRenderPipeline without first calling destroy()!");
    }

    // Construct framebuffer
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
    _awesomiumRenderStage = new AwesomiumRenderStage(awesomiumInterface, glProgramManager->getProgram("Texture2D"));

    _hdrRenderStage = new HdrRenderStage(glProgramManager, &_quad, camera);
    // TODO(Ben): Use texture pack iomanager
    m_spaceSystemRenderStage = new SpaceSystemRenderStage(ui32v2(_viewport.z, _viewport.w),
                                                          spaceSystem, nullptr, systemViewer, camera, nullptr,
                                                          glProgramManager->getProgram("BasicColor"),
                                                          glProgramManager->getProgram("SphericalTerrain"),
                                                          glProgramManager->getProgram("SphericalWater"),
                                                          GameManager::textureCache->addTexture("Textures/selector.png").id);

}

void MainMenuRenderPipeline::render() {
    
    // Bind the FBO
    _hdrFrameBuffer->use();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // Main render passes
    _skyboxRenderStage->draw();
    m_spaceSystemRenderStage->draw();

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

    _awesomiumRenderStage->draw();

    // Check for errors, just in case
    checkGlError("MainMenuRenderPipeline::render()");
}

void MainMenuRenderPipeline::destroy() {
    delete _skyboxRenderStage;
    _skyboxRenderStage = nullptr;

    delete _awesomiumRenderStage;
    _awesomiumRenderStage = nullptr;

    delete _hdrRenderStage;
    _hdrRenderStage = nullptr;

    delete m_spaceSystemRenderStage;
    m_spaceSystemRenderStage = nullptr;

    delete _hdrFrameBuffer;
    _hdrFrameBuffer = nullptr;

    delete _swapChain;
    _swapChain = nullptr;

    _quad.dispose();
}

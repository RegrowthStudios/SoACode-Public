#include "stdafx.h"
#include "GamePlayRenderPipeline.h"
#include "Errors.h"
#include "SkyboxRenderStage.h"
#include "PlanetRenderStage.h"
#include "HdrRenderStage.h"
#include "FrameBuffer.h"
#include "Options.h"


GamePlayRenderPipeline::GamePlayRenderPipeline() {
    // Empty
}

void GamePlayRenderPipeline::init(const ui32v4& viewport, Camera* chunkCamera, Camera* worldCamera, vg::GLProgramManager* glProgramManager) {
    // Set the viewport
    _viewport = viewport;

    // Set cameras
    _worldCamera = worldCamera;
    _chunkCamera = chunkCamera;

    // Check to make sure we aren't leaking memory
    if (_skyboxRenderStage != nullptr) {
        pError("Reinitializing MainMenuRenderPipeline without first calling destroy()!");
    }

    // Construct frame buffer
    if (graphicsOptions.msaa > 0) {
        glEnable(GL_MULTISAMPLE);
        _hdrFrameBuffer = new vg::FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT, _viewport.z, _viewport.w, graphicsOptions.msaa);
    } else {
        glDisable(GL_MULTISAMPLE);
        _hdrFrameBuffer = new vg::FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT, _viewport.z, _viewport.w);
    }

    // Init render stages
    _skyboxRenderStage = new SkyboxRenderStage(glProgramManager->getProgram("Texture"), _worldCamera);
    _planetRenderStage = new PlanetRenderStage(_worldCamera);
    _hdrRenderStage = new HdrRenderStage(glProgramManager->getProgram("HDR"), _viewport);
}

GamePlayRenderPipeline::~GamePlayRenderPipeline() {
    destroy();
}

void GamePlayRenderPipeline::render() {
    // Bind the FBO
    _hdrFrameBuffer->bind();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // Main render passes
    _skyboxRenderStage->draw();
    _planetRenderStage->draw();

    // Post processing
    _hdrRenderStage->setInputFbo(_hdrFrameBuffer);
    _hdrRenderStage->draw();

    // Check for errors, just in case
    checkGlError("MainMenuRenderPipeline::render()");
}

void GamePlayRenderPipeline::destroy() {
    delete _skyboxRenderStage;
    _skyboxRenderStage = nullptr;

    delete _planetRenderStage;
    _planetRenderStage = nullptr;

    delete _hdrRenderStage;
    _hdrRenderStage = nullptr;

    delete _hdrFrameBuffer;
    _hdrFrameBuffer = nullptr;
}

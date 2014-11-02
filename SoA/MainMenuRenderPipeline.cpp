#include "stdafx.h"
#include "MainMenuRenderPipeline.h"

#include "Errors.h"
#include "SkyboxRenderStage.h"
#include "PlanetRenderStage.h"
#include "AwesomiumRenderStage.h"
#include "HdrRenderStage.h"
#include "FrameBuffer.h"
#include "Options.h"

MainMenuRenderPipeline::MainMenuRenderPipeline() {
    // Empty
}


MainMenuRenderPipeline::~MainMenuRenderPipeline() {
    destroy();
}

void MainMenuRenderPipeline::init(const ui32v4& viewport, Camera* camera, IAwesomiumInterface* awesomiumInterface, vg::GLProgramManager* glProgramManager) {
    // Set the viewport
    _viewport = viewport;

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
    _skyboxRenderStage = new SkyboxRenderStage(glProgramManager->getProgram("Texture"), camera);
    _planetRenderStage = new PlanetRenderStage(camera);
    _awesomiumRenderStage = new AwesomiumRenderStage(awesomiumInterface, glProgramManager->getProgram("Texture2D"));
    _hdrRenderStage = new HdrRenderStage(glProgramManager->getProgram("HDR"), _viewport);
}

void MainMenuRenderPipeline::render() {
 
    // Bind the FBO
    _hdrFrameBuffer->bind();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // Main render passes
    _skyboxRenderStage->draw();
    _planetRenderStage->draw();
    _awesomiumRenderStage->draw();

    // Post processing
    _hdrRenderStage->setInputFbo(_hdrFrameBuffer);
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

    delete _hdrFrameBuffer;
    _hdrFrameBuffer = nullptr;
}

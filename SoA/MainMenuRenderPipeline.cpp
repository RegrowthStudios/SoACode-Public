#include "stdafx.h"
#include "MainMenuRenderPipeline.h"

#include "Errors.h"
#include "SpaceRenderStage.h"
#include "PlanetRenderStage.h"
#include "AwesomiumRenderStage.h"
#include "HdrRenderStage.h"
#include "FrameBuffer.h"
#include "Options.h"

MainMenuRenderPipeline::MainMenuRenderPipeline() :
_spaceRenderStage(nullptr),
_planetRenderStage(nullptr),
_awesomiumRenderStage(nullptr),
_hdrRenderStage(nullptr),
_hdrFrameBuffer(nullptr) {
}


MainMenuRenderPipeline::~MainMenuRenderPipeline() {
}

void MainMenuRenderPipeline::init(const ui32v2& viewport, Camera* camera, IAwesomiumInterface* awesomiumInterface, vg::GLProgramManager* glProgramManager) {

    _viewport = viewport;

    // Check to make sure we aren't leaking memory
    if (_spaceRenderStage != nullptr) {
        pError("Reinitializing MainMenuRenderPipeline without first calling destroy()!");
    }

    // Construct frame buffer
    if (graphicsOptions.msaa > 0) {
        glEnable(GL_MULTISAMPLE);
        _hdrFrameBuffer = new vg::FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT, _viewport.x, _viewport.y, graphicsOptions.msaa);
    } else {
        glDisable(GL_MULTISAMPLE);
        _hdrFrameBuffer = new vg::FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT, _viewport.x, _viewport.y);
    }

    // Init render stages
    _spaceRenderStage = new SpaceRenderStage(glProgramManager->getProgram("Texture"), camera);
    _planetRenderStage = new PlanetRenderStage(camera);
    _awesomiumRenderStage = new AwesomiumRenderStage(awesomiumInterface, glProgramManager->getProgram("Texture2D"));
    _hdrRenderStage = new HdrRenderStage(glProgramManager->getProgram("HDR"), _viewport);
}

void MainMenuRenderPipeline::render() {
 
    // Bind the FBO
 //   _hdrFrameBuffer->bind();
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    // Main render passes
    _spaceRenderStage->draw();
 //   _planetRenderStage->draw();
 //   glDisable(GL_CULL_FACE);
    _awesomiumRenderStage->draw();
 //   glEnable(GL_CULL_FACE);
    // Post processing
 //   _hdrRenderStage->setState(_hdrFrameBuffer);
 //   _hdrRenderStage->draw();

    glEnable(GL_DEPTH_TEST);

    checkGlError("LAWL");
}

void MainMenuRenderPipeline::destroy() {
    delete _spaceRenderStage;
    _spaceRenderStage = nullptr;

    delete _planetRenderStage;
    _planetRenderStage = nullptr;

    delete _awesomiumRenderStage;
    _awesomiumRenderStage = nullptr;

    delete _hdrRenderStage;
    _hdrRenderStage = nullptr;

    delete _hdrFrameBuffer;
    _hdrFrameBuffer = nullptr;
}

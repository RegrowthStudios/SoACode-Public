#include "stdafx.h"
#include "GamePlayRenderPipeline.h"
#include "Errors.h"
#include "SkyboxRenderStage.h"
#include "OpaqueVoxelRenderStage.h"
#include "PlanetRenderStage.h"
#include "HdrRenderStage.h"
#include "MeshManager.h"
#include "FrameBuffer.h"
#include "Options.h"
#include "Camera.h"


GamePlayRenderPipeline::GamePlayRenderPipeline() :
    _skyboxRenderStage(nullptr),
    _planetRenderStage(nullptr),
    _opaqueVoxelRenderStage(nullptr),
    _awesomiumRenderStage(nullptr),
    _hdrRenderStage(nullptr),
    _hdrFrameBuffer(nullptr)
{
    // Empty
}

void GamePlayRenderPipeline::init(const ui32v4& viewport, Camera* chunkCamera, Camera* worldCamera, MeshManager* meshManager, vg::GLProgramManager* glProgramManager) {
    // Set the viewport
    _viewport = viewport;

    // Set cameras
    _worldCamera = worldCamera;
    _chunkCamera = chunkCamera;

    // Check to make sure we aren't leaking memory
    if (_skyboxRenderStage != nullptr) {
        pError("Reinitializing GamePlayRenderPipeline without first calling destroy()!");
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
    _opaqueVoxelRenderStage = new OpaqueVoxelRenderStage(_chunkCamera, &_gameRenderParams, meshManager);
    _hdrRenderStage = new HdrRenderStage(glProgramManager->getProgram("HDR"), _viewport);
}

GamePlayRenderPipeline::~GamePlayRenderPipeline() {
    destroy();
}

void GamePlayRenderPipeline::render() {
    // Set up the gameRenderParams
    _gameRenderParams.calculateParams(_worldCamera->position(), false);
    // Bind the FBO
    _hdrFrameBuffer->bind();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // worldCamera passes
    _skyboxRenderStage->draw();
    _planetRenderStage->draw();

    // Clear the depth buffer so we can draw the voxel passes
    glClear(GL_DEPTH_BUFFER_BIT);

    // chunkCamera passes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.ID);
    _opaqueVoxelRenderStage->draw();

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

#include "stdafx.h"
#include "GamePlayRenderPipeline.h"
#include "Errors.h"
#include "SkyboxRenderStage.h"
#include "OpaqueVoxelRenderStage.h"
#include "CutoutVoxelRenderStage.h"
#include "LiquidVoxelRenderStage.h"
#include "DevHudRenderStage.h"
#include "TransparentVoxelRenderStage.h"
#include "PlanetRenderStage.h"
#include "HdrRenderStage.h"
#include "MeshManager.h"
#include "FrameBuffer.h"
#include "Options.h"
#include "Camera.h"

#define DEVHUD_FONT_SIZE 32

GamePlayRenderPipeline::GamePlayRenderPipeline() :
    _skyboxRenderStage(nullptr),
    _planetRenderStage(nullptr),
    _opaqueVoxelRenderStage(nullptr),
    _cutoutVoxelRenderStage(nullptr),
    _transparentVoxelRenderStage(nullptr),
    _liquidVoxelRenderStage(nullptr),
    _devHudRenderStage(nullptr),
    _awesomiumRenderStage(nullptr),
    _hdrRenderStage(nullptr),
    _hdrFrameBuffer(nullptr) {
    // Empty
}

void GamePlayRenderPipeline::init(const ui32v4& viewport, Camera* chunkCamera,
                                  const Camera* worldCamera, const App* app,
                                  const Player* player, const MeshManager* meshManager,
                                  const vg::GLProgramManager* glProgramManager) {
    // Set the viewport
    _viewport = viewport;

    // Hold the world camera
    _worldCamera = worldCamera;

    // Check to make sure we aren't leaking memory
    if (_skyboxRenderStage != nullptr) {
        pError("Reinitializing GamePlayRenderPipeline without first calling destroy()!");
    }

    // Construct frame buffer
    if (graphicsOptions.msaa > 0) {
        glEnable(GL_MULTISAMPLE);
        _hdrFrameBuffer = new vg::FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT,
                                              _viewport.z, _viewport.w,
                                              graphicsOptions.msaa);
    } else {
        glDisable(GL_MULTISAMPLE);
        _hdrFrameBuffer = new vg::FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT,
                                              _viewport.z, _viewport.w);
    }

    // Get window dimensions
    f32v2 windowDims(_viewport.z, _viewport.w);

    // Init render stages
    _skyboxRenderStage = new SkyboxRenderStage(glProgramManager->getProgram("Texture"), _worldCamera);
    _planetRenderStage = new PlanetRenderStage(_worldCamera);
    _opaqueVoxelRenderStage = new OpaqueVoxelRenderStage(chunkCamera, &_gameRenderParams, meshManager);
    _cutoutVoxelRenderStage = new CutoutVoxelRenderStage(chunkCamera, &_gameRenderParams, meshManager);
    _transparentVoxelRenderStage = new TransparentVoxelRenderStage(chunkCamera, &_gameRenderParams, meshManager);
    _liquidVoxelRenderStage = new LiquidVoxelRenderStage(chunkCamera, &_gameRenderParams, meshManager);
    _devHudRenderStage = new DevHudRenderStage("Fonts\\chintzy.ttf", DEVHUD_FONT_SIZE, player, app, windowDims);
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

    // Bind voxel texture pack
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.ID);

    // chunkCamera passes
    _opaqueVoxelRenderStage->draw();
    _cutoutVoxelRenderStage->draw();
    _liquidVoxelRenderStage->draw();
    _transparentVoxelRenderStage->draw();

    // Dev hud
    _devHudRenderStage->draw();

    // Post processing
    _hdrRenderStage->setInputFbo(_hdrFrameBuffer);
    _hdrRenderStage->draw();

    // Check for errors, just in case
    checkGlError("GamePlayRenderPipeline::render()");
}

void GamePlayRenderPipeline::destroy() {
    // Make sure everything is freed here!
    delete _skyboxRenderStage;
    _skyboxRenderStage = nullptr;

    delete _planetRenderStage;
    _planetRenderStage = nullptr;

    delete _opaqueVoxelRenderStage;
    _opaqueVoxelRenderStage = nullptr;

    delete _cutoutVoxelRenderStage;
    _cutoutVoxelRenderStage = nullptr;

    delete _transparentVoxelRenderStage;
    _transparentVoxelRenderStage = nullptr;

    delete _liquidVoxelRenderStage;
    _liquidVoxelRenderStage = nullptr;

    delete _devHudRenderStage;
    _devHudRenderStage = nullptr;

    delete _awesomiumRenderStage;
    _awesomiumRenderStage = nullptr;

    delete _hdrRenderStage;
    _hdrRenderStage = nullptr;

    delete _hdrFrameBuffer;
    _hdrFrameBuffer = nullptr;
}

void GamePlayRenderPipeline::cycleDevHud(int offset /* = 1 */) {
    _devHudRenderStage->cycleMode(offset);
}
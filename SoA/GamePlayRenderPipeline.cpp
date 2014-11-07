#include "stdafx.h"

#include "Camera.h"
#include "CutoutVoxelRenderStage.h"
#include "DevHudRenderStage.h"
#include "Errors.h"
#include "FrameBuffer.h"
#include "GamePlayRenderPipeline.h"
#include "HdrRenderStage.h"
#include "LiquidVoxelRenderStage.h"
#include "MeshManager.h"
#include "OpaqueVoxelRenderStage.h"
#include "Options.h"
#include "PdaRenderStage.h"
#include "PlanetRenderStage.h"
#include "SkyboxRenderStage.h"
#include "TransparentVoxelRenderStage.h"

#define DEVHUD_FONT_SIZE 32

GamePlayRenderPipeline::GamePlayRenderPipeline() {
    // Empty
}

void GamePlayRenderPipeline::init(const ui32v4& viewport, Camera* chunkCamera,
                                  const Camera* worldCamera, const App* app,
                                  const Player* player, const MeshManager* meshManager,
                                  const PDA* pda, const vg::GLProgramManager* glProgramManager) {
    // Set the viewport
    _viewport = viewport;

    // Grab mesh manager handle
    _meshManager = meshManager;

    // Get the camera handles
    _worldCamera = worldCamera;
    _chunkCamera = chunkCamera;

    // Check to make sure we aren't leaking memory
    if (_skyboxRenderStage != nullptr) {
        pError("Reinitializing GamePlayRenderPipeline without first calling destroy()!");
    }

    // Construct framebuffer
    _hdrFrameBuffer = new vg::GLRenderTarget(_viewport.z, _viewport.w);
    _hdrFrameBuffer->init(vg::TextureInternalFormat::RGBA16F, vg::TextureInternalFormat::DEPTH_COMPONENT32, graphicsOptions.msaa);
    if (graphicsOptions.msaa > 0) {
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }

    // Make swap chain
    _swapChain = new vg::RTSwapChain<2>(_viewport.z, _viewport.w);
    _swapChain->init(vg::TextureInternalFormat::RGBA8);
    _quad.init();

    // Get window dimensions
    f32v2 windowDims(_viewport.z, _viewport.w);

    // Init render stages
    _skyboxRenderStage = new SkyboxRenderStage(glProgramManager->getProgram("Texture"), _worldCamera);
    _planetRenderStage = new PlanetRenderStage(_worldCamera);
    _opaqueVoxelRenderStage = new OpaqueVoxelRenderStage(&_gameRenderParams);
    _cutoutVoxelRenderStage = new CutoutVoxelRenderStage(&_gameRenderParams);
    _transparentVoxelRenderStage = new TransparentVoxelRenderStage(&_gameRenderParams);
    _liquidVoxelRenderStage = new LiquidVoxelRenderStage(&_gameRenderParams);
    _devHudRenderStage = new DevHudRenderStage("Fonts\\chintzy.ttf", DEVHUD_FONT_SIZE, player, app, windowDims);
    _pdaRenderStage = new PdaRenderStage(pda);
    _hdrRenderStage = new HdrRenderStage(glProgramManager->getProgram("HDR"), &_quad);
}

GamePlayRenderPipeline::~GamePlayRenderPipeline() {
    destroy();
}

void GamePlayRenderPipeline::render() {
    // Set up the gameRenderParams
    _gameRenderParams.calculateParams(_worldCamera->getPosition(), _chunkCamera, &_meshManager->getChunkMeshes(), false);
    // Bind the FBO
    _hdrFrameBuffer->use();
  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    // PDA
    _pdaRenderStage->draw();

    // Post processing
    _swapChain->reset(0, _hdrFrameBuffer, graphicsOptions.msaa > 0, false);

    // TODO: More Effects

    // Draw to backbuffer for the last effect
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
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

    delete _pdaRenderStage;
    _pdaRenderStage = nullptr;

    delete _hdrRenderStage;
    _hdrRenderStage = nullptr;

    _hdrFrameBuffer->dispose();
    delete _hdrFrameBuffer;
    _hdrFrameBuffer = nullptr;

    _swapChain->dispose();
    delete _swapChain;
    _swapChain = nullptr;

    _quad.dispose();
}

void GamePlayRenderPipeline::cycleDevHud(int offset /* = 1 */) {
    _devHudRenderStage->cycleMode(offset);
}
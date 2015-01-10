#include "stdafx.h"
#include "GamePlayRenderPipeline.h"

#include <Vorb/GLStates.h>

#include "Camera.h"
#include "ChunkGridRenderStage.h"
#include "CutoutVoxelRenderStage.h"
#include "DevHudRenderStage.h"
#include "Errors.h"
#include "HdrRenderStage.h"
#include "LiquidVoxelRenderStage.h"
#include "MeshManager.h"
#include "NightVisionRenderStage.h"
#include "OpaqueVoxelRenderStage.h"
#include "Options.h"
#include "PauseMenu.h"
#include "PauseMenuRenderStage.h"
#include "PdaRenderStage.h"
#include "PhysicsBlockRenderStage.h"
#include "SkyboxRenderStage.h"
#include "SpaceSystemRenderStage.h"
#include "TransparentVoxelRenderStage.h"

#define DEVHUD_FONT_SIZE 32

GamePlayRenderPipeline::GamePlayRenderPipeline() :
    m_drawMode(GL_FILL) {
    // Empty
}

void GamePlayRenderPipeline::init(const ui32v4& viewport, Camera* chunkCamera,
                                  const Camera* worldCamera, const App* app,
                                  const Player* player, const MeshManager* meshManager,
                                  const PDA* pda, const vg::GLProgramManager* glProgramManager,
                                  SpaceSystem* spaceSystem,
                                  const PauseMenu* pauseMenu, const std::vector<ChunkSlot>& chunkSlots) {
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
    _hdrFrameBuffer->init(vg::TextureInternalFormat::RGBA16F, graphicsOptions.msaa, vg::TextureFormat::RGBA, vg::TexturePixelType::HALF_FLOAT).initDepth();
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
    _physicsBlockRenderStage = new PhysicsBlockRenderStage(&_gameRenderParams, _meshManager->getPhysicsBlockMeshes(), glProgramManager->getProgram("PhysicsBlock"));
    _opaqueVoxelRenderStage = new OpaqueVoxelRenderStage(&_gameRenderParams);
    _cutoutVoxelRenderStage = new CutoutVoxelRenderStage(&_gameRenderParams);
    _chunkGridRenderStage = new ChunkGridRenderStage(&_gameRenderParams, chunkSlots);
    _transparentVoxelRenderStage = new TransparentVoxelRenderStage(&_gameRenderParams);
    _liquidVoxelRenderStage = new LiquidVoxelRenderStage(&_gameRenderParams);
    _devHudRenderStage = new DevHudRenderStage("Fonts\\chintzy.ttf", DEVHUD_FONT_SIZE, player, app, windowDims);
    _pdaRenderStage = new PdaRenderStage(pda);
    _pauseMenuRenderStage = new PauseMenuRenderStage(pauseMenu);
    _nightVisionRenderStage = new NightVisionRenderStage(glProgramManager->getProgram("NightVision"), &_quad);
    _hdrRenderStage = new HdrRenderStage(glProgramManager, &_quad, _chunkCamera);
    m_spaceSystemRenderStage = new SpaceSystemRenderStage(ui32v2(windowDims),
                                                          spaceSystem, nullptr, _worldCamera,
                                                          glProgramManager->getProgram("BasicColor"),
                                                          glProgramManager->getProgram("SphericalTerrain"),
                                                          glProgramManager->getProgram("SphericalWater"),
                                                          GameManager::textureCache->addTexture("Textures/selector.png").id);

    loadNightVision();
    // No post-process effects to begin with
    _nightVisionRenderStage->setIsVisible(false);
    _chunkGridRenderStage->setIsVisible(false);
}

GamePlayRenderPipeline::~GamePlayRenderPipeline() {
    destroy();
}

void GamePlayRenderPipeline::render() {
    // Set up the gameRenderParams
    _gameRenderParams.calculateParams(_worldCamera->getPosition(), _chunkCamera,
                                      &_meshManager->getChunkMeshes(), false);
    // Bind the FBO
    _hdrFrameBuffer->use();
  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // worldCamera passes
    _skyboxRenderStage->draw();

    // Clear the depth buffer so we can draw the voxel passes
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    glPolygonMode(GL_FRONT_AND_BACK, m_drawMode);
    _opaqueVoxelRenderStage->draw();
    _physicsBlockRenderStage->draw();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _cutoutVoxelRenderStage->draw();
    _chunkGridRenderStage->draw();
    _liquidVoxelRenderStage->draw();
    _transparentVoxelRenderStage->draw();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Post processing
    _swapChain->reset(0, _hdrFrameBuffer, graphicsOptions.msaa > 0, false);

    // TODO: More Effects
    if (_nightVisionRenderStage->isVisible()) {
        _nightVisionRenderStage->draw();
        _swapChain->swap();
        _swapChain->use(0, false);
    }

    // Draw to backbuffer for the last effect
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(_hdrFrameBuffer->getTextureTarget(), _hdrFrameBuffer->getTextureDepthID());
    _hdrRenderStage->draw();

    // UI
    _devHudRenderStage->draw();
    _pdaRenderStage->draw();
    _pauseMenuRenderStage->draw();

    // Check for errors, just in case
    checkGlError("GamePlayRenderPipeline::render()");
}

void GamePlayRenderPipeline::destroy() {
    // Make sure everything is freed here!
    delete _skyboxRenderStage;
    _skyboxRenderStage = nullptr;


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

    delete _pauseMenuRenderStage;
    _pauseMenuRenderStage = nullptr;

    delete _nightVisionRenderStage;
    _nightVisionRenderStage = nullptr;

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

void GamePlayRenderPipeline::toggleNightVision() {
    if (!_nightVisionRenderStage->isVisible()) {
        _nightVisionRenderStage->setIsVisible(true);
        _nvIndex = 0;
        _nightVisionRenderStage->setParams(&_nvParams[_nvIndex]);
    } else {
        _nvIndex++;
        if (_nvIndex >= _nvParams.size()) {
            _nightVisionRenderStage->setIsVisible(false);
        } else {
            _nightVisionRenderStage->setParams(&_nvParams[_nvIndex]);
        }
    }
}
void GamePlayRenderPipeline::loadNightVision() {
    _nightVisionRenderStage->setIsVisible(false);

    _nvIndex = 0;
    _nvParams.clear();

    vio::IOManager iom;
    const cString nvData = iom.readFileToString("Data/NightVision.yml");
    if (nvData) {
        Array<NightVisionRenderParams> arr;
        YAML::Node node = YAML::Load(nvData);
        Keg::Value v = Keg::Value::array(0, Keg::Value::custom("NightVisionRenderParams", 0, false));
        Keg::evalData((ui8*)&arr, &v, node, Keg::getGlobalEnvironment());
        for (i32 i = 0; i < arr.getLength(); i++) {
            _nvParams.push_back(arr[i]);
        }
        delete[] nvData;
    }
    if (_nvParams.size() < 1) {
        _nvParams.push_back(NightVisionRenderParams::createDefault());
    }
}

void GamePlayRenderPipeline::toggleChunkGrid() {
    _chunkGridRenderStage->setIsVisible(!_chunkGridRenderStage->isVisible());
}

void GamePlayRenderPipeline::cycleDrawMode() {
    switch (m_drawMode) {
    case GL_FILL:
        m_drawMode = GL_LINE;
        break;
    case GL_LINE:
        m_drawMode = GL_POINT;
        break;
    case GL_POINT:
    default:
        m_drawMode = GL_FILL;
        break;
    }
}

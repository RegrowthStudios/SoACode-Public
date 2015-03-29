#include "stdafx.h"
#include "GameplayRenderPipeline.h"

#include <Vorb/graphics/GLStates.h>

#include "ChunkGridRenderStage.h"
#include "ChunkMemoryManager.h"
#include "ChunkMeshManager.h"
#include "CutoutVoxelRenderStage.h"
#include "DevHudRenderStage.h"
#include "Errors.h"
#include "GameSystem.h"
#include "HdrRenderStage.h"
#include "LiquidVoxelRenderStage.h"
#include "MTRenderState.h"
#include "MeshManager.h"
#include "NightVisionRenderStage.h"
#include "OpaqueVoxelRenderStage.h"
#include "Options.h"
#include "PauseMenu.h"
#include "PauseMenuRenderStage.h"
#include "PdaRenderStage.h"
#include "PhysicsBlockRenderStage.h"
#include "SkyboxRenderStage.h"
#include "SoAState.h"
#include "SpaceSystem.h"
#include "SpaceSystemRenderStage.h"
#include "TransparentVoxelRenderStage.h"
#include "soaUtils.h"

#define DEVHUD_FONT_SIZE 32

GameplayRenderPipeline::GameplayRenderPipeline() :
    m_drawMode(GL_FILL) {
    // Empty
}

void GameplayRenderPipeline::init(const ui32v4& viewport, const SoaState* soaState, const App* app,
                                  const PDA* pda,
                                  SpaceSystem* spaceSystem,
                                  GameSystem* gameSystem,
                                  const PauseMenu* pauseMenu) {
    // Set the viewport
    _viewport = viewport;

    m_soaState = soaState;

    // Grab mesh manager handle
    _meshManager = soaState->chunkMeshManager.get();

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

    m_spaceCamera.setAspectRatio(windowDims.x / windowDims.y);
    m_voxelCamera.setAspectRatio(windowDims.x / windowDims.y);

    // Set up shared params
    const vg::GLProgramManager* glProgramManager = soaState->glProgramManager.get();
    _gameRenderParams.glProgramManager = glProgramManager;

    // Init render stages
    _skyboxRenderStage = new SkyboxRenderStage(glProgramManager->getProgram("Texture"), &m_spaceCamera);
    //_physicsBlockRenderStage = new PhysicsBlockRenderStage(&_gameRenderParams, _meshManager->getPhysicsBlockMeshes(), glProgramManager->getProgram("PhysicsBlock"));
    _opaqueVoxelRenderStage = new OpaqueVoxelRenderStage(&_gameRenderParams);
    _cutoutVoxelRenderStage = new CutoutVoxelRenderStage(&_gameRenderParams);
    _chunkGridRenderStage = new ChunkGridRenderStage(&_gameRenderParams);
    _transparentVoxelRenderStage = new TransparentVoxelRenderStage(&_gameRenderParams);
    _liquidVoxelRenderStage = new LiquidVoxelRenderStage(&_gameRenderParams);
    _devHudRenderStage = new DevHudRenderStage("Fonts\\chintzy.ttf", DEVHUD_FONT_SIZE, app, windowDims);
    _pdaRenderStage = new PdaRenderStage(pda);
    _pauseMenuRenderStage = new PauseMenuRenderStage(pauseMenu);
    _nightVisionRenderStage = new NightVisionRenderStage(glProgramManager->getProgram("NightVision"), &_quad);
    _hdrRenderStage = new HdrRenderStage(glProgramManager, &_quad, &m_voxelCamera);
    m_spaceSystemRenderStage = new SpaceSystemRenderStage(ui32v2(windowDims),
                                                          spaceSystem,
                                                          gameSystem,
                                                          nullptr, &m_spaceCamera,
                                                          &m_farTerrainCamera,
                                                          GameManager::textureCache->addTexture("Textures/selector.png").id);

    loadNightVision();
    // No post-process effects to begin with
    _nightVisionRenderStage->setIsVisible(false);
    _chunkGridRenderStage->setIsVisible(false);
}

void GameplayRenderPipeline::setRenderState(const MTRenderState* renderState) {
    m_renderState = renderState;
}

GameplayRenderPipeline::~GameplayRenderPipeline() {
    destroy(true);
}

void GameplayRenderPipeline::render() {
    const GameSystem* gameSystem = m_soaState->gameSystem.get();
    const SpaceSystem* spaceSystem = m_soaState->spaceSystem.get();

    updateCameras();
    // Set up the gameRenderParams
    _gameRenderParams.calculateParams(m_spaceCamera.getPosition(), &m_voxelCamera,
                                      _meshManager, false);
    // Bind the FBO
    _hdrFrameBuffer->use();
  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // worldCamera passes
    _skyboxRenderStage->render();
    glPolygonMode(GL_FRONT_AND_BACK, m_drawMode);
    m_spaceSystemRenderStage->setRenderState(m_renderState);
    m_spaceSystemRenderStage->render();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Check for face transition animation state
    if (m_spaceSystemRenderStage->needsFaceTransitionAnimation) {
        m_spaceSystemRenderStage->needsFaceTransitionAnimation = false;
        m_increaseQuadAlpha = true;
        m_coloredQuadAlpha = 0.0f;
    }

    // Clear the depth buffer so we can draw the voxel passes
    glClear(GL_DEPTH_BUFFER_BIT);

    if (m_voxelsActive) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, m_drawMode);
        _opaqueVoxelRenderStage->render();
       // _physicsBlockRenderStage->draw();
        _cutoutVoxelRenderStage->render();

        auto& voxcmp = gameSystem->voxelPosition.getFromEntity(m_soaState->playerEntity).parentVoxelComponent;
        _chunkGridRenderStage->setChunks(spaceSystem->m_sphericalVoxelCT.get(voxcmp).chunkMemoryManager);
        _chunkGridRenderStage->render();
        _liquidVoxelRenderStage->render();
        _transparentVoxelRenderStage->render();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Post processing
    _swapChain->reset(0, _hdrFrameBuffer, graphicsOptions.msaa > 0, false);

    // TODO: More Effects
    if (_nightVisionRenderStage->isVisible()) {
        _nightVisionRenderStage->render();
        _swapChain->swap();
        _swapChain->use(0, false);
    }

    // Draw to backbuffer for the last effect
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(_hdrFrameBuffer->getTextureTarget(), _hdrFrameBuffer->getTextureDepthID());
    _hdrRenderStage->render();

    // UI
    _devHudRenderStage->render();
    _pdaRenderStage->render();
    _pauseMenuRenderStage->render();

    // Cube face fade animation
    if (m_increaseQuadAlpha) {
        static const f32 FADE_INC = 0.07f;
        m_coloredQuadAlpha += FADE_INC;
        if (m_coloredQuadAlpha >= 3.5f) {
            m_coloredQuadAlpha = 3.5f;
            m_increaseQuadAlpha = false;
        }
        m_coloredQuadRenderer.draw(_quad, f32v4(0.0, 0.0, 0.0, glm::min(m_coloredQuadAlpha, 1.0f)));
    } else if (m_coloredQuadAlpha > 0.0f) {
        static const float FADE_DEC = 0.01f;  
        m_coloredQuadRenderer.draw(_quad, f32v4(0.0, 0.0, 0.0, glm::min(m_coloredQuadAlpha, 1.0f)));
        m_coloredQuadAlpha -= FADE_DEC;
    }

    // Check for errors, just in case
    checkGlError("GamePlayRenderPipeline::render()");
}

void GameplayRenderPipeline::destroy(bool shouldDisposeStages) {
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

void GameplayRenderPipeline::cycleDevHud(int offset /* = 1 */) {
    _devHudRenderStage->cycleMode(offset);
}

void GameplayRenderPipeline::toggleNightVision() {
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
void GameplayRenderPipeline::loadNightVision() {
    _nightVisionRenderStage->setIsVisible(false);

    _nvIndex = 0;
    _nvParams.clear();

    vio::IOManager iom;
    const cString nvData = iom.readFileToString("Data/NightVision.yml");
    if (nvData) {
        Array<NightVisionRenderParams> arr;
        keg::YAMLReader reader;
        reader.init(nvData);
        keg::Node node = reader.getFirst();
        keg::Value v = keg::Value::array(0, keg::Value::custom(0, "NightVisionRenderParams", false));
        keg::evalData((ui8*)&arr, &v, node, reader, keg::getGlobalEnvironment());
        for (i32 i = 0; i < arr.size(); i++) {
            _nvParams.push_back(arr[i]);
        }
        reader.dispose();
        delete[] nvData;
    }
    if (_nvParams.size() < 1) {
        _nvParams.push_back(NightVisionRenderParams::createDefault());
    }
}

void GameplayRenderPipeline::toggleChunkGrid() {
    _chunkGridRenderStage->setIsVisible(!_chunkGridRenderStage->isVisible());
}

void GameplayRenderPipeline::cycleDrawMode() {
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

void GameplayRenderPipeline::updateCameras() {
    const GameSystem* gs = m_soaState->gameSystem.get();
    const SpaceSystem* ss = m_soaState->spaceSystem.get();

    // For dynamic clipping plane to minimize Z fighting
    float dynamicZNear = m_spaceSystemRenderStage->getDynamicNearPlane(m_spaceCamera.getFieldOfView(),
                                                                       m_spaceCamera.getAspectRatio());
    if (dynamicZNear < 0.1f) dynamicZNear = 0.1f; // Clamp to 1/10 KM

    // Get the physics component
    auto& phycmp = gs->physics.getFromEntity(m_soaState->playerEntity);
    if (phycmp.voxelPositionComponent) {
        auto& vpcmp = gs->voxelPosition.get(phycmp.voxelPositionComponent);
        m_voxelCamera.setClippingPlane(0.1f, 10000.0f);
        m_voxelCamera.setPosition(vpcmp.gridPosition.pos);
        m_voxelCamera.setOrientation(vpcmp.orientation);
        m_voxelCamera.update();

        // Far terrain camera is exactly like voxel camera except for clip plane
        m_farTerrainCamera = m_voxelCamera;
        m_farTerrainCamera.setClippingPlane(dynamicZNear, 100000.0f);
        m_farTerrainCamera.update();

        m_voxelsActive = true;
    } else {
        m_voxelsActive = false;
    }
    // Player is relative to a planet, so add position if needed
    auto& spcmp = gs->spacePosition.get(phycmp.spacePositionComponent);
    if (spcmp.parentGravityID) {
        auto& it = m_renderState->spaceBodyPositions.find(spcmp.parentEntityID);
        if (it != m_renderState->spaceBodyPositions.end()) {
            m_spaceCamera.setPosition(m_renderState->spaceCameraPos + it->second);
        } else {
            auto& gcmp = ss->m_sphericalGravityCT.get(spcmp.parentGravityID);
            auto& npcmp = ss->m_namePositionCT.get(gcmp.namePositionComponent);
            m_spaceCamera.setPosition(m_renderState->spaceCameraPos + npcmp.position);
        }
    } else {
        m_spaceCamera.setPosition(m_renderState->spaceCameraPos);
    }
    //printVec("POSITION: ", spcmp.position);
    m_spaceCamera.setClippingPlane(dynamicZNear, 100000000000.0f);
   
    m_spaceCamera.setOrientation(m_renderState->spaceCameraOrientation);
    m_spaceCamera.update();
}
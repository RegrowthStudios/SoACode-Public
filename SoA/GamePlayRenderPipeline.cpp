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
    m_viewport = viewport;

    m_soaState = soaState;

    // Grab mesh manager handle
    m_meshManager = soaState->chunkMeshManager.get();

    // Check to make sure we aren't leaking memory
    if (m_skyboxRenderStage != nullptr) {
        pError("Reinitializing GamePlayRenderPipeline without first calling destroy()!");
    }

    // Construct framebuffer
    m_hdrFrameBuffer = new vg::GLRenderTarget(m_viewport.z, m_viewport.w);
    m_hdrFrameBuffer->init(vg::TextureInternalFormat::RGBA16F, graphicsOptions.msaa, vg::TextureFormat::RGBA, vg::TexturePixelType::HALF_FLOAT).initDepth();
    if (graphicsOptions.msaa > 0) {
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }

    // Make swap chain
    m_swapChain = new vg::RTSwapChain<2>();
    m_swapChain->init(m_viewport.z, m_viewport.w, vg::TextureInternalFormat::RGBA8);
    m_quad.init();

    // Get window dimensions
    f32v2 windowDims(m_viewport.z, m_viewport.w);

    m_spaceCamera.setAspectRatio(windowDims.x / windowDims.y);
    m_voxelCamera.setAspectRatio(windowDims.x / windowDims.y);

    // Helpful macro to reduce code size
#define ADD_STAGE(type, ...) static_cast<type*>(addStage(std::make_shared<type>(__VA_ARGS__)))

    // Init and track render stages
    m_skyboxRenderStage = ADD_STAGE(SkyboxRenderStage, &m_spaceCamera, &soaState->texturePathResolver);
    m_opaqueVoxelRenderStage = ADD_STAGE(OpaqueVoxelRenderStage, &m_gameRenderParams);
    m_cutoutVoxelRenderStage = ADD_STAGE(CutoutVoxelRenderStage, &m_gameRenderParams);
    m_chunkGridRenderStage = ADD_STAGE(ChunkGridRenderStage, &m_gameRenderParams);
    m_transparentVoxelRenderStage = ADD_STAGE(TransparentVoxelRenderStage, &m_gameRenderParams);
    m_liquidVoxelRenderStage = ADD_STAGE(LiquidVoxelRenderStage, &m_gameRenderParams);
    m_devHudRenderStage = ADD_STAGE(DevHudRenderStage, "Fonts\\chintzy.ttf", DEVHUD_FONT_SIZE, app, windowDims);
    m_pdaRenderStage = ADD_STAGE(PdaRenderStage, pda);
    m_pauseMenuRenderStage = ADD_STAGE(PauseMenuRenderStage, pauseMenu);
    m_nightVisionRenderStage = ADD_STAGE(NightVisionRenderStage, &m_quad);
    m_hdrRenderStage = ADD_STAGE(HdrRenderStage, &m_quad, &m_voxelCamera);
    m_spaceSystemRenderStage = ADD_STAGE(SpaceSystemRenderStage, soaState, ui32v2(windowDims),
        spaceSystem, gameSystem,
        nullptr, &m_spaceCamera,
        &m_farTerrainCamera);

    loadNightVision();
    // No post-process effects to begin with
    m_nightVisionRenderStage->setIsVisible(false);
    m_chunkGridRenderStage->setIsVisible(false);
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
    m_gameRenderParams.calculateParams(m_spaceCamera.getPosition(), &m_voxelCamera,
                                      m_meshManager, false);
    // Bind the FBO
    m_hdrFrameBuffer->use();
  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // worldCamera passes
    m_skyboxRenderStage->render();
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
        m_opaqueVoxelRenderStage->render();
       // _physicsBlockRenderStage->draw();
        m_cutoutVoxelRenderStage->render();

        auto& voxcmp = gameSystem->voxelPosition.getFromEntity(m_soaState->playerEntity).parentVoxelComponent;
        m_chunkGridRenderStage->setChunks(spaceSystem->m_sphericalVoxelCT.get(voxcmp).chunkMemoryManager);
        m_chunkGridRenderStage->render();
        m_liquidVoxelRenderStage->render();
        m_transparentVoxelRenderStage->render();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Post processing
    m_swapChain->reset(0, m_hdrFrameBuffer->getID(), m_hdrFrameBuffer->getTextureID(), graphicsOptions.msaa > 0, false);

    // TODO: More Effects
    if (m_nightVisionRenderStage->isVisible()) {
        m_nightVisionRenderStage->render();
        m_swapChain->swap();
        m_swapChain->use(0, false);
    }

    // Draw to backbuffer for the last effect
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(m_hdrFrameBuffer->getTextureTarget(), m_hdrFrameBuffer->getTextureDepthID());
    m_hdrRenderStage->render();

    // UI
    m_devHudRenderStage->render();
    m_pdaRenderStage->render();
    m_pauseMenuRenderStage->render();

    // Cube face fade animation
    if (m_increaseQuadAlpha) {
        static const f32 FADE_INC = 0.07f;
        m_coloredQuadAlpha += FADE_INC;
        if (m_coloredQuadAlpha >= 3.5f) {
            m_coloredQuadAlpha = 3.5f;
            m_increaseQuadAlpha = false;
        }
        m_coloredQuadRenderer.draw(m_quad, f32v4(0.0, 0.0, 0.0, glm::min(m_coloredQuadAlpha, 1.0f)));
    } else if (m_coloredQuadAlpha > 0.0f) {
        static const float FADE_DEC = 0.01f;  
        m_coloredQuadRenderer.draw(m_quad, f32v4(0.0, 0.0, 0.0, glm::min(m_coloredQuadAlpha, 1.0f)));
        m_coloredQuadAlpha -= FADE_DEC;
    }

    if (m_shouldScreenshot) dumpScreenshot();

    // Check for errors, just in case
    checkGlError("GamePlayRenderPipeline::render()");
}

void GameplayRenderPipeline::destroy(bool shouldDisposeStages) {
    
    // Call base destroy
    RenderPipeline::destroy(shouldDisposeStages);

    m_hdrFrameBuffer->dispose();
    delete m_hdrFrameBuffer;
    m_hdrFrameBuffer = nullptr;

    m_swapChain->dispose();
    delete m_swapChain;
    m_swapChain = nullptr;

    m_quad.dispose();
}

void GameplayRenderPipeline::cycleDevHud(int offset /* = 1 */) {
    m_devHudRenderStage->cycleMode(offset);
}

void GameplayRenderPipeline::toggleNightVision() {
    if (!m_nightVisionRenderStage->isVisible()) {
        m_nightVisionRenderStage->setIsVisible(true);
        m_nvIndex = 0;
        m_nightVisionRenderStage->setParams(m_nvParams[m_nvIndex]);
    } else {
        m_nvIndex++;
        if (m_nvIndex >= m_nvParams.size()) {
            m_nightVisionRenderStage->setIsVisible(false);
        } else {
            m_nightVisionRenderStage->setParams(m_nvParams[m_nvIndex]);
        }
    }
}

void GameplayRenderPipeline::loadNightVision() {
    m_nightVisionRenderStage->setIsVisible(false);

    m_nvIndex = 0;
    m_nvParams.clear();

    vio::IOManager iom;
    const cString nvData = iom.readFileToString("Data/NightVision.yml");
    if (nvData) {
        Array<NightVisionRenderParams> arr;
        keg::ReadContext context;
        context.env = keg::getGlobalEnvironment();
        context.reader.init(nvData);
        keg::Node node = context.reader.getFirst();
        keg::Value v = keg::Value::array(0, keg::Value::custom(0, "NightVisionRenderParams", false));
        keg::evalData((ui8*)&arr, &v, node, context);
        for (i32 i = 0; i < arr.size(); i++) {
            m_nvParams.push_back(arr[i]);
        }
        context.reader.dispose();
        delete[] nvData;
    }
    if (m_nvParams.size() < 1) {
        m_nvParams.push_back(NightVisionRenderParams::createDefault());
    }
}

void GameplayRenderPipeline::toggleChunkGrid() {
    m_chunkGridRenderStage->setIsVisible(!m_chunkGridRenderStage->isVisible());
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

void GameplayRenderPipeline::dumpScreenshot() {
    // Make screenshots directory
    vio::IOManager().makeDirectory("Screenshots");
    // Take screenshot
    dumpFramebufferImage("Screenshots/", m_viewport);
    m_shouldScreenshot = false;
}

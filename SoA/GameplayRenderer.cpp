#include "stdafx.h"
#include "GameplayRenderer.h"

#include <Vorb/graphics/GLStates.h>
#include <Vorb/ui/GameWindow.h>
#include <Vorb/AssetLoader.h>

#include "ChunkMeshManager.h"
#include "CommonState.h"
#include "Errors.h"
#include "GameSystem.h"
#include "GameplayScreen.h"
#include "MTRenderState.h"
#include "MeshManager.h"
#include "PauseMenu.h"
#include "SoAState.h"
#include "SoaOptions.h"
#include "SpaceSystem.h"
#include "soaUtils.h"

#define DEVHUD_FONT_SIZE 32

void GameplayRenderer::init(vui::GameWindow* window, StaticLoadContext& context,
                            GameplayScreen* gameplayScreen, CommonState* commonState) {
    m_window = window;
    m_gameplayScreen = gameplayScreen;
    m_commonState = commonState;
    m_state = m_commonState->state;

    // TODO(Ben): Dis is bad mkay
    m_viewport = f32v4(0, 0, m_window->getWidth(), m_window->getHeight());
    // Get window dimensions
    f32v2 windowDims(m_viewport.z, m_viewport.w);

    m_state->spaceCamera.setAspectRatio(windowDims.x / windowDims.y);
    m_state->localCamera.setAspectRatio(windowDims.x / windowDims.y);

    // Init Stages
    stages.opaqueVoxel.init(window, context);
    stages.cutoutVoxel.init(window, context);
    stages.chunkGrid.init(window, context);
    stages.transparentVoxel.init(window, context);
    stages.liquidVoxel.init(window, context);
    stages.devHud.init(window, context);
    stages.pda.init(window, context);
    stages.pauseMenu.init(window, context);
    stages.nightVision.init(window, context);
    stages.ssao.init(window, context);
    stages.bloom.init(window, context);
    stages.bloom.setParams();
    stages.exposureCalc.init(window, context);

    loadNightVision();

    // No post-process effects to begin with
    stages.bloom.setActive(true);
    stages.nightVision.setActive(false);
    stages.chunkGrid.setActive(true); // TODO(Ben): Temporary
    //stages.chunkGrid.setActive(false);
}

void GameplayRenderer::setRenderState(const MTRenderState* renderState) {
    m_renderState = renderState;
}

void GameplayRenderer::dispose(StaticLoadContext& context) {

    // Kill the builder
    if (m_loadThread) {
        delete m_loadThread;
        m_loadThread = nullptr;
    }

    stages.opaqueVoxel.dispose(context);
    stages.cutoutVoxel.dispose(context);
    stages.chunkGrid.dispose(context);
    stages.transparentVoxel.dispose(context);
    stages.liquidVoxel.dispose(context);
    stages.devHud.dispose(context);
    stages.pda.dispose(context);
    stages.pauseMenu.dispose(context);
    stages.nightVision.dispose(context);
    stages.ssao.dispose(context);
    stages.bloom.dispose(context);
    stages.exposureCalc.dispose(context);

    // dispose of persistent rendering resources
    m_hdrTarget.dispose();
    m_swapChain.dispose();
}

void GameplayRenderer::reloadShaders() {
    // TODO(Ben): More
    StaticLoadContext context;
    m_chunkRenderer.dispose();
    m_chunkRenderer.init();
    m_commonState->stages.spaceSystem.reloadShaders();
    
}

void GameplayRenderer::load(StaticLoadContext& context) {
    m_isLoaded = false;

    m_loadThread = new std::thread([&]() {
        
        vcore::GLRPC so[4];
        size_t i = 0;
        // Create the HDR target     
        so[i].set([&](Sender, void*) {
            Array<vg::GBufferAttachment> attachments;
            vg::GBufferAttachment att[2];
            // TODO(Ben): Don't think this is right.
            // Color
            att[0].format = vg::TextureInternalFormat::RGBA16F;
            att[0].pixelFormat = vg::TextureFormat::RGBA;
            att[0].pixelType = vg::TexturePixelType::UNSIGNED_BYTE;
            att[0].number = 1;
            // Normals
            att[1].format = vg::TextureInternalFormat::RGBA16F;
            att[1].pixelFormat = vg::TextureFormat::RGBA;
            att[1].pixelType = vg::TexturePixelType::UNSIGNED_BYTE;
            att[1].number = 2;
            m_hdrTarget.setSize(m_window->getWidth(), m_window->getHeight());
            m_hdrTarget.init(Array<vg::GBufferAttachment>(att, 2), vg::TextureInternalFormat::RGBA16F).initDepth();
                        
            if (soaOptions.get(OPT_MSAA).value.i > 0) {
                glEnable(GL_MULTISAMPLE);
            } else {
                glDisable(GL_MULTISAMPLE);
            }
        });
        m_glrpc.invoke(&so[i++], false);

        // Create the swap chain for post process effects (HDR-capable)
        so[i].set([&](Sender, void*) {
            m_swapChain.init(m_window->getWidth(), m_window->getHeight(), vg::TextureInternalFormat::RGBA16F);
        });
        m_glrpc.invoke(&so[i++], false);

        // Initialize the chunk renderer
        so[i].set([&](Sender, void*) {
            m_chunkRenderer.init();
        });
        m_glrpc.invoke(&so[i++], false);
        // Wait for the last command to complete
        so[i - 1].block();

        // Load all the stages
        stages.opaqueVoxel.load(context);
        stages.cutoutVoxel.load(context);
        stages.chunkGrid.load(context);
        stages.transparentVoxel.load(context);
        stages.liquidVoxel.load(context);
        stages.devHud.load(context);
        stages.pda.load(context);
        stages.pauseMenu.load(context);
        stages.nightVision.load(context);
        stages.ssao.load(context);
        stages.bloom.load(context);
        stages.exposureCalc.load(context);
        m_isLoaded = true;
    });
    m_loadThread->detach();
}

void GameplayRenderer::hook() {
    // Note: Common stages are hooked in MainMenuRenderer, no need to re-hook
    // Grab mesh manager handle
    m_meshManager = m_state->chunkMeshManager;
    stages.opaqueVoxel.hook(&m_chunkRenderer, &m_gameRenderParams);
    stages.cutoutVoxel.hook(&m_chunkRenderer, &m_gameRenderParams);
    stages.transparentVoxel.hook(&m_chunkRenderer, &m_gameRenderParams);
    stages.liquidVoxel.hook(&m_chunkRenderer, &m_gameRenderParams);
    stages.chunkGrid.hook(&m_gameRenderParams);
 
    //stages.devHud.hook();
    //stages.pda.hook();
    stages.pauseMenu.hook(&m_gameplayScreen->m_pauseMenu);
    stages.nightVision.hook(&m_commonState->quad);
    stages.ssao.hook(&m_commonState->quad, m_window->getWidth(), m_window->getHeight());
    stages.bloom.hook(&m_commonState->quad);
    stages.exposureCalc.hook(&m_commonState->quad, &m_hdrTarget, &m_viewport, 1024);
}

void GameplayRenderer::updateGL() {
    // TODO(Ben): Experiment with more requests
    m_glrpc.processRequests(1);
}


void GameplayRenderer::render() {
    const GameSystem* gameSystem = m_state->gameSystem;
    const SpaceSystem* spaceSystem = m_state->spaceSystem;

    updateCameras();
    // Set up the gameRenderParams
    const GameSystem* gs = m_state->gameSystem;

    // Get the physics component
    auto& phycmp = gs->physics.getFromEntity(m_state->playerEntity);
    VoxelPosition3D pos;
    if (phycmp.voxelPositionComponent) {
        pos = gs->voxelPosition.get(phycmp.voxelPositionComponent).gridPosition;
    }
    // TODO(Ben): Is this causing the camera slide descrepency? SHouldn't we use MTRenderState?
    m_gameRenderParams.calculateParams(m_state->spaceCamera.getPosition(), &m_state->localCamera,
                                       pos, 100, m_meshManager, &m_state->blocks, m_state->blockTextures, false);
    // Bind the FBO
    m_hdrTarget.useGeometry();
  
    glClear(GL_DEPTH_BUFFER_BIT);

    // worldCamera passes
    m_commonState->stages.skybox.render(&m_state->spaceCamera);

    if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    m_commonState->stages.spaceSystem.setShowAR(false);
    m_commonState->stages.spaceSystem.setRenderState(m_renderState);
    m_commonState->stages.spaceSystem.render(&m_state->spaceCamera);

    if (m_voxelsActive) {
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        stages.opaqueVoxel.render(&m_state->localCamera);
        // _physicsBlockRenderStage->draw();
        //  m_cutoutVoxelRenderStage->render();

        auto& voxcmp = gameSystem->voxelPosition.getFromEntity(m_state->playerEntity).parentVoxelComponent;
        stages.chunkGrid.setState(m_renderState);
        stages.chunkGrid.render(&m_state->localCamera);
        //  m_liquidVoxelRenderStage->render();
        //  m_transparentVoxelRenderStage->render();
    }
    if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Check for face transition animation state
    if (m_commonState->stages.spaceSystem.needsFaceTransitionAnimation) {
        m_commonState->stages.spaceSystem.needsFaceTransitionAnimation = false;
        m_increaseQuadAlpha = true;
        m_coloredQuadAlpha = 0.0f;
    }

    stages.exposureCalc.render();
    // Move exposure towards target
    static const f32 EXPOSURE_STEP = 0.005f;
    stepTowards(soaOptions.get(OPT_HDR_EXPOSURE).value.f, stages.exposureCalc.getExposure(), EXPOSURE_STEP);

    // Post processing
    m_swapChain.reset(0, m_hdrTarget.getGeometryID(), m_hdrTarget.getGeometryTexture(0), soaOptions.get(OPT_MSAA).value.i > 0, false);

    // TODO(Ben): This is broken
    if (stages.ssao.isActive()) {
        stages.ssao.set(m_hdrTarget.getDepthTexture(), m_hdrTarget.getGeometryTexture(0), m_swapChain.getCurrent().getID());
        stages.ssao.render();
        m_swapChain.swap();
        m_swapChain.use(0, false);
    }

    // last effect should not swap swapChain
    if (stages.nightVision.isActive()) {
        stages.nightVision.render();
        m_swapChain.swap();
        m_swapChain.use(0, false);
    }

    if (stages.bloom.isActive()) {
        stages.bloom.render();

        // Render star glow into same framebuffer for performance
        glBlendFunc(GL_ONE, GL_ONE);
        m_commonState->stages.spaceSystem.renderStarGlows(f32v3(1.0f));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_swapChain.swap();
        m_swapChain.use(0, false);
    } else {
        glBlendFunc(GL_ONE, GL_ONE);
        m_commonState->stages.spaceSystem.renderStarGlows(f32v3(1.0f));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // TODO: More Effects

    // Draw to backbuffer for the last effect
   // m_swapChain.bindPreviousTexture(0);
    m_swapChain.unuse(m_window->getWidth(), m_window->getHeight());
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    m_hdrTarget.bindDepthTexture(1);
    m_commonState->stages.hdr.render();

    // UI
    // stages.devHud.render();
    // stages.pda.render();
    stages.pauseMenu.render();

    // Cube face fade animation
    if (m_increaseQuadAlpha) {
        static const f32 FADE_INC = 0.07f;
        m_coloredQuadAlpha += FADE_INC;
        if (m_coloredQuadAlpha >= 3.5f) {
            m_coloredQuadAlpha = 3.5f;
            m_increaseQuadAlpha = false;
        }
      //  m_coloredQuadRenderer.draw(m_commonState->quad, f32v4(0.0, 0.0, 0.0, glm::min(m_coloredQuadAlpha, 1.0f)));
    } else if (m_coloredQuadAlpha > 0.0f) {
        static const float FADE_DEC = 0.01f;  
   //     m_coloredQuadRenderer.draw(m_commonState->quad, f32v4(0.0, 0.0, 0.0, glm::min(m_coloredQuadAlpha, 1.0f)));
        m_coloredQuadAlpha -= FADE_DEC;
    }

    if (m_shouldScreenshot) dumpScreenshot();

    // Check for errors, just in case
    checkGlError("GamePlayRenderer::render()");
}

void GameplayRenderer::cycleDevHud(int offset /* = 1 */) {
   // stages.devHud.cycleMode(offset);
}

void GameplayRenderer::toggleNightVision() {
    if (!stages.nightVision.isActive()) {
        stages.nightVision.setActive(true);
        m_nvIndex = 0;
        stages.nightVision.setParams(m_nvParams[m_nvIndex]);
    } else {
        m_nvIndex++;
        if (m_nvIndex >= m_nvParams.size()) {
            stages.nightVision.setActive(false);
        } else {
            stages.nightVision.setParams(m_nvParams[m_nvIndex]);
        }
    }
}

void GameplayRenderer::loadNightVision() {
    stages.nightVision.setActive(false);

    // TODO(Ben): This yaml code is outdated
    /*m_nvIndex = 0;
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
    for (size_t i = 0; i < arr.size(); i++) {
    m_nvParams.push_back(arr[i]);
    }
    context.reader.load();
    delete[] nvData;
    }
    if (m_nvParams.size() < 1) {
    m_nvParams.push_back(NightVisionRenderParams::createDefault());
    }*/
}

void GameplayRenderer::toggleChunkGrid() {
    stages.chunkGrid.toggleActive();
}

void GameplayRenderer::updateCameras() {
    const GameSystem* gs = m_state->gameSystem;
    const SpaceSystem* ss = m_state->spaceSystem;

    // Get the physics component
    auto& phycmp = gs->physics.getFromEntity(m_state->playerEntity);
    if (phycmp.voxelPositionComponent) {
        auto& vpcmp = gs->voxelPosition.get(phycmp.voxelPositionComponent);
        m_state->localCamera.setFocalLength(0.0f);
        m_state->localCamera.setClippingPlane(0.1f, 10000.0f);
        m_state->localCamera.setPosition(vpcmp.gridPosition.pos);
        m_state->localCamera.setOrientation(vpcmp.orientation);
        m_state->localCamera.update();

        m_voxelsActive = true;
    } else {
        m_voxelsActive = false;
    }
    // Player is relative to a planet, so add position if needed
    auto& spcmp = gs->spacePosition.get(phycmp.spacePositionComponent);
    if (spcmp.parentGravityID) {
        auto& it = m_renderState->spaceBodyPositions.find(spcmp.parentEntity);
        if (it != m_renderState->spaceBodyPositions.end()) {
            m_state->spaceCamera.setPosition(m_renderState->spaceCameraPos + it->second);
        } else {
            auto& gcmp = ss->m_sphericalGravityCT.get(spcmp.parentGravityID);
            auto& npcmp = ss->m_namePositionCT.get(gcmp.namePositionComponent);
            m_state->spaceCamera.setPosition(m_renderState->spaceCameraPos + npcmp.position);
        }
    } else {
        m_state->spaceCamera.setPosition(m_renderState->spaceCameraPos);
    }
    m_state->spaceCamera.setIsDynamic(false);
    m_state->spaceCamera.setFocalLength(0.0f);
    m_state->spaceCamera.setClippingPlane(0.1f, 100000000000.0f);
   
    m_state->spaceCamera.setOrientation(m_renderState->spaceCameraOrientation);
    m_state->spaceCamera.update();
}

void GameplayRenderer::dumpScreenshot() {
    // Make screenshots directory
    vio::IOManager().makeDirectory("Screenshots");
    // Take screenshot
    dumpFramebufferImage("Screenshots/", m_viewport);
    m_shouldScreenshot = false;
}

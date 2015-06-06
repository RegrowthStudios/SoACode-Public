#include "stdafx.h"
#include "GameplayRenderer.h"

#include <Vorb/graphics/GLStates.h>
#include <Vorb/ui/GameWindow.h>
#include <Vorb/AssetLoader.h>

#include "ChunkMemoryManager.h"
#include "ChunkMeshManager.h"
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

void GameplayRenderer::init(vui::GameWindow* window, LoadContext& context, GameplayScreen* gameplayScreen) {
    m_window = window;
    m_gameplayScreen = gameplayScreen;

    // TODO(Ben): Dis is bad mkay
    m_viewport = f32v4(0, 0, m_window->getWidth(), m_window->getHeight());
    // Get window dimensions
    f32v2 windowDims(m_viewport.z, m_viewport.w);

    m_spaceCamera.setAspectRatio(windowDims.x / windowDims.y);
    m_localCamera.setAspectRatio(windowDims.x / windowDims.y);

    // Init Stages
    stages.skybox.init(window, context);
    stages.spaceSystem.init(window, context);
    stages.opaqueVoxel.init(window, context);
    stages.cutoutVoxel.init(window, context);
    stages.chunkGrid.init(window, context);
    stages.transparentVoxel.init(window, context);
    stages.liquidVoxel.init(window, context);
    stages.devHud.init(window, context);
    stages.pda.init(window, context);
    stages.pauseMenu.init(window, context);
    stages.nightVision.init(window, context);
    stages.hdr.init(window, context);

    loadNightVision();

    // No post-process effects to begin with
    stages.nightVision.setActive(false);
    stages.chunkGrid.setActive(false);
}

void GameplayRenderer::setRenderState(const MTRenderState* renderState) {
    m_renderState = renderState;
}

void GameplayRenderer::dispose(LoadContext& context) {

    // Kill the builder
    if (m_loadThread) {
        delete m_loadThread;
        m_loadThread = nullptr;
    }

    stages.skybox.dispose(context);
    stages.spaceSystem.dispose(context);
    stages.opaqueVoxel.dispose(context);
    stages.cutoutVoxel.dispose(context);
    stages.chunkGrid.dispose(context);
    stages.transparentVoxel.dispose(context);
    stages.liquidVoxel.dispose(context);
    stages.devHud.dispose(context);
    stages.pda.dispose(context);
    stages.pauseMenu.dispose(context);
    stages.nightVision.dispose(context);
    stages.hdr.dispose(context);

    // dispose of persistent rendering resources
    m_hdrTarget.dispose();
    m_swapChain.dispose();
    m_quad.dispose();
}

void GameplayRenderer::load(LoadContext& context) {
    m_isLoaded = false;

    m_loadThread = new std::thread([&]() {
        vcore::GLRPC so[4];
        size_t i = 0;

        // Create the HDR target     
        so[i].set([&](Sender, void*) {
            m_hdrTarget.setSize(m_window->getWidth(), m_window->getHeight());
            m_hdrTarget.init(vg::TextureInternalFormat::RGBA16F, (ui32)soaOptions.get(OPT_MSAA).value.i).initDepth();
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

        // Create full-screen quad
        so[i].set([&](Sender, void*) {
            m_quad.init();
        });
        m_glrpc.invoke(&so[i++], false);

        // Wait for the last command to complete
        so[i - 1].block();

        // Load all the stages
        stages.skybox.load(context, m_glrpc);
        stages.spaceSystem.load(context, m_glrpc);
        stages.opaqueVoxel.load(context, m_glrpc);
        stages.cutoutVoxel.load(context, m_glrpc);
        stages.chunkGrid.load(context, m_glrpc);
        stages.transparentVoxel.load(context, m_glrpc);
        stages.liquidVoxel.load(context, m_glrpc);
        stages.devHud.load(context, m_glrpc);
        stages.pda.load(context, m_glrpc);
        stages.pauseMenu.load(context, m_glrpc);
        stages.nightVision.load(context, m_glrpc);
        stages.hdr.load(context, m_glrpc);

        m_isLoaded = true;
    });
    m_loadThread->detach();
}

void GameplayRenderer::hook(SoaState* state) {
    m_state = state;
    // Grab mesh manager handle
    m_meshManager = state->chunkMeshManager.get();
    stages.skybox.hook(state);
    stages.spaceSystem.hook(state, &state->spaceCamera);
    stages.opaqueVoxel.hook(&m_gameRenderParams);
    stages.cutoutVoxel.hook(&m_gameRenderParams);
    stages.chunkGrid.hook(&m_gameRenderParams);
    stages.transparentVoxel.hook(&m_gameRenderParams);
    stages.liquidVoxel.hook(&m_gameRenderParams);
    //stages.devHud.hook();
    //stages.pda.hook();
    stages.pauseMenu.hook(&m_gameplayScreen->m_pauseMenu);
    stages.nightVision.hook(&m_quad);
    stages.hdr.hook(&m_quad);
}

void GameplayRenderer::updateGL() {
    // TODO(Ben): Experiment with more requests
    m_glrpc.processRequests(1);
}


void GameplayRenderer::render() {
    const GameSystem* gameSystem = m_state->gameSystem.get();
    const SpaceSystem* spaceSystem = m_state->spaceSystem.get();

    updateCameras();
    // Set up the gameRenderParams
    const GameSystem* gs = m_state->gameSystem.get();

    // Get the physics component
    auto& phycmp = gs->physics.getFromEntity(m_state->playerEntity);
    VoxelPosition3D pos;
    if (phycmp.voxelPositionComponent) {
        pos = gs->voxelPosition.get(phycmp.voxelPositionComponent).gridPosition;
    }
    m_gameRenderParams.calculateParams(m_spaceCamera.getPosition(), &m_localCamera,
                                       pos, 100, m_meshManager, false);
    // Bind the FBO
    m_hdrTarget.use();
  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // worldCamera passes
    stages.skybox.render(&m_spaceCamera);

    if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    stages.spaceSystem.setShowAR(false);
    stages.spaceSystem.setRenderState(m_renderState);
    stages.spaceSystem.render(&m_spaceCamera);

    if (m_voxelsActive) {
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        stages.opaqueVoxel.render(&m_localCamera);
        // _physicsBlockRenderStage->draw();
        //  m_cutoutVoxelRenderStage->render();

        auto& voxcmp = gameSystem->voxelPosition.getFromEntity(m_state->playerEntity).parentVoxelComponent;
        stages.chunkGrid.setChunks(spaceSystem->m_sphericalVoxelCT.get(voxcmp).chunkMemoryManager);
        stages.chunkGrid.render(&m_localCamera);
        //  m_liquidVoxelRenderStage->render();
        //  m_transparentVoxelRenderStage->render();
    }
    if (m_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Check for face transition animation state
    if (stages.spaceSystem.needsFaceTransitionAnimation) {
        stages.spaceSystem.needsFaceTransitionAnimation = false;
        m_increaseQuadAlpha = true;
        m_coloredQuadAlpha = 0.0f;
    }

    // Render last
    glBlendFunc(GL_ONE, GL_ONE);
    stages.spaceSystem.renderStarGlows(f32v3(1.0f));
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Post processing
    m_swapChain.reset(0, m_hdrTarget.getID(), m_hdrTarget.getTextureID(), soaOptions.get(OPT_MSAA).value.i > 0, false);

    // TODO: More Effects
    if (stages.nightVision.isActive()) {
        stages.nightVision.render();
        m_swapChain.swap();
        m_swapChain.use(0, false);
    }

    // Draw to backbuffer for the last effect
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(m_hdrTarget.getTextureTarget(), m_hdrTarget.getTextureDepthID());
    stages.hdr.render();

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
    const GameSystem* gs = m_state->gameSystem.get();
    const SpaceSystem* ss = m_state->spaceSystem.get();

    // Get the physics component
    auto& phycmp = gs->physics.getFromEntity(m_state->playerEntity);
    if (phycmp.voxelPositionComponent) {
        auto& vpcmp = gs->voxelPosition.get(phycmp.voxelPositionComponent);
        m_localCamera.setClippingPlane(0.1f, 10000.0f);
        m_localCamera.setPosition(vpcmp.gridPosition.pos);
        m_localCamera.setOrientation(vpcmp.orientation);
        m_localCamera.update();

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

    m_spaceCamera.setClippingPlane(0.1f, 100000000000.0f);
   
    m_spaceCamera.setOrientation(m_renderState->spaceCameraOrientation);
    m_spaceCamera.update();
}

void GameplayRenderer::dumpScreenshot() {
    // Make screenshots directory
    vio::IOManager().makeDirectory("Screenshots");
    // Take screenshot
    dumpFramebufferImage("Screenshots/", m_viewport);
    m_shouldScreenshot = false;
}

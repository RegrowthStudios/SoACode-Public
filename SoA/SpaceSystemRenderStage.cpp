#include "stdafx.h"
#include "SpaceSystem.h"
#include "SpaceSystemRenderStage.h"

#include <Vorb/os.h>
#include <Vorb/graphics/DepthState.h>
#include <Vorb/graphics/RasterizerState.h>

#include "App.h"
#include "Camera.h"
#include "DebugRenderer.h"
#include "Errors.h"
#include "MTRenderState.h"
#include "MainMenuSystemViewer.h"
#include "RenderUtils.h"
#include "SoAState.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatch.h"
#include "TerrainPatchMeshManager.h"
#include "soaUtils.h"

#include <Vorb/Timing.h>

#define ATMO_LOAD_DIST 50000.0f

/* ===================== LoadContext =================== */
const ui32 LENS_WORK = 2;
const ui32 STAR_WORK = 2;
const ui32 AR_WORK = 2;
const ui32 SPHER_WORK = 2;
const ui32 GAS_WORK = 2;
const ui32 CLOUD_WORK = 2;
const ui32 ATMO_WORK = 2;
const ui32 RING_WORK = 2;
const ui32 FAR_WORK = 2;
// Make sure to sum all work here
const ui32 TOTAL_WORK = LENS_WORK + STAR_WORK + AR_WORK +
                        SPHER_WORK + GAS_WORK + CLOUD_WORK +
                        ATMO_WORK + RING_WORK + FAR_WORK;
// Make sure NUM_TASKS matches the number of tasks listed
const ui32 NUM_TASKS = 9;
/* ====================================================== */

const f64q FACE_ORIENTATIONS[6] = {
    f64q(f64v3(0.0, 0.0, 0.0)), // TOP
    f64q(f64v3(0.0, 0.0, -M_PI / 2.0)), // LEFT
    f64q(f64v3(0.0, 0.0, M_PI / 2.0)), // RIGHT
    f64q(f64v3(M_PI / 2.0, 0.0, 0.0)), // FRONT
    f64q(f64v3(-M_PI / 2.0, 0.0, 0.0)), // BACK
    f64q(f64v3(M_PI, 0.0, 0.0))  // BOTTOM
};

void SpaceSystemRenderStage::init(vui::GameWindow* window, StaticLoadContext& context) {
    IRenderStage::init(window, context);
    context.addAnticipatedWork(TOTAL_WORK, NUM_TASKS);
}

void SpaceSystemRenderStage::hook(SoaState* state, const Camera* spaceCamera, const Camera* farTerrainCamera /*= nullptr*/) {
    m_viewport = m_window->getViewportDims();
    m_spaceSystem = state->spaceSystem;
    m_mainMenuSystemViewer = state->clientState.systemViewer;
    m_lensFlareRenderer.init(&state->clientState.texturePathResolver);
    m_starRenderer.init(&state->clientState.texturePathResolver);
    m_systemARRenderer.init(&state->clientState.texturePathResolver);
    m_spaceCamera = spaceCamera;
    m_farTerrainCamera = farTerrainCamera;
}

void SpaceSystemRenderStage::load(StaticLoadContext& context) {
    context.addTask([&](Sender, void*) {
        m_lensFlareRenderer.initGL();
        context.addWorkCompleted(LENS_WORK);
    }, false);
    context.addTask([&](Sender, void*) {
        m_starRenderer.initGL();
        context.addWorkCompleted(STAR_WORK);
    }, false);
    context.addTask([&](Sender, void*) {
        m_systemARRenderer.initGL();
        context.addWorkCompleted(AR_WORK);
    }, false);
    context.addTask([&](Sender, void*) {
        m_sphericalTerrainComponentRenderer.initGL();
        context.addWorkCompleted(SPHER_WORK);
    }, false);
    context.addTask([&](Sender, void*) {
        m_gasGiantComponentRenderer.initGL();
        context.addWorkCompleted(GAS_WORK);
    }, false);
    context.addTask([&](Sender, void*) {
        m_cloudsComponentRenderer.initGL();
        context.addWorkCompleted(CLOUD_WORK);
    }, false);
    context.addTask([&](Sender, void*) {
        m_atmosphereComponentRenderer.initGL();
        context.addWorkCompleted(ATMO_WORK);
    }, false);
    context.addTask([&](Sender, void*) {
        m_ringsRenderer.initGL();
        context.addWorkCompleted(RING_WORK);
    }, false);
    context.addTask([&](Sender, void*) {
        m_farTerrainComponentRenderer.initGL();
        context.addWorkCompleted(LENS_WORK);
    }, false);
}

void SpaceSystemRenderStage::dispose(StaticLoadContext& context) {
    m_lensFlareRenderer.dispose();
    m_starRenderer.dispose();
    m_systemARRenderer.dispose();
    m_sphericalTerrainComponentRenderer.dispose();
    m_gasGiantComponentRenderer.dispose();
    m_cloudsComponentRenderer.dispose();
    m_atmosphereComponentRenderer.dispose();
    m_ringsRenderer.dispose();
    m_farTerrainComponentRenderer.dispose();
}

void SpaceSystemRenderStage::setRenderState(const MTRenderState* renderState) {
    m_renderState = renderState;
}

void SpaceSystemRenderStage::render(const Camera* camera) {
    drawBodies();
    if (m_showAR) m_systemARRenderer.draw(m_spaceSystem, m_spaceCamera,
                                          m_mainMenuSystemViewer,
                                          m_viewport);
}

void SpaceSystemRenderStage::reloadShaders() {
    StaticLoadContext tmp;
    dispose(tmp);

    m_lensFlareRenderer.initGL();
    m_starRenderer.initGL();
    m_systemARRenderer.initGL();
    m_sphericalTerrainComponentRenderer.initGL();
    m_gasGiantComponentRenderer.initGL();
    m_cloudsComponentRenderer.initGL();
    m_atmosphereComponentRenderer.initGL();
    m_ringsRenderer.initGL();
    m_farTerrainComponentRenderer.initGL();
}

void SpaceSystemRenderStage::renderStarGlows(const f32v3& colorMult) {
    for (auto& it : m_starGlowsToRender) {
        m_starRenderer.drawGlow(it.first, m_spaceCamera->getViewProjectionMatrix(), it.second,
                                m_spaceCamera->getAspectRatio(), m_spaceCamera->getDirection(),
                                m_spaceCamera->getRight(), colorMult);
        // TODO(Ben): Don't do this twice?
        f32v3 starColor = m_starRenderer.calculateStarColor(it.first);
        f32 intensity = (f32)glm::min(m_starRenderer.calculateGlowSize(it.first, it.second), 1.0) * it.first.visibility;
        m_lensFlareRenderer.render(m_spaceCamera->getViewProjectionMatrix(), it.second,
                                   starColor * colorMult,
                                   m_spaceCamera->getAspectRatio(), 0.1f, intensity);
    }
}

void SpaceSystemRenderStage::drawBodies() {

    glEnable(GL_DEPTH_CLAMP);
    bool needsDepthClear = false;
    // TODO(Ben): Optimize out getFromEntity
    f64v3 lightPos;
    // For caching light for far terrain
    std::map<vecs::EntityID, std::pair<f64v3, SpaceLightComponent*> > lightCache;
    const f64v3* pos;
    f32 zCoef = computeZCoef(m_spaceCamera->getFarClip());
    // Render spherical terrain
    for (auto& it : m_spaceSystem->sphericalTerrain) {
        auto& cmp = it.second;
        auto& npCmp = m_spaceSystem->namePosition.get(cmp.namePositionComponent);
        // Cant render if it hasn't generated yet
        if (!cmp.planetGenData) continue;
        // Indicate the need for face transition animation
        if (cmp.needsFaceTransitionAnimation) {
            cmp.needsFaceTransitionAnimation = false;
            needsFaceTransitionAnimation = true;
        }

        // If we are using MTRenderState, get position from it
        pos = getBodyPosition(npCmp, it.first);

        // Need to clear depth on fade transitions
        if (cmp.farTerrainComponent && (cmp.alpha > 0.0f && cmp.alpha < TERRAIN_FADE_LENGTH)) {
            needsDepthClear = true;
        }

        SpaceLightComponent* lightCmp = getBrightestLight(npCmp, lightPos);
        lightCache[it.first] = std::make_pair(lightPos, lightCmp);

        f32v3 lightDir(glm::normalize(lightPos - *pos));
    
        m_sphericalTerrainComponentRenderer.draw(cmp, m_spaceCamera,
                                                 lightDir,
                                                 *pos,
                                                 zCoef,
                                                 lightCmp,
                                                 &m_spaceSystem->axisRotation.get(cmp.axisRotationComponent),
                                                 &m_spaceSystem->atmosphere.getFromEntity(it.first));
    }
    // Render gas giants
    for (auto& it : m_spaceSystem->gasGiant) {
        auto& ggCmp = it.second;
        auto& npCmp = m_spaceSystem->namePosition.get(ggCmp.namePositionComponent);

        pos = getBodyPosition(npCmp, it.first);

        f32v3 relCamPos(m_spaceCamera->getPosition() - *pos);

        SpaceLightComponent* lightCmp = getBrightestLight(npCmp, lightPos);
        lightCache[it.first] = std::make_pair(lightPos, lightCmp);

        f32v3 lightDir(glm::normalize(lightPos - *pos));

        m_gasGiantComponentRenderer.draw(ggCmp, it.first,
                                         m_spaceCamera->getViewProjectionMatrix(),
                                         m_spaceSystem->axisRotation.getFromEntity(it.first).currentOrientation,
                                         relCamPos, lightDir,
                                         zCoef, lightCmp,
                                         &m_spaceSystem->atmosphere.getFromEntity(it.first));
    }

    // Render clouds
  /*  glDisable(GL_CULL_FACE);
    for (auto& it : m_spaceSystem->m_cloudsCT) {
        auto& cCmp = it.second;
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(cCmp.namePositionComponent);

        pos = getBodyPosition(npCmp, it.first);
        f32v3 relCamPos(m_spaceCamera->getPosition() - *pos);
        auto& l = lightCache[it.first];
        f32v3 lightDir(glm::normalize(l.first - *pos));
        
        m_cloudsComponentRenderer.draw(cCmp, m_spaceCamera->getViewProjectionMatrix(), relCamPos, lightDir,
                                       zCoef, l.second,
                                       m_spaceSystem->m_axisRotationCT.getFromEntity(it.first), 
                                       m_spaceSystem->m_atmosphereCT.getFromEntity(it.first));
    }
    glEnable(GL_CULL_FACE);*/

    // Render atmospheres
    for (auto& it : m_spaceSystem->atmosphere) {
        auto& atCmp = it.second;
        auto& npCmp = m_spaceSystem->namePosition.get(atCmp.namePositionComponent);

        pos = getBodyPosition(npCmp, it.first);

        f32v3 relCamPos(m_spaceCamera->getPosition() - *pos);

        if (glm::length(relCamPos) < atCmp.radius * 11.0f) {
            auto& l = lightCache[it.first];

            f32v3 lightDir(glm::normalize(l.first - *pos));

            m_atmosphereComponentRenderer.draw(atCmp, m_spaceCamera->getViewProjectionMatrix(), relCamPos, lightDir,
                                               zCoef, l.second);
        }
    }

    // Render planet rings
    glDepthMask(GL_FALSE);
    for (auto& it : m_spaceSystem->planetRings) {
        auto& prCmp = it.second;
        auto& npCmp = m_spaceSystem->namePosition.get(prCmp.namePositionComponent);

        // TODO(Ben): Don't use getFromEntity
        auto& sgCmp = m_spaceSystem->sphericalGravity.getFromEntity(it.first);

        pos = getBodyPosition(npCmp, it.first);

        f32v3 relCamPos(m_spaceCamera->getPosition() - *pos);

        auto& l = lightCache[it.first];

        f32v3 lightDir(glm::normalize(l.first - *pos));

        // TODO(Ben): Worry about f64 to f32 precision loss
        m_ringsRenderer.draw(prCmp, it.first, m_spaceCamera->getViewProjectionMatrix(), relCamPos,
                             f32v3(l.first - m_spaceCamera->getPosition()), (f32)sgCmp.radius,
                             zCoef, l.second);
    }
    glDepthMask(GL_TRUE);

    // Render far terrain
    if (m_farTerrainCamera) {

        if (needsDepthClear) {
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        for (auto& it : m_spaceSystem->farTerrain) {
            auto& cmp = it.second;
            auto& npCmp = m_spaceSystem->namePosition.getFromEntity(it.first);

            if (!cmp.meshManager) continue;

            pos = getBodyPosition(npCmp, it.first);

            auto& l = lightCache[it.first];
            f64v3 lightDir = glm::normalize(l.first - *pos);

            m_farTerrainComponentRenderer.draw(cmp, m_farTerrainCamera,
                                               lightDir,
                                               zCoef,
                                               l.second,
                                               &m_spaceSystem->axisRotation.getFromEntity(it.first),
                                               &m_spaceSystem->atmosphere.getFromEntity(it.first));
        }
    }

    // Render stars
    m_starGlowsToRender.clear();
    for (auto& it : m_spaceSystem->star) {
        auto& sCmp = it.second;
        auto& npCmp = m_spaceSystem->namePosition.get(sCmp.namePositionComponent);

        pos = getBodyPosition(npCmp, it.first);

        f64v3 relCamPos = m_spaceCamera->getPosition() - *pos;
        f32v3 fRelCamPos(relCamPos);

        // Render the star
        m_starRenderer.updateOcclusionQuery(sCmp, zCoef, m_spaceCamera->getViewProjectionMatrix(), relCamPos);
        m_starRenderer.drawStar(sCmp, m_spaceCamera->getViewProjectionMatrix(), f64q(), fRelCamPos, zCoef);
        m_starRenderer.drawCorona(sCmp, m_spaceCamera->getViewProjectionMatrix(), m_spaceCamera->getViewMatrix(), fRelCamPos, zCoef);
        
        m_starGlowsToRender.emplace_back(sCmp, relCamPos);
    }

    glDisable(GL_DEPTH_CLAMP);
    vg::DepthState::FULL.set();
}

SpaceLightComponent* SpaceSystemRenderStage::getBrightestLight(NamePositionComponent& npCmp, OUT f64v3& pos) {
    SpaceLightComponent* rv = nullptr;
    f64 closestDist = 999999999999999999999999999999999999999999999.0;
    for (auto& it : m_spaceSystem->spaceLight) {
        auto& lightNpCmp = m_spaceSystem->namePosition.get(it.second.npID);    
        f64 dist = selfDot(lightNpCmp.position - npCmp.position);
        if (dist < closestDist) {
            closestDist = dist;
            rv = &it.second;
            pos = lightNpCmp.position;
        }
    }
    return rv;
}

const f64v3* SpaceSystemRenderStage::getBodyPosition(NamePositionComponent& npCmp, vecs::EntityID eid) {
    const f64v3* pos;
    // If we are using MTRenderState, get position from it
    if (m_renderState) {
        auto sit = m_renderState->spaceBodyPositions.find(eid);
        if (sit != m_renderState->spaceBodyPositions.end()) {
            pos = &sit->second;
        } else {
            pos = &npCmp.position;
        }
    } else {
        pos = &npCmp.position;
    }
    return pos;
}

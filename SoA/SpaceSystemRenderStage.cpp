#include "stdafx.h"
#include "SpaceSystem.h"
#include "SpaceSystemRenderStage.h"

#include <Vorb/graphics/DepthState.h>
#include <Vorb/graphics/RasterizerState.h>

#include "App.h"
#include "Camera.h"
#include "DebugRenderer.h"
#include "Errors.h"
#include "MTRenderState.h"
#include "MainMenuSystemViewer.h"
#include "RenderUtils.h"
#include "SoaState.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatch.h"
#include "TerrainPatchMeshManager.h"
#include "soaUtils.h"

#include <Vorb/Timing.h>

#define ATMO_LOAD_DIST 50000.0f

const f64q FACE_ORIENTATIONS[6] = {
    f64q(f64v3(0.0, 0.0, 0.0)), // TOP
    f64q(f64v3(0.0, 0.0, -M_PI / 2.0)), // LEFT
    f64q(f64v3(0.0, 0.0, M_PI / 2.0)), // RIGHT
    f64q(f64v3(M_PI / 2.0, 0.0, 0.0)), // FRONT
    f64q(f64v3(-M_PI / 2.0, 0.0, 0.0)), // BACK
    f64q(f64v3(M_PI, 0.0, 0.0))  // BOTTOM
};

SpaceSystemRenderStage::SpaceSystemRenderStage(const SoaState* soaState,
                                               ui32v2 viewport,
                                               SpaceSystem* spaceSystem,
                                               GameSystem* gameSystem,
                                               const MainMenuSystemViewer* systemViewer,
                                               const Camera* spaceCamera,
                                               const Camera* farTerrainCamera) :
    m_viewport(viewport),
    m_spaceSystem(spaceSystem),
    m_gameSystem(gameSystem),
    m_mainMenuSystemViewer(systemViewer),
    m_spaceCamera(spaceCamera),
    m_farTerrainCamera(farTerrainCamera),
    m_lensFlareRenderer(&soaState->texturePathResolver),
    m_starRenderer(&soaState->texturePathResolver),
    m_systemARRenderer(&soaState->texturePathResolver) {
    // Empty
}

SpaceSystemRenderStage::~SpaceSystemRenderStage() {
    // Empty
}

void SpaceSystemRenderStage::setRenderState(const MTRenderState* renderState) {
    m_renderState = renderState;
}

void SpaceSystemRenderStage::render() {
    drawBodies();
    if (m_showAR) m_systemARRenderer.draw(m_spaceSystem, m_spaceCamera,
                                          m_mainMenuSystemViewer,
                                          m_viewport);
}

void SpaceSystemRenderStage::renderStarGlows(const f32v3& colorMult) {
    for (auto& it : m_starGlowsToRender) {
        m_starRenderer.drawGlow(it.first, m_spaceCamera->getViewProjectionMatrix(), it.second,
                                m_spaceCamera->getAspectRatio(), m_spaceCamera->getDirection(),
                                m_spaceCamera->getRight(), colorMult);
        // TODO(Ben): Don't do this twice?
        f32v3 starColor = m_starRenderer.calculateStarColor(it.first);
        f32 intensity = glm::min(m_starRenderer.calculateGlowSize(it.first, it.second), 1.0) * it.first.visibility;
        m_lensFlareRenderer.render(m_spaceCamera->getViewProjectionMatrix(), it.second,
                                   starColor * colorMult,
                                   m_spaceCamera->getAspectRatio(), 0.1f, intensity);
    }
}

void SpaceSystemRenderStage::reloadShader() {
    m_sphericalTerrainComponentRenderer.disposeShaders();
    m_farTerrainComponentRenderer.disposeShaders();
    m_atmosphereComponentRenderer.disposeShader();
    m_gasGiantComponentRenderer.disposeShader();
    m_starRenderer.disposeShaders();
    m_ringsRenderer.disposeShader();
    m_lensFlareRenderer.dispose();
    m_cloudsComponentRenderer.disposeShader();
}

f32 SpaceSystemRenderStage::getDynamicNearPlane(float verticalFOV, float aspectRatio) {
    if (m_closestPatchDistance2 == DOUBLE_SENTINEL) return 0.0f;
    f32 radFOV = verticalFOV * (f32)DEG_TO_RAD;

    // This is just algebra using the formulas at
    // http://gamedev.stackexchange.com/a/19801
    // We have to solve for Z
    f32 planeDist = (f32)sqrt(m_closestPatchDistance2);
    f32 g = tan(radFOV / 2.0f);
    f32 g2 = g * g;
    f32 a = g * planeDist / sqrt(g2 * aspectRatio * aspectRatio + g2 + 1);
    return a / g; // Return Z
}

void SpaceSystemRenderStage::drawBodies() {

    m_closestPatchDistance2 = DOUBLE_SENTINEL;

    bool needsDepthClear = false;
    // TODO(Ben): Optimize out getFromEntity
    f64v3 lightPos;
    // For caching light for far terrain
    std::map<vecs::EntityID, std::pair<f64v3, SpaceLightComponent*> > lightCache;
    const f64v3* pos;
    // Render spherical terrain
    for (auto& it : m_spaceSystem->m_sphericalTerrainCT) {
        auto& cmp = it.second;
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(cmp.namePositionComponent);
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
                                                 lightCmp,
                                                 &m_spaceSystem->m_axisRotationCT.get(cmp.axisRotationComponent),
                                                 &m_spaceSystem->m_atmosphereCT.getFromEntity(it.first));
        
        f64 dist = cmp.meshManager->getClosestSphericalDistance2();
        if (dist < m_closestPatchDistance2) m_closestPatchDistance2 = dist;
    }
    // Render gas giants
    for (auto& it : m_spaceSystem->m_gasGiantCT) {
        auto& ggCmp = it.second;
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(ggCmp.namePositionComponent);

        pos = getBodyPosition(npCmp, it.first);

        f32v3 relCamPos(m_spaceCamera->getPosition() - *pos);

        SpaceLightComponent* lightCmp = getBrightestLight(npCmp, lightPos);
        lightCache[it.first] = std::make_pair(lightPos, lightCmp);

        f32v3 lightDir(glm::normalize(lightPos - *pos));

        m_gasGiantComponentRenderer.draw(ggCmp, m_spaceCamera->getViewProjectionMatrix(),
                                         m_spaceSystem->m_axisRotationCT.getFromEntity(it.first).currentOrientation,
                                         relCamPos, lightDir, lightCmp,
                                         &m_spaceSystem->m_atmosphereCT.getFromEntity(it.first));
    }

    // Render clouds
    for (auto& it : m_spaceSystem->m_cloudsCT) {
        auto& cCmp = it.second;
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(cCmp.namePositionComponent);

        pos = getBodyPosition(npCmp, it.first);
        f32v3 relCamPos(m_spaceCamera->getPosition() - *pos);
        auto& l = lightCache[it.first];
        f32v3 lightDir(glm::normalize(l.first - *pos));
        
        m_cloudsComponentRenderer.draw(cCmp, m_spaceCamera->getViewProjectionMatrix(), relCamPos, lightDir, l.second,
                                       m_spaceSystem->m_axisRotationCT.getFromEntity(it.first), 
                                       m_spaceSystem->m_atmosphereCT.getFromEntity(it.first));
    }

    // Render atmospheres
    for (auto& it : m_spaceSystem->m_atmosphereCT) {
        auto& atCmp = it.second;
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(atCmp.namePositionComponent);

        pos = getBodyPosition(npCmp, it.first);

        f32v3 relCamPos(m_spaceCamera->getPosition() - *pos);

        if (glm::length(relCamPos) < atCmp.radius * 11.0f) {
            auto& l = lightCache[it.first];

            f32v3 lightDir(glm::normalize(l.first - *pos));

            m_atmosphereComponentRenderer.draw(atCmp, m_spaceCamera->getViewProjectionMatrix(), relCamPos, lightDir, l.second);
        }
    }

    // Render planet rings
    glDepthMask(GL_FALSE);
    for (auto& it : m_spaceSystem->m_planetRingCT) {
        auto& prCmp = it.second;
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(prCmp.namePositionComponent);

        // TODO(Ben): Don't use getFromEntity
        auto& sgCmp = m_spaceSystem->m_sphericalGravityCT.getFromEntity(it.first);

        pos = getBodyPosition(npCmp, it.first);

        f32v3 relCamPos(m_spaceCamera->getPosition() - *pos);

        auto& l = lightCache[it.first];

        f32v3 lightDir(glm::normalize(l.first - *pos));

        // TODO(Ben): Worry about f64 to f32 precision loss
        m_ringsRenderer.draw(prCmp, m_spaceCamera->getViewProjectionMatrix(), relCamPos, f32v3(l.first - m_spaceCamera->getPosition()), sgCmp.radius, l.second);
    }
    glDepthMask(GL_TRUE);

    // Render far terrain
    if (m_farTerrainCamera) {

        if (needsDepthClear) {
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        for (auto& it : m_spaceSystem->m_farTerrainCT) {
            auto& cmp = it.second;
            auto& npCmp = m_spaceSystem->m_namePositionCT.getFromEntity(it.first);

            if (!cmp.meshManager) continue;

            pos = getBodyPosition(npCmp, it.first);

            auto& l = lightCache[it.first];
            f64v3 lightDir = glm::normalize(l.first - *pos);

            m_farTerrainComponentRenderer.draw(cmp, m_farTerrainCamera,
                                               lightDir,
                                               l.second,
                                               &m_spaceSystem->m_axisRotationCT.getFromEntity(it.first),
                                               &m_spaceSystem->m_atmosphereCT.getFromEntity(it.first));
            
            f64 dist = cmp.meshManager->getClosestFarDistance2();
            if (dist < m_closestPatchDistance2) m_closestPatchDistance2 = dist;
        }
    }

    // Render stars
    m_starGlowsToRender.clear();
    for (auto& it : m_spaceSystem->m_starCT) {
        auto& sCmp = it.second;
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(sCmp.namePositionComponent);

        pos = getBodyPosition(npCmp, it.first);

        f64v3 relCamPos = m_spaceCamera->getPosition() - *pos;
        f32v3 fRelCamPos(relCamPos);

        // Render the star
        m_starRenderer.updateOcclusionQuery(sCmp, m_spaceCamera->getViewProjectionMatrix(), relCamPos);
        m_starRenderer.drawStar(sCmp, m_spaceCamera->getViewProjectionMatrix(), f64q(), fRelCamPos);
        m_starRenderer.drawCorona(sCmp, m_spaceCamera->getViewProjectionMatrix(), m_spaceCamera->getViewMatrix(), fRelCamPos);
        
        m_starGlowsToRender.emplace_back(sCmp, relCamPos);
    }

    vg::DepthState::FULL.set();
}

SpaceLightComponent* SpaceSystemRenderStage::getBrightestLight(NamePositionComponent& npCmp, OUT f64v3& pos) {
    SpaceLightComponent* rv = nullptr;
    f64 closestDist = 999999999999999999999999999999999999999999999.0;
    for (auto& it : m_spaceSystem->m_spaceLightCT) {
        auto& lightNpCmp = m_spaceSystem->m_namePositionCT.get(it.second.npID);    
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
        auto& sit = m_renderState->spaceBodyPositions.find(eid);
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

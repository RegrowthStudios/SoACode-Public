#include "stdafx.h"
#include "SpaceSystem.h"
#include "SpaceSystemRenderStage.h"

#include <Vorb/graphics/DepthState.h>
#include <Vorb/graphics/RasterizerState.h>

#include "App.h"
#include "Camera.h"
#include "DebugRenderer.h"
#include "Errors.h"
#include "GLProgramManager.h"
#include "MTRenderState.h"
#include "MainMenuSystemViewer.h"
#include "RenderUtils.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatch.h"


const f64q FACE_ORIENTATIONS[6] = {
    f64q(f64v3(0.0, 0.0, 0.0)), // TOP
    f64q(f64v3(0.0, 0.0, -M_PI / 2.0)), // LEFT
    f64q(f64v3(0.0, 0.0, M_PI / 2.0)), // RIGHT
    f64q(f64v3(M_PI / 2.0, 0.0, 0.0)), // FRONT
    f64q(f64v3(-M_PI / 2.0, 0.0, 0.0)), // BACK
    f64q(f64v3(M_PI, 0.0, 0.0))  // BOTTOM
};

SpaceSystemRenderStage::SpaceSystemRenderStage(ui32v2 viewport,
                                               SpaceSystem* spaceSystem,
                                               GameSystem* gameSystem,
                                               const MainMenuSystemViewer* systemViewer,
                                               const Camera* spaceCamera,
                                               const Camera* farTerrainCamera,
                                               VGTexture selectorTexture) :
    m_viewport(viewport),
    m_spaceSystem(spaceSystem),
    m_gameSystem(gameSystem),
    m_mainMenuSystemViewer(systemViewer),
    m_spaceCamera(spaceCamera),
    m_farTerrainCamera(farTerrainCamera),
    m_selectorTexture(selectorTexture) {
    // Empty
}

SpaceSystemRenderStage::~SpaceSystemRenderStage() {
    // Empty
}

void SpaceSystemRenderStage::setRenderState(const MTRenderState* renderState) {
    m_renderState = renderState;
}

void SpaceSystemRenderStage::draw() {

    drawBodies();
    m_systemARRenderer.draw(m_spaceSystem, m_spaceCamera,
                            m_mainMenuSystemViewer, m_selectorTexture,
                            m_viewport);
   
}

void SpaceSystemRenderStage::drawBodies() {
    glEnable(GL_CULL_FACE);

    // TODO(Ben): Try to optimize out getFromEntity

    f64v3 lightPos;
    // For caching light for far terrain
    std::map<vecs::EntityID, std::pair<f64v3, SpaceLightComponent*> > lightCache;
    const f64v3* pos;
    // Render spherical terrain
    for (auto& it : m_spaceSystem->m_sphericalTerrainCT) {
        auto& cmp = it.second;
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(cmp.namePositionComponent);

        // If we are using MTRenderState, get position from it
        pos = getBodyPosition(npCmp, it.first);

        SpaceLightComponent* lightCmp = getBrightestLight(cmp, lightPos);
        lightCache[it.first] = std::make_pair(lightPos, lightCmp);

        f32v3 lightDir(glm::normalize(lightPos - *pos));

        m_sphericalTerrainComponentRenderer.draw(cmp, m_spaceCamera,
                                                 lightDir,
                                                 *pos,
                                                 lightCmp,
                                                 &m_spaceSystem->m_axisRotationCT.get(cmp.axisRotationComponent),
                                                 &m_spaceSystem->m_atmosphereCT.getFromEntity(it.first));
    }

    // Render atmospheres
    for (auto& it : m_spaceSystem->m_atmosphereCT) {
        auto& atCmp = it.second;
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(atCmp.namePositionComponent);

        pos = getBodyPosition(npCmp, it.first);

        f32v3 relCamPos(m_spaceCamera->getPosition() - *pos);

        if (glm::length(relCamPos) < 50000.0f) {
            auto& l = lightCache[it.first];

            f32v3 lightDir(glm::normalize(l.first - *pos));

            m_atmosphereComponentRenderer.draw(atCmp, m_spaceCamera->getViewProjectionMatrix(), relCamPos, lightDir, l.second);
        }
    }

    // Render far terrain
    if (m_farTerrainCamera) {
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
        }
    }

    vg::DepthState::FULL.set();
}

SpaceLightComponent* SpaceSystemRenderStage::getBrightestLight(SphericalTerrainComponent& cmp, OUT f64v3& pos) {
    SpaceLightComponent* rv = nullptr;
    f64 closestDist = 9999999999999999.0;
    for (auto& it : m_spaceSystem->m_spaceLightCT) {
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(it.second.parentNpId);
        // TODO(Ben): Optimize out sqrt
        f64 dist = glm::length(npCmp.position - m_spaceSystem->m_namePositionCT.get(cmp.namePositionComponent).position);
        if (dist < closestDist) {
            closestDist = dist;
            rv = &it.second;
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

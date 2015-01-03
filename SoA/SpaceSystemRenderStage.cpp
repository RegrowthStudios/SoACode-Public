#include "stdafx.h"
#include "SpaceSystem.h"
#include "SpaceSystemRenderStage.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <Vorb/DepthState.h>
#include <Vorb/GLProgram.h>
#include <Vorb/SamplerState.h>
#include <Vorb/SpriteBatch.h>
#include <Vorb/SpriteFont.h>
#include <Vorb/colors.h>

#include "App.h"
#include "AxisRotationComponent.h"
#include "Camera.h"
#include "DebugRenderer.h"
#include "GLProgramManager.h"
#include "RenderUtils.h"

SpaceSystemRenderStage::SpaceSystemRenderStage(ui32v2 viewport,
                                               SpaceSystem* spaceSystem,
                                               const Camera* camera,
                                               vg::GLProgram* colorProgram,
                                               vg::GLProgram* terrainProgram,
                                               vg::GLProgram* waterProgram,
                                               VGTexture selectorTexture) :
    m_viewport(viewport),
    m_spaceSystem(spaceSystem),
    m_camera(camera),
    m_colorProgram(colorProgram),
    m_terrainProgram(terrainProgram),
    m_waterProgram(waterProgram),
    m_selectorTexture(selectorTexture) {
    // Empty
}

SpaceSystemRenderStage::~SpaceSystemRenderStage() {
    // Empty
}

void SpaceSystemRenderStage::draw() {
    drawBodies();
    drawPaths();
    drawHud();
}

void SpaceSystemRenderStage::drawBodies() {

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    static DebugRenderer debugRenderer;
    m_spaceSystem->m_mutex.lock();
    for (auto& it : m_spaceSystem->m_sphericalGravityCT) {
        auto& sgcmp = it.second;
        float radius = sgcmp.radius;
        const f64v3& position = m_spaceSystem->m_namePositionCT.getFromEntity(it.first).position;

        debugRenderer.drawIcosphere(f32v3(0), radius * 0.99, f32v4(1.0), 4);

        const AxisRotationComponent& axisRotComp = m_spaceSystem->m_axisRotationCT.getFromEntity(it.first);

        f32m4 rotationMatrix = f32m4(glm::toMat4(axisRotComp.currentOrientation));

        f32m4 WVP = m_camera->getProjectionMatrix() * m_camera->getViewMatrix();

        debugRenderer.render(WVP, f32v3(m_camera->getPosition() - position), rotationMatrix);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    m_terrainProgram->use();
    glUniform1i(m_terrainProgram->getUniform("unNormalMap"), 0);
    glUniform1i(m_terrainProgram->getUniform("unColorMap"), 1);
    glUniform1i(m_terrainProgram->getUniform("unTexture"), 2);
    glUniform1f(m_terrainProgram->getUniform("unTexelWidth"), (float)PATCH_NORMALMAP_WIDTH);
    m_waterProgram->use();
    glUniform1i(m_waterProgram->getUniform("unNormalMap"), 0);
    glUniform1i(m_waterProgram->getUniform("unColorMap"), 1);

    for (auto& it : m_spaceSystem->m_sphericalTerrainCT) {
        auto& cmp = it.second;

        cmp.draw(m_camera, m_terrainProgram, m_waterProgram,
                 &m_spaceSystem->m_namePositionCT.getFromEntity(it.first),
                 &m_spaceSystem->m_axisRotationCT.getFromEntity(it.first));
    }
    m_waterProgram->unuse();
    m_spaceSystem->m_mutex.unlock();


    DepthState::FULL.set();
}

void SpaceSystemRenderStage::drawPaths() {

    // Draw paths
    m_colorProgram->use();
    m_colorProgram->enableVertexAttribArrays();
    glDepthMask(GL_FALSE);
    glLineWidth(3.0f);

    f32m4 wvp = m_camera->getProjectionMatrix() * m_camera->getViewMatrix();
    m_spaceSystem->m_mutex.lock();
    for (auto& it : m_spaceSystem->m_orbitCT) {
        auto& cmp = it.second;
        if (cmp.parentNpId) {
            cmp.drawPath(m_colorProgram, wvp, &m_spaceSystem->m_namePositionCT.getFromEntity(it.first),
                         m_camera->getPosition(), &m_spaceSystem->m_namePositionCT.get(cmp.parentNpId));
        } else {
            cmp.drawPath(m_colorProgram, wvp, &m_spaceSystem->m_namePositionCT.getFromEntity(it.first),
                         m_camera->getPosition());
        }
    }
    m_spaceSystem->m_mutex.unlock();
    m_colorProgram->disableVertexAttribArrays();
    m_colorProgram->unuse();
    glDepthMask(GL_TRUE);
}


void SpaceSystemRenderStage::drawHud() {

    // Lazily load spritebatch
    if (!m_spriteBatch.get()) {
        m_spriteBatch = std::make_unique<SpriteBatch>(true, true);
        m_spriteFont = std::make_unique<SpriteFont>("Fonts/orbitron_bold-webfont.ttf", 32);
    }

    // Reset the yOffset
    float yOffset = 0.0f;
    float fontHeight = m_spriteFont->getFontHeight();

    m_spriteBatch->begin();

    /*m_spriteBatch->drawString(m_spriteFont,
    ("Name: " + getTargetName()).c_str(),
    f32v2(0.0f, yOffset),
    f32v2(1.0f),
    color::White);
    yOffset += fontHeight;
    m_spriteBatch->drawString(m_spriteFont,
    ("Radius: " + std::to_string(getTargetRadius()) + " KM").c_str(),
    f32v2(0.0f, yOffset),
    f32v2(1.0f),
    color::White);
    yOffset += fontHeight;*/



    // Render all bodies
    for (auto& it : m_spaceSystem->m_namePositionCT) {
        vcore::ComponentID componentID;
        color4 textColor = color::White;
        f32v2 selectorSize(32.0f, 32.0f);
        const f32v2 textOffset(14.0f, -30.0f);
        f64v3 relativePos = it.second.position - m_camera->getPosition();
        f64 distance = glm::length(relativePos);
        float radiusPixels;
        if (m_camera->pointInFrustum(f32v3(relativePos))) {
            // Get screen position 
            f32v2 screenCoords = m_camera->worldToScreenPoint(relativePos);

            // See if it has a radius
            componentID = m_spaceSystem->m_sphericalGravityCT.getComponentID(it.first);
            if (componentID) {
                // Get radius of projected sphere
                radiusPixels = (m_spaceSystem->m_sphericalGravityCT.get(componentID).radius /
                                (tan(m_camera->getFieldOfView() / 2) * distance)) *
                                (m_viewport.y / 2.0f);
            } else {
                radiusPixels = (m_spaceSystem->m_sphericalGravityCT.get(componentID).radius /
                                (tan(m_camera->getFieldOfView() / 2) * distance)) *
                                (m_viewport.y / 2.0f);
            }

            if (radiusPixels < 16.0f) {

                // Draw Indicator
                m_spriteBatch->draw(m_selectorTexture, screenCoords * m_viewport - selectorSize / 2.0f, selectorSize, textColor);
                // Draw Text
                m_spriteBatch->drawString(m_spriteFont.get(),
                                          it.second.name.c_str(),
                                          screenCoords * m_viewport + textOffset,
                                          f32v2(0.5f),
                                          textColor);
            }

        }
    }

    m_spriteBatch->end();
    m_spriteBatch->renderBatch(m_viewport);
}
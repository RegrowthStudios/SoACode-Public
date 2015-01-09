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
#include <Vorb/utils.h>

#include "App.h"
#include "AxisRotationComponent.h"
#include "Camera.h"
#include "DebugRenderer.h"
#include "Errors.h"
#include "GLProgramManager.h"
#include "OrbitComponent.h"
#include "RenderUtils.h"
#include "MainMenuSystemViewer.h"

SpaceSystemRenderStage::SpaceSystemRenderStage(ui32v2 viewport,
                                               SpaceSystem* spaceSystem,
                                               const MainMenuSystemViewer* systemViewer,
                                               const Camera* camera,
                                               vg::GLProgram* colorProgram,
                                               vg::GLProgram* terrainProgram,
                                               vg::GLProgram* waterProgram,
                                               VGTexture selectorTexture) :
    m_viewport(viewport),
    m_spaceSystem(spaceSystem),
    m_mainMenuSystemViewer(systemViewer),
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
    if (m_mainMenuSystemViewer) {
        drawPaths();
        drawHud();
    }
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

    DepthState::READ.set();

    // Draw paths
    m_colorProgram->use();
    m_colorProgram->enableVertexAttribArrays();
    glDepthMask(GL_FALSE);
    glLineWidth(3.0f);

    f32m4 wvp = m_camera->getProjectionMatrix() * m_camera->getViewMatrix();
    m_spaceSystem->m_mutex.lock();
    for (auto& it : m_spaceSystem->m_orbitCT) {

        // Get the augmented reality data
        const MainMenuSystemViewer::BodyArData* bodyArData = m_mainMenuSystemViewer->finBodyAr(it.first);
        if (bodyArData == nullptr) continue;

        // Interpolated alpha
        float alpha = 0.15 + 0.85 * hermite(bodyArData->hoverTime);

        auto& cmp = it.second;
        if (cmp.parentNpId) {
            m_orbitComponentRenderer.drawPath(cmp, m_colorProgram, wvp, &m_spaceSystem->m_namePositionCT.getFromEntity(it.first),
                         m_camera->getPosition(), alpha, &m_spaceSystem->m_namePositionCT.get(cmp.parentNpId));
        } else {
            m_orbitComponentRenderer.drawPath(cmp, m_colorProgram, wvp, &m_spaceSystem->m_namePositionCT.getFromEntity(it.first),
                         m_camera->getPosition(), alpha);
        }
    }
    m_spaceSystem->m_mutex.unlock();
    m_colorProgram->disableVertexAttribArrays();
    m_colorProgram->unuse();
    glDepthMask(GL_TRUE);
}


void SpaceSystemRenderStage::drawHud() {

    const float ROTATION_FACTOR = M_PI * 2.0f + M_PI / 4;
    static float dt = 0.0;
    dt += 0.01;

    // Lazily load spritebatch
    if (!m_spriteBatch) {
        m_spriteBatch = new SpriteBatch(true, true);
        m_spriteFont = new SpriteFont("Fonts/orbitron_bold-webfont.ttf", 32);
    }

    m_spriteBatch->begin();

    // Render all bodies
    for (auto& it : m_spaceSystem->m_namePositionCT) {

        // Get the augmented reality data
        const MainMenuSystemViewer::BodyArData* bodyArData = m_mainMenuSystemViewer->finBodyAr(it.first);
        if (bodyArData == nullptr) continue;

        vcore::ComponentID componentID;

        f64v3 position = it.second.position;
        f64v3 relativePos = position - m_camera->getPosition();
        color4 textColor;

        float hoverTime = bodyArData->hoverTime;

        if (bodyArData->inFrustum) {

            // Get screen position 
            f32v3 screenCoords = m_camera->worldToScreenPoint(relativePos);
            f32v2 xyScreenCoords(screenCoords.x * m_viewport.x, screenCoords.y * m_viewport.y);

            // Get a smooth interpolator with hermite
            float interpolator = hermite(hoverTime);
            
            // Find its orbit path color and do color interpolation
            componentID = m_spaceSystem->m_orbitCT.getComponentID(it.first);
            if (componentID) {
                const ui8v4& tcolor = m_spaceSystem->m_orbitCT.get(componentID).pathColor;
                ColorRGBA8 targetColor(tcolor.r, tcolor.g, tcolor.b, tcolor.a);
                textColor.interpolate(color::White, targetColor, interpolator);
            } else {
                textColor.interpolate(color::White, color::Aquamarine, interpolator);
            }

            float selectorSize = bodyArData->selectorSize;
          
            // Only render if it isn't too big
            if (selectorSize < MainMenuSystemViewer::MAX_SELECTOR_SIZE) {

                // Draw Indicator
                m_spriteBatch->draw(m_selectorTexture, nullptr, nullptr,
                                    xyScreenCoords,
                                    f32v2(0.5f, 0.5f),
                                    f32v2(selectorSize),
                                    interpolator * ROTATION_FACTOR,
                                    textColor, screenCoords.z);

                // Text offset and scaling
                const f32v2 textOffset(selectorSize / 2.0f, -selectorSize / 2.0f);
                const f32v2 textScale((((selectorSize - MainMenuSystemViewer::MIN_SELECTOR_SIZE) /
                    (MainMenuSystemViewer::MAX_SELECTOR_SIZE - MainMenuSystemViewer::MIN_SELECTOR_SIZE)) * 0.5 + 0.5) * 0.6);

                // Draw Text
                m_spriteBatch->drawString(m_spriteFont,
                                          it.second.name.c_str(),
                                          xyScreenCoords + textOffset,
                                          textScale,
                                          textColor,
                                          screenCoords.z);

            }
            // Land selector
            if (bodyArData->isLandSelected) {
                f32v3 selectedPos = bodyArData->selectedPos;
                // Apply axis rotation if applicable
                componentID = m_spaceSystem->m_axisRotationCT.getComponentID(it.first);
                if (componentID) {
                    f64q rot = m_spaceSystem->m_axisRotationCT.get(componentID).currentOrientation;
                    selectedPos = f32v3(rot * f64v3(selectedPos));
                }

                relativePos = (position + f64v3(selectedPos)) - m_camera->getPosition();
                screenCoords = m_camera->worldToScreenPoint(relativePos);
                xyScreenCoords = f32v2(screenCoords.x * m_viewport.x, screenCoords.y * m_viewport.y);

                color4 sColor = color::Red;
                sColor.a = 155;
                m_spriteBatch->draw(m_selectorTexture, nullptr, nullptr,
                                    xyScreenCoords,
                                    f32v2(0.5f, 0.5f),
                                    f32v2(22.0f) + cos(dt * 8.0f) * 4.0f,
                                    dt * ROTATION_FACTOR,
                                    sColor, 0.0f);
            }
        }
    }

    m_spriteBatch->end();
    m_spriteBatch->renderBatch(m_viewport, nullptr, &DepthState::READ);

    // Restore depth state
    DepthState::FULL.set();
}
#include "stdafx.h"
#include "SystemARRenderer.h"

#include "Camera.h"
#include "SpaceSystem.h"
#include "MainMenuSystemViewer.h"

#include <Vorb/graphics/DepthState.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/SamplerState.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/colors.h>
#include <Vorb/utils.h>

const cString VERT_SRC = R"(
uniform mat4 unWVP;
in vec4 vPosition;
void main() {
    gl_Position = unWVP * vPosition;
}
)";

const cString FRAG_SRC = R"(
uniform vec4 unColor;
out vec4 pColor;
void main() {
    pColor = unColor;
}
)";

SystemARRenderer::~SystemARRenderer() {
    if (m_colorProgram) {
        m_colorProgram->dispose();
        delete m_colorProgram;
    }
    if (m_spriteBatch) {
        m_spriteBatch->dispose();
        m_spriteFont->dispose();
        delete m_spriteBatch;
        delete m_spriteFont;
    }
}

void SystemARRenderer::draw(SpaceSystem* spaceSystem, const Camera* camera,
                            OPT const MainMenuSystemViewer* systemViewer, VGTexture selectorTexture,
                            const f32v2& viewport) {
    // Lazy shader init
    if (!m_colorProgram) {
        buildShader();
    }

    // Get handles so we don't have huge parameter lists
    m_spaceSystem = spaceSystem;
    m_camera = camera;
    m_systemViewer = systemViewer;
    m_selectorTexture = selectorTexture;
    m_viewport = viewport;

    drawPaths();
    if (m_systemViewer) {
        drawHUD();
    }
}

void SystemARRenderer::buildShader() {
    m_colorProgram = new vg::GLProgram(true);
    m_colorProgram->addShader(vg::ShaderType::VERTEX_SHADER, VERT_SRC);
    m_colorProgram->addShader(vg::ShaderType::FRAGMENT_SHADER, FRAG_SRC);
    m_colorProgram->link();
    m_colorProgram->initAttributes();
    m_colorProgram->initUniforms();
    if (!m_colorProgram->getIsLinked()) {
        std::cerr << "Failed to link shader for SystemARRenderer\n";
    }
}

void SystemARRenderer::drawPaths() {

    vg::DepthState::READ.set();
    float alpha;

    // Draw paths
    m_colorProgram->use();
    m_colorProgram->enableVertexAttribArrays();
    glDepthMask(GL_FALSE);
    glLineWidth(3.0f);

    f32m4 wvp = m_camera->getProjectionMatrix() * m_camera->getViewMatrix();
    for (auto& it : m_spaceSystem->m_orbitCT) {

        // Get the augmented reality data
        if (m_systemViewer) {
            const MainMenuSystemViewer::BodyArData* bodyArData = m_systemViewer->finBodyAr(it.first);
            if (bodyArData == nullptr) continue;

            // Interpolated alpha
            alpha = 0.15 + 0.85 * hermite(bodyArData->hoverTime);
        } else {
            alpha = 0.15;
        }

        auto& cmp = it.second;
        if (cmp.parentOrbId) {
            OrbitComponent& pOrbCmp = m_spaceSystem->m_orbitCT.get(cmp.parentOrbId);
            m_orbitComponentRenderer.drawPath(cmp, m_colorProgram, wvp, &m_spaceSystem->m_namePositionCT.getFromEntity(it.first),
                                              m_camera->getPosition(), alpha, &m_spaceSystem->m_namePositionCT.get(pOrbCmp.npID));
        } else {
            m_orbitComponentRenderer.drawPath(cmp, m_colorProgram, wvp, &m_spaceSystem->m_namePositionCT.getFromEntity(it.first),
                                              m_camera->getPosition(), alpha);
        }
    }
    m_colorProgram->disableVertexAttribArrays();
    m_colorProgram->unuse();
    glDepthMask(GL_TRUE);
}

void SystemARRenderer::drawHUD() {
    // Currently we need a viewer for this
    if (!m_systemViewer) return;

    const f32 ROTATION_FACTOR = (f32)(M_PI * 2.0 + M_PI / 4.0);
    static f32 dt = 0.0;
    dt += 0.01;

    // Lazily load spritebatch
    if (!m_spriteBatch) {
        m_spriteBatch = new vg::SpriteBatch(true, true);
        m_spriteFont = new vg::SpriteFont();
        m_spriteFont->init("Fonts/orbitron_bold-webfont.ttf", 32);
    }

    m_spriteBatch->begin();

    // Render all bodies
    for (auto& it : m_spaceSystem->m_namePositionCT) {

        // Get the augmented reality data
        const MainMenuSystemViewer::BodyArData* bodyArData = m_systemViewer->finBodyAr(it.first);
        if (bodyArData == nullptr) continue;

        vecs::ComponentID componentID;

        f64v3 position = it.second.position;
        f64v3 relativePos = position - m_camera->getPosition();
        color4 textColor;

        f32 hoverTime = bodyArData->hoverTime;

        if (bodyArData->inFrustum) {

            // Get screen position 
            f32v3 screenCoords = m_camera->worldToScreenPoint(relativePos);
            f32v2 xyScreenCoords(screenCoords.x * m_viewport.x, screenCoords.y * m_viewport.y);

            // Get a smooth interpolator with hermite
            f32 interpolator = hermite(hoverTime);

            // Find its orbit path color and do color interpolation
            componentID = m_spaceSystem->m_orbitCT.getComponentID(it.first);
            if (componentID) {
                const ui8v4& tcolor = m_spaceSystem->m_orbitCT.get(componentID).pathColor;
                ColorRGBA8 targetColor(tcolor.r, tcolor.g, tcolor.b, tcolor.a);
                textColor.lerp(color::White, targetColor, interpolator);
            } else {
                textColor.lerp(color::White, color::Aquamarine, interpolator);
            }

            f32 selectorSize = bodyArData->selectorSize;

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
    m_spriteBatch->renderBatch(m_viewport, nullptr, &vg::DepthState::READ);

    // Restore depth state
    vg::DepthState::FULL.set();
}

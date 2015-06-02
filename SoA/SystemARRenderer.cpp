#include "stdafx.h"
#include "SystemARRenderer.h"

#include "Camera.h"
#include "MainMenuSystemViewer.h"
#include "ModPathResolver.h"
#include "ShaderLoader.h"
#include "SpaceSystem.h"
#include "soaUtils.h"

#include <Vorb/colors.h>
#include <Vorb/graphics/DepthState.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/SamplerState.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/utils.h>

namespace {
    const cString VERT_SRC = R"(
uniform mat4 unWVP;
in vec4 vPosition;
in float vAngle;
out float fAngle;
#include "Shaders/Utils/logz.glsl"
void main() {
    fAngle = vAngle;
    gl_Position = unWVP * vPosition;
    applyLogZ();
}
)";

    const cString FRAG_SRC = R"(
uniform vec4 unColor;
uniform float currentAngle;
in float fAngle;
out vec4 pColor;
void main() {
    pColor = unColor * vec4(1.0, 1.0, 1.0, 1.0 - mod(fAngle + currentAngle, 1.0));
}
)";

    const cString SPRITE_VERT_SRC = R"(
uniform mat4 World;
uniform mat4 VP;

in vec4 vPosition;
in vec2 vUV;
in vec4 vUVRect;
in vec4 vTint;

out vec2 fUV;
flat out vec4 fUVRect;
out vec4 fTint;

#include "Shaders/Utils/logz.glsl"

void main() {
    fTint = vTint;
    fUV = vUV;
    fUVRect = vUVRect;
    vec4 worldPos = World * vPosition;
    gl_Position = VP * worldPos;
    applyLogZ();
}
)";
    const cString SPRITE_FRAG_SRC = R"(
uniform sampler2D SBTex;

in vec2 fUV;
flat in vec4 fUVRect;
in vec4 fTint;

out vec4 fColor;

void main() {
    fColor = texture(SBTex, fract(fUV.xy) * fUVRect.zw + fUVRect.xy) * fTint;
}
)";
}

SystemARRenderer::SystemARRenderer(const ModPathResolver* textureResolver) :
    m_textureResolver(textureResolver) {
    // Empty
}

SystemARRenderer::~SystemARRenderer() {
    dispose();
}

void SystemARRenderer::draw(SpaceSystem* spaceSystem, const Camera* camera,
                            OPT const MainMenuSystemViewer* systemViewer,
                            const f32v2& viewport) {
    // Lazy init
    if (!m_colorProgram) m_colorProgram = ShaderLoader::createProgram("SystemAR", VERT_SRC, FRAG_SRC);
    if (m_selectorTexture == 0) loadTextures();

    // Get handles so we don't have huge parameter lists
    m_spaceSystem = spaceSystem;
    m_camera = camera;
    m_systemViewer = systemViewer;
    m_viewport = viewport;
    m_zCoef = computeZCoef(camera->getFarClip());
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    drawPaths();
    if (m_systemViewer) {
        if (!m_spriteProgram) m_spriteProgram = ShaderLoader::createProgram("SystemARHUD", SPRITE_VERT_SRC, SPRITE_FRAG_SRC);
        drawHUD();
    }
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SystemARRenderer::dispose() {
    if (m_colorProgram) {
        m_colorProgram->dispose();
        delete m_colorProgram;
        m_colorProgram = nullptr;
    }
    if (m_spriteProgram) {
        m_spriteProgram->dispose();
        delete m_spriteProgram;
        m_spriteProgram = nullptr;
    }
    if (m_spriteBatch) {
        m_spriteBatch->dispose();
        m_spriteFont->dispose();
        delete m_spriteBatch;
        m_spriteBatch = nullptr;
        delete m_spriteFont;
        m_spriteFont = nullptr;
    }
}

void SystemARRenderer::loadTextures() {
    { // Selector
        vio::Path path;
        m_textureResolver->resolvePath("GUI/selector.png", path);
        vg::ScopedBitmapResource res = vg::ImageIO().load(path);
        if (!res.data) {
            fprintf(stderr, "ERROR: Failed to load GUI/selector.png\n");
        }
        m_selectorTexture = vg::GpuMemory::uploadTexture(&res, vg::TexturePixelType::UNSIGNED_BYTE,
                                                     vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_CLAMP_MIPMAP);
    }
    { // Barycenter
        vio::Path path;
        m_textureResolver->resolvePath("GUI/barycenter.png", path);
        vg::ScopedBitmapResource res = vg::ImageIO().load(path);
        if (!res.data) {
            fprintf(stderr, "ERROR: Failed to load GUI/barycenter.png\n");
        }
        m_baryTexture = vg::GpuMemory::uploadTexture(&res, vg::TexturePixelType::UNSIGNED_BYTE,
                                                     vg::TextureTarget::TEXTURE_2D, &vg::SamplerState::LINEAR_CLAMP_MIPMAP);
    }
}

void SystemARRenderer::drawPaths() {

    //vg::DepthState::READ.set();
    float blendFactor;

    // Draw paths
    m_colorProgram->use();
    m_colorProgram->enableVertexAttribArrays();

    // For logarithmic Z buffer
    glUniform1f(m_colorProgram->getUniform("unZCoef"), m_zCoef);
    glLineWidth(3.0f);

    f32m4 wvp = m_camera->getProjectionMatrix() * m_camera->getViewMatrix();
    for (auto& it : m_spaceSystem->m_orbitCT) {
        auto& cmp = it.second;
    
        bool isSelected = false;
        f32v4 oldPathColor; // To cache path color since we force it to a different one
        if (m_systemViewer) {
            // Get the augmented reality data
            const MainMenuSystemViewer::BodyArData* bodyArData = m_systemViewer->finBodyAr(it.first);
            if (bodyArData == nullptr) continue;

            ui8v3 ui8Color;
            // If its selected we force a different color
            if (m_systemViewer->getTargetBody() == it.first) {
                isSelected = true;
                oldPathColor = cmp.pathColor[0];
                cmp.pathColor[0] = m_spaceSystem->pathColorMap["Selected"].second;
                blendFactor = 0.0;
            } else {
                // Hermite interpolated alpha
                blendFactor = hermite(bodyArData->hoverTime);
            }
        } else {
            blendFactor = 0.0;
        }

        if (cmp.parentOrbId) {
            OrbitComponent& pOrbCmp = m_spaceSystem->m_orbitCT.get(cmp.parentOrbId);
            m_orbitComponentRenderer.drawPath(cmp, m_colorProgram, wvp, &m_spaceSystem->m_namePositionCT.getFromEntity(it.first),
                                              m_camera->getPosition(), blendFactor, &m_spaceSystem->m_namePositionCT.get(pOrbCmp.npID));
        } else {
            m_orbitComponentRenderer.drawPath(cmp, m_colorProgram, wvp, &m_spaceSystem->m_namePositionCT.getFromEntity(it.first),
                                              m_camera->getPosition(), blendFactor);
        }

        // Restore path color
        if (isSelected) cmp.pathColor[0] = oldPathColor;
    }
    m_colorProgram->disableVertexAttribArrays();
    m_colorProgram->unuse();
}

void SystemARRenderer::drawHUD() {
    // Currently we need a viewer for this
    if (!m_systemViewer) return;

    const f32 ROTATION_FACTOR = (f32)(M_PI + M_PI / 4.0);
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
    for (auto& it : m_spaceSystem->m_orbitCT) {

        auto& oCmp = it.second;
        auto& npCmp = m_spaceSystem->m_namePositionCT.get(oCmp.npID);

        // Get the augmented reality data
        const MainMenuSystemViewer::BodyArData* bodyArData = m_systemViewer->finBodyAr(it.first);
        if (bodyArData == nullptr) continue;

        if (bodyArData->inFrustum) {

            f64v3 position = npCmp.position;
            f64v3 relativePos = position - m_camera->getPosition();
            f64 distance = glm::length(relativePos);
            color4 textColor;

            f32 hoverTime = bodyArData->hoverTime;

            // Get screen position 
            f32v3 screenCoords = m_camera->worldToScreenPoint(relativePos);
            f32v2 xyScreenCoords(screenCoords.x * m_viewport.x, screenCoords.y * m_viewport.y);

            // Get a smooth interpolator with hermite
            f32 interpolator = hermite(hoverTime);

            // Calculate colors
            ui8v3 ui8Color;
            // If its selected we use a different color
            bool isSelected = false;
            if (m_systemViewer->getTargetBody() == it.first) {
                isSelected = true;
                ui8Color = ui8v3(m_spaceSystem->pathColorMap["Selected"].second * 255.0f);
            } else {
                ui8Color = ui8v3(lerp(oCmp.pathColor[0], oCmp.pathColor[1], interpolator) * 255.0f);
            }
            color4 oColor(ui8Color.r, ui8Color.g, ui8Color.b, 255u);
            textColor.lerp(color::LightGray, color::White, interpolator);

            f32 selectorSize = bodyArData->selectorSize;

            // Only render if it isn't too big
            if (selectorSize < MainMenuSystemViewer::MAX_SELECTOR_SIZE) {

                // Alpha interpolation from size so they fade out
                f32 low = MainMenuSystemViewer::MAX_SELECTOR_SIZE * 0.7f;
                if (selectorSize > low) {
                    // Fade out when close
                    oColor.a = (ui8)((1.0f - (selectorSize - low) /
                        (MainMenuSystemViewer::MAX_SELECTOR_SIZE - low)) * 255);
                    textColor.a = oColor.a;
                } else {
                    f64 d = distance - (f64)low;
                    // Fade name based on distance
                    switch (oCmp.type) {
                        case SpaceObjectType::STAR:
                            textColor.a = oColor.a = (ui8)(glm::max(0.0, (f64)textColor.a - d * 0.00000000001));
                            break;
                        case SpaceObjectType::BARYCENTER:
                        case SpaceObjectType::PLANET:
                        case SpaceObjectType::DWARF_PLANET:
                            textColor.a = oColor.a = (ui8)(glm::max(0.0, (f64)textColor.a - d * 0.000000001));
                            break;
                        default:
                            textColor.a = oColor.a = (ui8)(glm::max(0.0, (f64)textColor.a - d * 0.000001));
                            break;
                    }
                }

                // Pick texture
                VGTexture tx;
                if (oCmp.type == SpaceObjectType::BARYCENTER) {
                    tx = m_baryTexture;
                    selectorSize = MainMenuSystemViewer::MIN_SELECTOR_SIZE * 2.5f - distance * 0.00000000001;
                    if (selectorSize < 0.0) continue;        
                    interpolator = 0.0f; // Don't rotate barycenters
                } else {
                    tx = m_selectorTexture;
                }

                // Draw Indicator
                m_spriteBatch->draw(tx, nullptr, nullptr,
                                    xyScreenCoords,
                                    f32v2(0.5f, 0.5f),
                                    f32v2(selectorSize),
                                    interpolator * ROTATION_FACTOR,
                                    oColor, screenCoords.z);

                // Text offset and scaling
                const f32v2 textOffset(selectorSize / 2.0f, -selectorSize / 2.0f);
                const f32v2 textScale((((selectorSize - MainMenuSystemViewer::MIN_SELECTOR_SIZE) /
                    (MainMenuSystemViewer::MAX_SELECTOR_SIZE - MainMenuSystemViewer::MIN_SELECTOR_SIZE)) * 0.5 + 0.5) * 0.6);

                // Draw Text
                if (textColor.a > 0) {
                    m_spriteBatch->drawString(m_spriteFont,
                                              npCmp.name.c_str(),
                                              xyScreenCoords + textOffset,
                                              textScale,
                                              textColor,
                                              vg::TextAlign::TOP_LEFT,
                                              screenCoords.z);
                }

            }
            // Land selector
            if (isSelected && bodyArData->isLandSelected) {
                f32v3 selectedPos = bodyArData->selectedPos;
                // Apply axis rotation if applicable
                vecs::ComponentID componentID = m_spaceSystem->m_axisRotationCT.getComponentID(it.first);
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
    m_spriteBatch->render(m_viewport, nullptr, &vg::DepthState::READ, nullptr, m_spriteProgram);

    // Restore depth state
    vg::DepthState::FULL.set();
}

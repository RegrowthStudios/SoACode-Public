#include "stdafx.h"
#include "StarComponentRenderer.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "ShaderLoader.h"
#include "SpaceSystemComponents.h"
#include "RenderUtils.h"

#include <Vorb/MeshGenerators.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/RasterizerState.h>
#include <Vorb/graphics/ShaderManager.h>
#include <Vorb/graphics/SamplerState.h>
#include <Vorb/utils.h>

#define MIN_TMP 800.0
#define TMP_RANGE 29200.0
#define ICOSPHERE_SUBDIVISIONS 5

StarComponentRenderer::StarComponentRenderer() {
    m_tempColorMap.width = -1;
}

StarComponentRenderer::~StarComponentRenderer() {
    dispose();
}

void StarComponentRenderer::drawStar(const StarComponent& sCmp,
                                     const f32m4& VP,
                                     const f64q& orientation,
                                     const f32v3& relCamPos) {
    checkLazyLoad();

    m_starProgram->use();

    // Calculate color
    f32v3 tColor = calculateStarColor(sCmp);

    // Convert f64q to f32q
    f32q orientationF32;
    orientationF32.x = (f32)orientation.x;
    orientationF32.y = (f32)orientation.y;
    orientationF32.z = (f32)orientation.z;
    orientationF32.w = (f32)orientation.w;

    // Convert to matrix
    f32m4 rotationMatrix = glm::toMat4(orientationF32);

    // Set up matrix
    f32m4 WVP(1.0);
    setMatrixTranslation(WVP, -relCamPos);
    WVP = VP * WVP * glm::scale(f32v3(sCmp.radius)) * rotationMatrix;

    f32v3 rotRelCamPos = relCamPos * orientationF32;

    // Upload uniforms
    static f32 dt = 1.0f;
    dt += 0.001f;
    glUniform1f(unDT, dt);
    glUniformMatrix4fv(unWVP, 1, GL_FALSE, &WVP[0][0]);
    glUniform3fv(m_starProgram->getUniform("unColor"), 1, &tColor[0]);

    // Bind VAO
    glBindVertexArray(m_sVao);

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    m_starProgram->unuse();
}

void StarComponentRenderer::drawCorona(const StarComponent& sCmp,
                                       const f32m4& VP,
                                       const f32m4& V,
                                       const f32v3& relCamPos) {
    m_coronaProgram->use();

    // Corona color
    f32v3 tColor = f32v3(0.9f);
    tColor += getTempBrightnessMod(sCmp);

    // Upload uniforms
    f32v3 center(-relCamPos);
    f32v3 camRight(V[0][0], V[1][0], V[2][0]);
    f32v3 camUp(V[0][1], V[1][1], V[2][1]);
    glUniform3fv(m_coronaProgram->getUniform("unCameraRight"), 1, &camRight[0]);
    glUniform3fv(m_coronaProgram->getUniform("unCameraUp"), 1, &camUp[0]);
    glUniform3fv(m_coronaProgram->getUniform("unCenter"), 1, &center[0]);
    glUniform3fv(m_coronaProgram->getUniform("unColor"), 1, &tColor[0]);
    // Size
    glUniform1f(m_coronaProgram->getUniform("unMaxSize"), 4.0);
    glUniform1f(m_coronaProgram->getUniform("unStarRadius"), sCmp.radius);
    // Time
    static f32 dt = 1.0f;
    dt += 0.001f;
    glUniform1f(m_coronaProgram->getUniform("unDT"), dt);
    glUniformMatrix4fv(m_coronaProgram->getUniform("unWVP"), 1, GL_FALSE, &VP[0][0]);

    // Bind VAO
    glBindVertexArray(m_cVao);

    glDepthMask(GL_FALSE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glDepthMask(GL_TRUE);

    glBindVertexArray(0);
    m_coronaProgram->unuse();
}

void StarComponentRenderer::drawGlow(const StarComponent& sCmp,
                                     const f32m4& VP,
                                     const f32m4& V,
                                     const f32v3& relCamPos) {
    m_glowProgram->use();

    // Calculate color
    f32v3 tColor = calculateStarColor(sCmp);

    // Upload uniforms
    f32v3 center(-relCamPos);
    f32v3 camRight(V[0][0], V[1][0], V[2][0]);
    f32v3 camUp(V[0][1], V[1][1], V[2][1]);
    glUniform3fv(m_glowProgram->getUniform("unCameraRight"), 1, &camRight[0]);
    glUniform3fv(m_glowProgram->getUniform("unCameraUp"), 1, &camUp[0]);
    glUniform3fv(m_glowProgram->getUniform("unCenter"), 1, &center[0]);
    glUniform3fv(m_glowProgram->getUniform("unColor"), 1, &tColor[0]);
    // Size
    f32 size = (f32)(sCmp.radius * 32.0);
    // f32 scale = (sCmp.temperature - MIN_TMP) / TMP_RANGE;
    glUniform1f(m_glowProgram->getUniform("unSize"), size);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_glowTexture);

    // Time
    static f32 dt = 1.0f;
    dt += 0.0001f;
    glUniform1f(m_coronaProgram->getUniform("unDT"), dt);

    //  glUniform1i(m_glowProgram->getUniform("unTexture"), 0);

    glUniformMatrix4fv(m_glowProgram->getUniform("unWVP"), 1, GL_FALSE, &VP[0][0]);
    // Bind VAO
    glBindVertexArray(m_gVao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
    m_glowProgram->unuse();
}

void StarComponentRenderer::dispose() {
    disposeShaders();
    disposeBuffers();
    if (m_tempColorMap.data) {
        vg::ImageIO().free(m_tempColorMap);
        m_tempColorMap.data = nullptr;
    }
    if (m_glowTexture) vg::GpuMemory::freeTexture(m_glowTexture);
}

void StarComponentRenderer::disposeShaders() {
    if (m_starProgram) {
        vg::ShaderManager::destroyProgram(&m_starProgram);
    }
    if (m_coronaProgram) {
        vg::ShaderManager::destroyProgram(&m_coronaProgram);
    }
    if (m_glowProgram) {
        vg::ShaderManager::destroyProgram(&m_glowProgram);
    }
    disposeBuffers();
}

void StarComponentRenderer::disposeBuffers() {
    // Dispose buffers too for proper reload
    if (m_sVbo) {
        vg::GpuMemory::freeBuffer(m_sVbo);
    }
    if (m_sIbo) {
        vg::GpuMemory::freeBuffer(m_sIbo);
    }
    if (m_sVao) {
        glDeleteVertexArrays(1, &m_sVao);
        m_sVao = 0;
    }
    if (m_cVbo) {
        vg::GpuMemory::freeBuffer(m_cVbo);
    }
    if (m_cIbo) {
        vg::GpuMemory::freeBuffer(m_cIbo);
    }
    if (m_cVao) {
        glDeleteVertexArrays(1, &m_cVao);
        m_cVao = 0;
    }
    if (m_gVao) {
        glDeleteVertexArrays(1, &m_gVao);
        m_gVao = 0;
    }
}

void StarComponentRenderer::checkLazyLoad() {
    if (!m_starProgram) buildShaders();
    if (!m_sVbo) buildMesh();
    if (m_tempColorMap.width == -1) loadTempColorMap();
    if (m_glowTexture == 0) loadGlowTexture();
}

void StarComponentRenderer::buildShaders() {
    m_starProgram = ShaderLoader::createProgramFromFile("Shaders/Star/star.vert",
                                                        "Shaders/Star/star.frag");
    m_starProgram->use();
    unWVP = m_starProgram->getUniform("unWVP");
    unDT = m_starProgram->getUniform("unDT");
    m_starProgram->unuse();

    m_coronaProgram = ShaderLoader::createProgramFromFile("Shaders/Star/corona.vert",
                                                          "Shaders/Star/corona.frag");

    m_glowProgram = ShaderLoader::createProgramFromFile("Shaders/Star/glow.vert",
                                                          "Shaders/Star/glow.frag");
}

void StarComponentRenderer::buildMesh() {
    // Build star mesh
    std::vector<ui32> indices;
    std::vector<f32v3> positions;

    // TODO(Ben): Optimize with LOD for far viewing
    vmesh::generateIcosphereMesh(ICOSPHERE_SUBDIVISIONS, indices, positions);
    m_numIndices = indices.size();

    glGenVertexArrays(1, &m_sVao);
    glBindVertexArray(m_sVao);

    vg::GpuMemory::createBuffer(m_sVbo);
    vg::GpuMemory::createBuffer(m_sIbo);

    vg::GpuMemory::bindBuffer(m_sVbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_sVbo, vg::BufferTarget::ARRAY_BUFFER, positions.size() * sizeof(f32v3),
                                    positions.data(), vg::BufferUsageHint::STATIC_DRAW);

    vg::GpuMemory::bindBuffer(m_sIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_sIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ui32),
                                    indices.data(), vg::BufferUsageHint::STATIC_DRAW);

    m_starProgram->enableVertexAttribArrays();
    glVertexAttribPointer(m_starProgram->getAttribute("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, 0);
  

    // Build corona and glow mesh
    glGenVertexArrays(1, &m_cVao);
    glBindVertexArray(m_cVao);

    vg::GpuMemory::createBuffer(m_cVbo);
    vg::GpuMemory::createBuffer(m_cIbo);

    f32v2 cPositions[4] = {
        f32v2(-1.0f, 1.0f),
        f32v2(-1.0f, -1.0f),
        f32v2(1.0f, -1.0f),
        f32v2(1.0f, 1.0f)
    };

    ui16 cIndices[6] = { 0, 1, 2, 2, 3, 0 };

    vg::GpuMemory::bindBuffer(m_cVbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_cVbo, vg::BufferTarget::ARRAY_BUFFER, sizeof(cPositions),
                                    cPositions, vg::BufferUsageHint::STATIC_DRAW);

    vg::GpuMemory::bindBuffer(m_cIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_cIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, sizeof(cIndices),
                                    cIndices, vg::BufferUsageHint::STATIC_DRAW);

    m_coronaProgram->enableVertexAttribArrays();
    glVertexAttribPointer(m_coronaProgram->getAttribute("vPosition"), 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Build glow VAO
    glGenVertexArrays(1, &m_gVao);
    glBindVertexArray(m_gVao);

    vg::GpuMemory::bindBuffer(m_cVbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::bindBuffer(m_cIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);

    m_glowProgram->enableVertexAttribArrays();
    glVertexAttribPointer(m_glowProgram->getAttribute("vPosition"), 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
}

void StarComponentRenderer::loadTempColorMap() {
    m_tempColorMap = vg::ImageIO().load("StarSystems/star_spectrum.png");
    if (!m_tempColorMap.data) {
        fprintf(stderr, "ERROR: Failed to load StarSystems/star_spectrum.png\n");
    }
}

void StarComponentRenderer::loadGlowTexture() {
    vg::BitmapResource rs = vg::ImageIO().load("Textures/nostalgic_glow.png");
    if (!m_tempColorMap.data) {
        fprintf(stderr, "ERROR: Failed to load StarSystems/nostalgic_glow.png\n");
    } else {
        m_glowTexture = vg::GpuMemory::uploadTexture(rs.bytesUI8, rs.width, rs.height, &vg::SamplerState::LINEAR_CLAMP_MIPMAP);
        vg::ImageIO().free(rs);
    }
}

f32v3 StarComponentRenderer::calculateStarColor(const StarComponent& sCmp) {
    // Calculate temperature color
    f32v3 tColor;
    f32 scale = m_tempColorMap.width * (sCmp.temperature - MIN_TMP) / TMP_RANGE;
    scale = glm::clamp(scale, 0.0f, (f32)m_tempColorMap.width);
    int rScale = (int)(scale + 0.5f);
    int iScale = (int)scale;

    if (iScale < rScale) { // Interpolate down
        if (iScale == 0) {
            tColor = getColor(iScale);
        } else {
            tColor = lerp(getColor(iScale), getColor(rScale), scale - (f32)iScale);
        }
    } else { // Interpolate up
        if (rScale >= (int)m_tempColorMap.width-1) {
            tColor = getColor((int)m_tempColorMap.width - 1);
        } else {
            tColor = lerp(getColor(rScale), getColor(rScale + 1), scale - (f32)rScale);
        }
    }
    
    return tColor + getTempBrightnessMod(sCmp);
}

f32v3 StarComponentRenderer::getColor(int index) {
    const ui8v4& bytes = m_tempColorMap.bytesUI8v4[index];
    return f32v3(bytes.r / 255.0f, bytes.g / 255.0f, bytes.b / 255.0f);
}

f32 StarComponentRenderer::getTempBrightnessMod(const StarComponent& sCmp) {
    return sCmp.temperature * (0.021 / 255.0);
}

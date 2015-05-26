#include "stdafx.h"
#include "StarComponentRenderer.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "Errors.h"
#include "ModPathResolver.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"
#include "SpaceSystemComponents.h"
#include "soaUtils.h"

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

namespace {
    cString OCCLUSION_VERT_SRC = R"(
uniform vec3 unCenterScreenspace;
uniform float unSize;
in vec2 vPosition;
void main() {
    gl_Position.xyz = unCenterScreenspace;
    gl_Position.xy += vPosition * unSize;
	gl_Position.w = 1.0;
}

)";
    cString OCCLUSION_FRAG_SRC = R"(
out vec4 pColor;
void main() {
    pColor = vec4(0.0, 0.0, 0.0, 0.0);
}
)";
}

StarComponentRenderer::StarComponentRenderer(const ModPathResolver* textureResolver) :
    m_textureResolver(textureResolver) {
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
    if (sCmp.visibility == 0.0f) return;

    m_starProgram->use();

    // Calculate color
    f32v3 tColor = calculateStarColor(sCmp) + getTempColorShift(sCmp);

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

    // Upload uniforms
    // Upload uniforms
    static f64 dt = 0.0;
    dt += 0.0001;
    glUniform1f(unDT, (f32)dt);
    glUniformMatrix4fv(unWVP, 1, GL_FALSE, &WVP[0][0]);
    glUniform3fv(m_starProgram->getUniform("unColor"), 1, &tColor[0]);
    glUniform1f(m_starProgram->getUniform("unRadius"), sCmp.radius);
    f32v3 unCenterDir = glm::normalize(relCamPos);
    glUniform3fv(m_starProgram->getUniform("unCenterDir"), 1, &unCenterDir[0]);

    // Bind VAO
    glBindVertexArray(m_sVao);

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    m_starProgram->unuse();
}

void StarComponentRenderer::drawCorona(StarComponent& sCmp,
                                       const f32m4& VP,
                                       const f32m4& V,
                                       const f32v3& relCamPos) {
    checkLazyLoad();

    f32v3 center(-relCamPos);
    f32v3 camRight(V[0][0], V[1][0], V[2][0]);
    f32v3 camUp(V[0][1], V[1][1], V[2][1]);

    if (sCmp.visibility == 0.0f) return;

    m_coronaProgram->use();

    // Corona color
    f32v3 tColor = getTempColorShift(sCmp);

    // Upload uniforms
    glUniform3fv(m_coronaProgram->getUniform("unCameraRight"), 1, &camRight[0]);
    glUniform3fv(m_coronaProgram->getUniform("unCameraUp"), 1, &camUp[0]);
    glUniform3fv(m_coronaProgram->getUniform("unCenter"), 1, &center[0]);
    glUniform3fv(m_coronaProgram->getUniform("unColor"), 1, &tColor[0]);
    // Size
    glUniform1f(m_coronaProgram->getUniform("unMaxSize"), 4.0f);
    glUniform1f(m_coronaProgram->getUniform("unStarRadius"), (f32)sCmp.radius);
    // Time
    static f64 dt = 0.0;
    dt += 0.0001;
    glUniform1f(m_coronaProgram->getUniform("unDT"), (f32)dt);
    glUniformMatrix4fv(m_coronaProgram->getUniform("unWVP"), 1, GL_FALSE, &VP[0][0]);

    // Bind VAO
    glBindVertexArray(m_cVao);

    glDepthMask(GL_FALSE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glDepthMask(GL_TRUE);

    glBindVertexArray(0);
    m_coronaProgram->unuse();
}

f32v3 hdrs(f32v3 v) {
    return f32v3(1.0f) - glm::exp(v * -3.0f);
}

void StarComponentRenderer::drawGlow(const StarComponent& sCmp,
                                     const f32m4& VP,
                                     const f64v3& relCamPos,
                                     float aspectRatio,
                                     const f32v3& viewDirW,
                                     const f32v3& viewRightW,
                                     const f32v3& colorMult /* = f32v3(1.0f) */ ) {

    if (sCmp.visibility == 0.0f) return;
  
    // Compute desired size based on distance and ratio of mass to Sol mass
    f64 s = calculateGlowSize(sCmp, relCamPos) * sCmp.visibility;
    // Don't render if its too small
    if (s <= 0.0) return;

    f32v2 dims(s, s * aspectRatio);
   
    m_glowProgram->use();

    f32 scale = glm::clamp((f32)((sCmp.temperature - MIN_TMP) / TMP_RANGE), 0.0f, 1.0f);

    // Upload uniforms
    f32v3 center(-relCamPos);
    glUniform3fv(m_glowProgram->getUniform("unCenter"), 1, &center[0]);
    glUniform3fv(m_glowProgram->getUniform("unColorMult"), 1, &colorMult[0]);
    glUniform1f(m_glowProgram->getUniform("unColorMapU"), scale);
    glUniform2fv(m_glowProgram->getUniform("unDims"), 1, &dims[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_glowColorMap);
    // For sparkles
    f32v3 vs = viewDirW - viewRightW;
    glUniform1f(m_glowProgram->getUniform("unNoiseZ"), (vs.x + vs.y - vs.z) * 0.125f);

    glUniformMatrix4fv(m_glowProgram->getUniform("unVP"), 1, GL_FALSE, &VP[0][0]);
    // Bind VAO
    glBindVertexArray(m_gVao);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(0);
    m_glowProgram->unuse();
}

void StarComponentRenderer::updateOcclusionQuery(StarComponent& sCmp,
                                                 const f32m4& VP,
                                                 const f64v3& relCamPos) {
    checkLazyLoad();
    if (m_occlusionProgram == nullptr) {
        m_occlusionProgram = vg::ShaderManager::createProgram(OCCLUSION_VERT_SRC, OCCLUSION_FRAG_SRC);
        glGenVertexArrays(1, &m_oVao);
        glBindVertexArray(m_oVao);
        vg::GpuMemory::bindBuffer(m_cVbo, vg::BufferTarget::ARRAY_BUFFER);
        vg::GpuMemory::bindBuffer(m_cIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
        m_occlusionProgram->enableVertexAttribArrays();
        glVertexAttribPointer(m_occlusionProgram->getAttribute("vPosition"), 2, GL_FLOAT, GL_FALSE, 0, 0);
        glBindVertexArray(0);
    }
    if (sCmp.occlusionQuery[0] == 0) {
        glGenQueries(2, sCmp.occlusionQuery);
    } else {
        int totalSamples = 0;
        int passedSamples = 0;
        glGetQueryObjectiv(sCmp.occlusionQuery[0], GL_QUERY_RESULT, &totalSamples);
        glGetQueryObjectiv(sCmp.occlusionQuery[1], GL_QUERY_RESULT, &passedSamples);
        if (passedSamples == 0) {
            sCmp.visibility = 0.0f;
        } else {
            sCmp.visibility = (f32)passedSamples / (f32)totalSamples;
        }
    }
    // Have to calculate on the CPU since we need 64 bit precision. Otherwise
    // we get the "phantom star" bug.
    f64v4 pos(-relCamPos, 1.0);
    f64v4 gl_Position = f64m4(VP) * pos;
    f64v3 centerScreenspace64(gl_Position.x / gl_Position.w,
                              gl_Position.y / gl_Position.w,
                              gl_Position.z / gl_Position.w);
    if (centerScreenspace64.z > 1.0) centerScreenspace64.z = 2.0;
    f32v3 centerScreenspace(centerScreenspace64);

    f64 s = calculateGlowSize(sCmp, relCamPos) / 128.0;
    s = glm::max(0.005, s); // make sure it never gets too small

    m_occlusionProgram->use();

    // Upload uniforms
    glUniform3fv(m_occlusionProgram->getUniform("unCenterScreenspace"), 1, &centerScreenspace[0]);
    glUniform1f(m_occlusionProgram->getUniform("unSize"), (f32)s);

    glBindVertexArray(m_oVao);

    glDepthMask(GL_FALSE);
    glBeginQuery(GL_SAMPLES_PASSED, sCmp.occlusionQuery[0]);
    glDisable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEndQuery(GL_SAMPLES_PASSED);
    glBeginQuery(GL_SAMPLES_PASSED, sCmp.occlusionQuery[1]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glEndQuery(GL_SAMPLES_PASSED);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    glBindVertexArray(0);

    m_occlusionProgram->unuse();
}

void StarComponentRenderer::dispose() {
    disposeShaders();
    disposeBuffers();
    if (m_tempColorMap.data) {
        vg::ImageIO().free(m_tempColorMap);
        m_tempColorMap.data = nullptr;
    }
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
    if (m_occlusionProgram) {
        vg::ShaderManager::destroyProgram(&m_occlusionProgram);
        // Also destroy VAO since they are related
        glDeleteVertexArrays(1, &m_oVao);
        m_oVao = 0;
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
    m_glowProgram->use();
    glUniform1i(m_glowProgram->getUniform("unColorMap"), 0);
    m_glowProgram->unuse();
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
    // Load all the bitmap data
    vio::Path path;
    m_textureResolver->resolvePath("Sky/Star/star_spectrum_1.png", path);
    m_tempColorMap = vg::ImageIO().load(path);
    if (!m_tempColorMap.data) {
        fprintf(stderr, "ERROR: Failed to load Sky/Star/star_spectrum_1.png\n");
    }
    m_textureResolver->resolvePath("Sky/Star/star_spectrum_2.png", path);
    vg::ScopedBitmapResource res2 = vg::ImageIO().load(path);
    if (!res2.data) {
        fprintf(stderr, "ERROR: Failed to load Sky/Star/star_spectrum_2.png\n");
    }
    m_textureResolver->resolvePath("Sky/Star/star_spectrum_3.png", path);
    vg::ScopedBitmapResource res3 = vg::ImageIO().load(path);
    if (!res3.data) {
        fprintf(stderr, "ERROR: Failed to load Sky/Star/star_spectrum_3.png\n");
    }

    // Error check dimensions
    if ((m_tempColorMap.width != res2.width) || (res2.width != res3.width) ||
        (m_tempColorMap.height != res2.height) || (res2.height != res3.height)) {
        pError("star_spectrum images should all be the same dimensions!");
    }

    // Create the texture array
    if (m_glowColorMap == 0) glGenTextures(1, &m_glowColorMap);
    glBindTexture(GL_TEXTURE_2D, m_glowColorMap);

    // Set up storage
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_tempColorMap.width, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
     
    // Upload the data to VRAM
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_tempColorMap.width, m_tempColorMap.height, GL_RGBA, GL_UNSIGNED_BYTE, m_tempColorMap.data);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 1, res2.width, res2.height, GL_RGBA, GL_UNSIGNED_BYTE, res2.data);
    // Copy res3 twice so we get PO2 texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 2, res3.width, res3.height, GL_RGBA, GL_UNSIGNED_BYTE, res3.data);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 3, res3.width, res3.height, GL_RGBA, GL_UNSIGNED_BYTE, res3.data);
  
    // Set up tex parameters
    vg::SamplerState::LINEAR_CLAMP.set(GL_TEXTURE_2D);
    // No mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);

    // Check if we had any errors
    checkGlError("StarComponentRenderer::loadTempColorMap()");
}

void StarComponentRenderer::loadGlowTextures() {
    // TODO(Ben): remove this
    /*vio::Path path;
    m_textureResolver->resolvePath("Sky/Star/star_glow_overlay.png", path);
    vg::ScopedBitmapResource rs2 = vg::ImageIO().load(path);
    if (!m_tempColorMap.data) {
    fprintf(stderr, "ERROR: Failed to load Sky/Star/star_glow_overlay.png\n");
    } else {
    m_glowTexture = vg::GpuMemory::uploadTexture(&rs2);
    }*/
}

f64 StarComponentRenderer::calculateGlowSize(const StarComponent& sCmp, const f64v3& relCamPos) {
    static const f64 DSUN = 1392684.0;
    static const f64 TSUN = 5778.0;

    // Georg's magic formula
    f64 d = glm::length(relCamPos); // Distance
    f64 D = sCmp.radius * 2.0 * DSUN;
    f64 L = (D * D) * pow(sCmp.temperature / TSUN, 4.0); // Luminosity
    return 0.016 * pow(L, 0.25) / pow(d, 0.5); // Size
}

f32v3 StarComponentRenderer::calculateStarColor(const StarComponent& sCmp) {
    // Calculate temperature color
    f32v3 tColor;
    f32 scale = m_tempColorMap.width * (sCmp.temperature - MIN_TMP) / TMP_RANGE;
    scale = glm::clamp(scale, 0.0f, (f32)m_tempColorMap.width);
    int rScale = (int)(scale + 0.5f);
    int iScale = (int)scale;

    if (rScale >= m_tempColorMap.width) rScale = m_tempColorMap.width - 1;

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
    
    return tColor;
}

f32v3 StarComponentRenderer::getColor(int index) {
    const ui8v4& bytes = m_tempColorMap.bytesUI8v4[index];
    return f32v3(bytes.r / 255.0f, bytes.g / 255.0f, bytes.b / 255.0f);
}

f32v3 StarComponentRenderer::getTempColorShift(const StarComponent& sCmp) {
    return f32v3(sCmp.temperature * (0.0534 / 255.0) - (43.0 / 255.0),
                 sCmp.temperature * (0.0628 / 255.0) - (77.0 / 255.0),
                 sCmp.temperature * (0.0735 / 255.0) - (115.0 / 255.0));

}

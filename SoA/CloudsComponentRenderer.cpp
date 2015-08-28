#include "stdafx.h"
#include "CloudsComponentRenderer.h"

#include "SpaceSystem.h"
#include "RenderUtils.h"
#include "soaUtils.h"
#include "ShaderLoader.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/MeshGenerators.h>
#include <Vorb/graphics/RasterizerState.h>
#include <Vorb/graphics/ShaderManager.h>

CloudsComponentRenderer::~CloudsComponentRenderer() {
    dispose();
}

void CloudsComponentRenderer::initGL() {
    if (!m_program.isCreated()) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/CloudsShading/Clouds.vert",
                                                        "Shaders/CloudsShading/Clouds.frag");
    }
    if (!m_icoVbo) buildMesh();
}

void CloudsComponentRenderer::draw(const CloudsComponent& cCmp,
                                   const f32m4& VP,
                                   const f32v3& relCamPos,
                                   const f32v3& lightDir,
                                   const f32 zCoef,
                                   const SpaceLightComponent* spComponent,
                                   const AxisRotationComponent& arComponent,
                                   const AtmosphereComponent& aCmp) {
    m_program.use();

    f64q invOrientation = vmath::inverse(arComponent.currentOrientation);
    const f32v3 rotpos(invOrientation * f64v3(relCamPos));
    const f32v3 rotLightDir = f32v3(invOrientation * f64v3(lightDir));
    // Convert f64q to f32q
    f32q orientationF32;
    orientationF32.x = (f32)arComponent.currentOrientation.x;
    orientationF32.y = (f32)arComponent.currentOrientation.y;
    orientationF32.z = (f32)arComponent.currentOrientation.z;
    orientationF32.w = (f32)arComponent.currentOrientation.w;
    // Convert to matrix
    f32m4 rotationMatrix = vmath::toMat4(orientationF32);

    // Create WVP matrix
    f32m4 WVP(1.0);
    setMatrixScale(WVP, f32v3(cCmp.height + cCmp.planetRadius));
    setMatrixTranslation(WVP, -relCamPos);
    WVP = VP * WVP * rotationMatrix;
    
    // Set uniforms
    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, GL_FALSE, &WVP[0][0]);
    static f32 time = 0.0;
    time += 0.001f;
    glUniform1f(m_program.getUniform("unTime"), time);
    glUniform3fv(m_program.getUniform("unColor"), 1, &cCmp.color[0]);
    glUniform3fv(m_program.getUniform("unNoiseScale"), 1, &cCmp.scale[0]);
    glUniform1f(m_program.getUniform("unDensity"), cCmp.density);
    glUniform1f(m_program.getUniform("unCloudsRadius"), cCmp.planetRadius + cCmp.height);
    // For logarithmic Z buffer
    glUniform1f(m_program.getUniform("unZCoef"), zCoef);

    // Scattering
    f32 camHeight = vmath::length(rotpos);
    f32 camHeight2 = camHeight * camHeight;
    glUniform3fv(m_program.getUniform("unLightDirWorld"), 1, &rotLightDir[0]);

    glUniform3fv(m_program.getUniform("unCameraPos"), 1, &rotpos[0]);
    glUniform3fv(m_program.getUniform("unInvWavelength"), 1, &aCmp.invWavelength4[0]);
    glUniform1f(m_program.getUniform("unCameraHeight2"), camHeight * camHeight);
    glUniform1f(m_program.getUniform("unOuterRadius"), aCmp.radius);
    glUniform1f(m_program.getUniform("unOuterRadius2"), aCmp.radius * aCmp.radius);
    glUniform1f(m_program.getUniform("unInnerRadius"), aCmp.planetRadius);
    glUniform1f(m_program.getUniform("unKrESun"), aCmp.kr * aCmp.esun);
    glUniform1f(m_program.getUniform("unKmESun"), aCmp.km * aCmp.esun);
    glUniform1f(m_program.getUniform("unKr4PI"), (f32)(aCmp.kr * M_4_PI));
    glUniform1f(m_program.getUniform("unKm4PI"), (f32)(aCmp.km * M_4_PI));
    float scale = 1.0f / (aCmp.radius - aCmp.planetRadius);
    glUniform1f(m_program.getUniform("unScale"), scale);
    glUniform1f(m_program.getUniform("unScaleDepth"), aCmp.scaleDepth);
    glUniform1f(m_program.getUniform("unScaleOverScaleDepth"), scale / aCmp.scaleDepth);
    glUniform1i(m_program.getUniform("unNumSamples"), 3);
    glUniform1f(m_program.getUniform("unNumSamplesF"), 3.0f);
    glUniform1f(m_program.getUniform("unG"), aCmp.g);
    glUniform1f(m_program.getUniform("unG2"), aCmp.g * aCmp.g);

    // Bind VAO
    glBindVertexArray(m_vao);

    // Render
    glDepthMask(GL_FALSE);

    if (camHeight > cCmp.planetRadius + cCmp.height) vg::RasterizerState::CULL_CLOCKWISE.set();
    else vg::RasterizerState::CULL_COUNTER_CLOCKWISE.set();
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
    glDepthMask(GL_TRUE);
    vg::RasterizerState::CULL_CLOCKWISE.set();

    glBindVertexArray(0);

    m_program.unuse();
}

void CloudsComponentRenderer::dispose() {
    if (m_program.isCreated()) m_program.dispose();
    if (m_icoVbo) {
        vg::GpuMemory::freeBuffer(m_icoVbo);
        m_icoVbo = 0;
    }
    if (m_icoIbo) {
        vg::GpuMemory::freeBuffer(m_icoIbo);
        m_icoIbo = 0;
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

void CloudsComponentRenderer::buildMesh() {
    std::vector<ui32> indices;
    std::vector<f32v3> positions;

    vmesh::generateIcosphereMesh(3, indices, positions);
    m_numIndices = indices.size();

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    vg::GpuMemory::createBuffer(m_icoVbo);
    vg::GpuMemory::createBuffer(m_icoIbo);

    vg::GpuMemory::bindBuffer(m_icoVbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_icoVbo, vg::BufferTarget::ARRAY_BUFFER, positions.size() * sizeof(f32v3),
        positions.data(), vg::BufferUsageHint::STATIC_DRAW);

    vg::GpuMemory::bindBuffer(m_icoIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_icoIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ui32),
        indices.data(), vg::BufferUsageHint::STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    m_program.enableVertexAttribArrays();

    glBindVertexArray(0);
}
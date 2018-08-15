#include "stdafx.h"
#include "OrbitComponentRenderer.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/utils.h>

#include "Constants.h"
#include "RenderUtils.h"
#include "SpaceSystemComponents.h"
#include "OrbitComponentUpdater.h"

void OrbitComponentRenderer::drawPath(OrbitComponent& cmp, vg::GLProgram& colorProgram, const f32m4& wvp, NamePositionComponent* npComponent VORB_UNUSED, const f64v3& camPos, float blendFactor, NamePositionComponent* parentNpComponent /*= nullptr*/) {

    // Lazily generate mesh
    if (cmp.vbo == 0) generateOrbitEllipse(cmp, colorProgram);
    if (cmp.numVerts == 0) return;
    
    f32m4 w(1.0f);
    if (parentNpComponent) {
        setMatrixTranslation(w, parentNpComponent->position - camPos);
    } else {
        setMatrixTranslation(w, -camPos);
    }
    
    f32m4 pathMatrix = wvp * w;

    f32v4 newColor = lerp(cmp.pathColor[0], cmp.pathColor[1], blendFactor);
    if (newColor.a <= 0.0f) return;
    glUniform4f(colorProgram.getUniform("unColor"), newColor.r, newColor.g, newColor.b, newColor.a);

    glUniformMatrix4fv(colorProgram.getUniform("unWVP"), 1, GL_FALSE, &pathMatrix[0][0]);

    float currentAngle = cmp.currentMeanAnomaly - (f32)cmp.startMeanAnomaly;
    glUniform1f(colorProgram.getUniform("currentAngle"), currentAngle / (float)M_2_PI);

    // Draw the ellipse
    glDepthMask(false);
    glBindVertexArray(cmp.vao);
    glDrawArrays(GL_LINE_STRIP, 0, cmp.numVerts);
    glBindVertexArray(0);
    glDepthMask(true);
}

void OrbitComponentRenderer::generateOrbitEllipse(OrbitComponent& cmp, vg::GLProgram& colorProgram) {

    if (cmp.verts.empty()) return;

    glGenVertexArrays(1, &cmp.vao);
    glBindVertexArray(cmp.vao);

    colorProgram.enableVertexAttribArrays();

    // Upload the buffer data
    vg::GpuMemory::createBuffer(cmp.vbo);
    vg::GpuMemory::bindBuffer(cmp.vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(cmp.vbo,
                                    vg::BufferTarget::ARRAY_BUFFER,
                                    cmp.verts.size() * sizeof(OrbitComponent::Vertex),
                                    cmp.verts.data(),
                                    vg::BufferUsageHint::STATIC_DRAW);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OrbitComponent::Vertex), 0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(OrbitComponent::Vertex), (const void*)offsetof(OrbitComponent::Vertex, angle));
    glBindVertexArray(0);
    cmp.numVerts = cmp.verts.size();
    std::vector<OrbitComponent::Vertex>().swap(cmp.verts);
}
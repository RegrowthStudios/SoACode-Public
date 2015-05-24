#include "stdafx.h"
#include "OrbitComponentRenderer.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/utils.h>

#include <glm/gtx/quaternion.hpp>

#include "Constants.h"
#include "RenderUtils.h"
#include "SpaceSystemComponents.h"

void OrbitComponentRenderer::drawPath(OrbitComponent& cmp, vg::GLProgram* colorProgram, const f32m4& wvp, NamePositionComponent* npComponent, const f64v3& camPos, float blendFactor, NamePositionComponent* parentNpComponent /*= nullptr*/) {
    
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
    
    // Blend hover color
    if (cmp.pathColor[0].r == 0.0f && cmp.pathColor[0].g == 0.0f && cmp.pathColor[0].b == 0.0f) {
        if (blendFactor <= 0.0f) return;
        glUniform4f(colorProgram->getUniform("unColor"), cmp.pathColor[1].r, cmp.pathColor[1].g, cmp.pathColor[1].b, 0.7f);// blendFactor);
    } else {
        f32v3 color = lerp(cmp.pathColor[0], cmp.pathColor[1], blendFactor);
        glUniform4f(colorProgram->getUniform("unColor"), color.r, color.g, color.b, blendFactor / 2.0 + 0.5);
    }
    glUniformMatrix4fv(colorProgram->getUniform("unWVP"), 1, GL_FALSE, &pathMatrix[0][0]);

    // Draw the ellipse
    glDepthMask(false);
    glBindVertexArray(cmp.vao);
    glDrawArrays(GL_LINE_STRIP, 0, cmp.numVerts);
    glBindVertexArray(0);
    glDepthMask(true);
}

void OrbitComponentRenderer::generateOrbitEllipse(OrbitComponent& cmp, vg::GLProgram* colorProgram) {

    if (cmp.verts.empty()) return;

    glGenVertexArrays(1, &cmp.vao);
    glBindVertexArray(cmp.vao);

    colorProgram->enableVertexAttribArrays();

    // Upload the buffer data
    vg::GpuMemory::createBuffer(cmp.vbo);
    vg::GpuMemory::bindBuffer(cmp.vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(cmp.vbo,
                                    vg::BufferTarget::ARRAY_BUFFER,
                                    cmp.verts.size() * sizeof(f32v3),
                                    cmp.verts.data(),
                                    vg::BufferUsageHint::STATIC_DRAW);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OrbitComponent::Vertex), 0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(OrbitComponent::Vertex), (const void*)offsetof(OrbitComponent::Vertex, opaqueness));
    glBindVertexArray(0);
    cmp.numVerts = cmp.verts.size();
    std::vector<OrbitComponent::Vertex>().swap(cmp.verts);
}
#include "stdafx.h"
#include "OrbitComponentRenderer.h"

#include <Vorb/graphics/GLProgram.h>
#include <glm/gtx/quaternion.hpp>
#include <Vorb/graphics/GpuMemory.h>

#include "Constants.h"
#include "RenderUtils.h"
#include "SpaceSystemComponents.h"

void OrbitComponentRenderer::drawPath(OrbitComponent& cmp, vg::GLProgram* colorProgram, const f32m4& wvp, NamePositionComponent* npComponent, const f64v3& camPos, float alpha, NamePositionComponent* parentNpComponent /*= nullptr*/) {
    
    // Lazily generate mesh
    if (cmp.vbo == 0) generateOrbitEllipse(cmp, colorProgram);
    if (cmp.numVerts == 0) return;
    
    f32v4 color(f32v4(cmp.pathColor) / 255.0f);
    f32m4 w(1.0f);
    if (parentNpComponent) {
        setMatrixTranslation(w, parentNpComponent->position - camPos);
    } else {
        setMatrixTranslation(w, -camPos);
    }
    f32m4 pathMatrix = wvp * w;
    glUniform4f(colorProgram->getUniform("unColor"), color.r, color.g, color.b, alpha);
    glUniformMatrix4fv(colorProgram->getUniform("unWVP"), 1, GL_FALSE, &pathMatrix[0][0]);

    // Draw the ellipse
    glBindVertexArray(cmp.vao);
    glDrawArrays(GL_LINE_STRIP, 0, cmp.numVerts);
    glBindVertexArray(0);
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32v3), 0);
    glBindVertexArray(0);
    cmp.numVerts = cmp.verts.size();
    std::vector<f32v3>().swap(cmp.verts);
}
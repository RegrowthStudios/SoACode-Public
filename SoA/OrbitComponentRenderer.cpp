#include "stdafx.h"
#include "OrbitComponentRenderer.h"

#include <Vorb/graphics/GLProgram.h>
#include <glm/gtx/quaternion.hpp>
#include <Vorb/graphics/GpuMemory.h>

#include "Constants.h"
#include "RenderUtils.h"
#include "SpaceSystemComponents.h"

#define DEGREES 360
#define VERTS_PER_DEGREE 8

void OrbitComponentRenderer::drawPath(OrbitComponent& cmp, vg::GLProgram* colorProgram, const f32m4& wvp, NamePositionComponent* npComponent, const f64v3& camPos, float alpha, NamePositionComponent* parentNpComponent /*= nullptr*/) {
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

    // Lazily generate mesh
    if (cmp.vbo == 0) generateOrbitEllipse(cmp);

    // Draw the ellipse
    vg::GpuMemory::bindBuffer(cmp.vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32v3), 0);
    glDrawArrays(GL_LINE_STRIP, 0, DEGREES * VERTS_PER_DEGREE + 1);
}

void OrbitComponentRenderer::generateOrbitEllipse(OrbitComponent& cmp) {
#define DEGTORAD (M_PI / 180.0)

    // Need to offset the ellipse mesh based on eccentricity
    f64 xOffset = cmp.a - cmp.r1;
    std::vector<f32v3> verts;
    verts.reserve(DEGREES * VERTS_PER_DEGREE + 1);

    // Generate all the verts
    for (int i = 0; i < DEGREES * VERTS_PER_DEGREE; i++) {
        f64 rad = ((double)i / VERTS_PER_DEGREE) * DEGTORAD;
        verts.emplace_back(cos(rad)*cmp.a - xOffset,
                           0.0,
                           sin(rad)*cmp.b);
    }
    // First vertex is duplicated
    verts.push_back(verts.front());

    // Upload the buffer data
    vg::GpuMemory::createBuffer(cmp.vbo);
    vg::GpuMemory::bindBuffer(cmp.vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(cmp.vbo,
                                    vg::BufferTarget::ARRAY_BUFFER,
                                    verts.size() * sizeof(f32v3),
                                    verts.data(),
                                    vg::BufferUsageHint::STATIC_DRAW);
}
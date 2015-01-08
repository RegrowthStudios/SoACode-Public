#include "stdafx.h"
#include "OrbitComponent.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <Vorb/GLProgram.h>

#include "Constants.h"
#include "NamePositionComponent.h"
#include "RenderUtils.h"

#define DEGREES 360
#define VERTS_PER_DEGREE 8

void OrbitComponent::destroy() {
    if (m_vbo != 0) {
        vg::GpuMemory::freeBuffer(m_vbo);
    }
    if (m_pvbo != 0) {
        vg::GpuMemory::freeBuffer(m_pvbo);
    }
}

void OrbitComponent::drawPath(vg::GLProgram* colorProgram, const f32m4& wvp, NamePositionComponent* npComponent,
                              const f64v3& camPos, float alpha, NamePositionComponent* parentNpComponent /* = nullptr */) {

    f32v4 color(f32v4(pathColor) / 255.0f);
    f32m4 transMatrix(1.0f);
    if (parentNpComponent) { 
        setMatrixTranslation(transMatrix, parentNpComponent->position - camPos);
    } else {
        setMatrixTranslation(transMatrix, -camPos);
    }
    f32m4 pathMatrix = wvp * transMatrix * glm::mat4(glm::toMat4(orientation));
    glUniform4f(colorProgram->getUniform("unColor"), color.r, color.g, color.b, alpha);
    glUniformMatrix4fv(colorProgram->getUniform("unWVP"), 1, GL_FALSE, &pathMatrix[0][0]);

    // Lazily generate mesh
    if (m_vbo == 0) generateOrbitEllipse();

    // Draw the ellipse
    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32v3), 0);
    glDrawArrays(GL_LINE_STRIP, 0, DEGREES * VERTS_PER_DEGREE + 1);

    glPointSize(10.0);
    // Point position
    f32v3 pos(npComponent->position - camPos);
    vg::GpuMemory::bindBuffer(m_pvbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_pvbo,
                                    vg::BufferTarget::ARRAY_BUFFER,
                                    1 * sizeof(f32v3),
                                    &pos,
                                    vg::BufferUsageHint::STREAM_DRAW);


    // Point already has orientation encoded
    glUniformMatrix4fv(colorProgram->getUniform("unWVP"), 1, GL_FALSE, &wvp[0][0]);
    // Draw the point
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32v3), 0);
   // glDrawArrays(GL_POINTS, 0, 1);
}

void OrbitComponent::generateOrbitEllipse() {
    #define DEGTORAD (M_PI / 180.0)

    // Need to offset the ellipse mesh based on eccentricity
    f64 xOffset = semiMajor - r1;
    std::vector<f32v3> verts;
    verts.reserve(DEGREES * VERTS_PER_DEGREE + 1);

    // Generate all the verts
    for (int i = 0; i < DEGREES * VERTS_PER_DEGREE; i++) {
        f64 rad = ((double)i / VERTS_PER_DEGREE) * DEGTORAD;
        verts.emplace_back(cos(rad)*semiMajor - xOffset,
                           0.0,
                           sin(rad)*semiMinor);
    }
    // First vertex is duplicated
    verts.push_back(verts.front());

    // Upload the buffer data
    vg::GpuMemory::createBuffer(m_vbo);
    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_vbo,
                                    vg::BufferTarget::ARRAY_BUFFER,
                                    verts.size() * sizeof(f32v3),
                                    verts.data(),
                                    vg::BufferUsageHint::STATIC_DRAW);

    vg::GpuMemory::createBuffer(m_pvbo);
}

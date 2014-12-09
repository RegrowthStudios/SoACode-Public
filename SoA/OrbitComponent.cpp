#include "stdafx.h"
#include "OrbitComponent.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Constants.h"
#include "GLProgram.h"
#include "NamePositionComponent.h"

void OrbitComponent::update(f64 time, NamePositionComponent* npComponent,
            NamePositionComponent* parentNpComponent = nullptr) {

    /// Calculates position as a function of time
    /// http://en.wikipedia.org/wiki/Kepler%27s_laws_of_planetary_motion#Position_as_a_function_of_time
    f64 semiMajor3 = semiMajor * semiMajor * semiMajor;
    f64 meanAnomaly = sqrt((M_G * totalMass) / semiMajor3) * time;

    // Solve Kepler's equation to compute eccentric anomaly 
    // using Newton's method
    // http://www.jgiesen.de/kepler/kepler.html
#define ITERATIONS 5
    f64 eccentricAnomaly, F;
    eccentricAnomaly = meanAnomaly;
    F = eccentricAnomaly - eccentricity * sin(meanAnomaly) - meanAnomaly;
    for (int i = 0; i < ITERATIONS; i++) {
        eccentricAnomaly = eccentricAnomaly -
            F / (1.0 - eccentricity * cos(eccentricAnomaly));
        F = eccentricAnomaly -
            eccentricity * sin(eccentricAnomaly) - meanAnomaly;
    }

    // Finally calculate position
    f64v3 position;
    position.x = semiMajor * (cos(eccentricAnomaly) - eccentricity);
    position.y = 0.0;
    position.z = semiMajor * sqrt(1.0 - eccentricity * eccentricity) *
        sin(eccentricAnomaly);
    if (parentNpComponent) {
        npComponent->position = orientation * position + parentNpComponent->position;
    } else {
        npComponent->position = orientation * position;
    }
}

void OrbitComponent::destroy() {
    if (m_vbo != 0) {
        vg::GpuMemory::freeBuffer(m_vbo);
    }
    if (m_pvbo != 0) {
        vg::GpuMemory::freeBuffer(m_pvbo);
    }
}

void OrbitComponent::drawPath(vg::GLProgram* colorProgram, const f32m4& wvp, NamePositionComponent* npComponent) {

    f32v4 color(f32v4(pathColor) / 255.0f);
    f32m4 matrix = wvp * glm::mat4(glm::toMat4(orientation));
    glUniform4f(colorProgram->getUniform("unColor"), color.r, color.g, color.b, color.a);
    glUniformMatrix4fv(colorProgram->getUniform("unWVP"), 1, GL_FALSE, &matrix[0][0]);

    if (m_vbo == 0) generateOrbitEllipse();
    // Draw the ellipse
    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32v3), 0);
    glDrawArrays(GL_LINE_STRIP, 0, DEGREES + 1);

    glPointSize(10.0);
    // Point position
    f32v3 pos(npComponent->position);
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
    glDrawArrays(GL_POINTS, 0, 1);
}

void OrbitComponent::generateOrbitEllipse() {
    #define DEGTORAD (M_PI / 180.0)


    f64 xOffset = semiMajor - r1;
    std::vector<f32v3> verts;
    verts.reserve(DEGREES + 1);

    for (int i = 0; i < DEGREES; i++) {
        f64 rad = i * DEGTORAD;
        verts.emplace_back(cos(rad)*semiMajor - xOffset,
                           0.0,
                           sin(rad)*semiMinor);
    }
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

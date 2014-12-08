///
/// OrbitComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 3 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines a component for handling ellipsoid orbit
///

#pragma once

#ifndef OrbitComponent_h__
#define OrbitComponent_h__

#include <SDL/SDL_stdinc.h>

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "GpuMemory.h"
#include "Constants.h"
#include "NamePositionComponent.h"

class OrbitComponent {
public:
    #define DEGREES 360

    ~OrbitComponent() {
        destroy();
    }

    void update(f64 time) {

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
        if (parent) {
            namePositionComponent->position = orientation * position + parent->position;
        } else {
            namePositionComponent->position = orientation * position;
        }
    }

    /// Frees resources
    void destroy() {
        if (m_vbo != 0) {
            vg::GpuMemory::freeBuffer(m_vbo);
        }
    }

    /// Draws the ellipse and a point for the body
    void drawPath(vg::GLProgram* colorProgram, const f32m4& wvp) {

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

        glPointSize(20.0);
        // Point already has orientation encoded
        glUniformMatrix4fv(colorProgram->getUniform("unWVP"), 1, GL_FALSE, &wvp[0][0]);
        // Draw the point
        vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ARRAY_BUFFER);
        f32v3 pos(namePositionComponent->position);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32v3), &pos);
        glDrawArrays(GL_POINTS, 0, 1);
    }

    /// Gets the vertex buffer ID for ellipse
    const VGBuffer& getVbo() const { return m_vbo; }

    f64 semiMajor = 0.0;
    f64 semiMinor = 0.0;
    f64 orbitalPeriod = 0.0;
    f64 currentOrbit = 0.0;
    f64 totalMass = 0.0; ///< Mass of this body + parent
    f64 eccentricity = 0.0;
    f64 r1 = 0.0;
    f64q orientation = f64q(0.0, 0.0, 0.0, 0.0);
    ui8v4 pathColor = ui8v4(255);
    NamePositionComponent* namePositionComponent = nullptr;
    NamePositionComponent* parent = nullptr;

private:
    /// Creates the ellipsoid mesh
    void generateOrbitEllipse() {
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
    }

    VGBuffer m_vbo = 0; ///< vbo for the ellipse
};

#endif // OrbitComponent_h__
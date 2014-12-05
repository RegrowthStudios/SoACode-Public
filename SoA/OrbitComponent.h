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

#include "gtypes.h"
#include "GpuMemory.h"

class OrbitComponent {
public:
    ~OrbitComponent() {
        destroy();
    }

    /// Frees resources
    void destroy() {
        if (m_vbo != 0) {
            vg::GpuMemory::freeBuffer(m_vbo);
        }
    }

    /// Creates the ellipsoid mesh
    void generateOrbitEllipse() {

        #define DEGREES 360
        #define DEGTORAD (M_PI / 180.0)

        std::vector<f32v3> verts(DEGREES);

        for (int i = 0; i < DEGREES; i++) {
            float rad = i * DEGTORAD;
            verts.emplace_back(cos(rad)*semiMajor,
                               0.0,
                               sin(rad)*semiMinor);
        }

        // Upload the buffer data
        if (m_vbo == 0) vg::GpuMemory::createBuffer(m_vbo);
        vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
        vg::GpuMemory::uploadBufferData(m_vbo,
                                        vg::BufferTarget::ARRAY_BUFFER,
                                        verts.size() * sizeof(f32v3),
                                        verts.data(),
                                        vg::BufferUsageHint::STATIC_DRAW);
    }

    /// Gets the vertex buffer ID for ellipse
    const VGBuffer& getVbo() const { return m_vbo; }

    f64 semiMajor = 0.0;
    f64 semiMinor = 0.0;
    f64 orbitalPeriod = 0.0;
    f64 currentOrbit = 0.0;

private:
    VGBuffer m_vbo = 0; ///< vbo for the ellipse
};

#endif // OrbitComponent_h__
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

#include "GpuMemory.h"
#include "Entity.h"

class NamePositionComponent;

class OrbitComponent {
public:
    #define DEGREES 360

    ~OrbitComponent() {
        destroy();
    }

    void update(f64 time, NamePositionComponent* npComponent,
                NamePositionComponent* parentNpComponent = nullptr);

    /// Frees resources
    void destroy();

    /// Draws the ellipse and a point for the body
    void drawPath(vg::GLProgram* colorProgram, const f32m4& wvp, NamePositionComponent* npComponent);

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
    vcore::ComponentID parentID = 0;

private:
    /// Creates the ellipsoid mesh
    void generateOrbitEllipse();

    VGBuffer m_vbo = 0; ///< vbo for the ellipse
    VGBuffer m_pvbo = 0; ///< vbo for the imposter
};

#endif // OrbitComponent_h__
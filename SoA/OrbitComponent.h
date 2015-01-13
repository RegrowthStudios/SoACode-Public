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
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/ecs/Entity.h>
#include <Vorb/graphics/gtypes.h>

class NamePositionComponent;

class OrbitComponent {
public:

    f64 semiMajor = 0.0; ///< Semi-major of the ellipse
    f64 semiMinor = 0.0; ///< Semi-minor of the ellipse
    f64 orbitalPeriod = 0.0; ///< Period in seconds of a full orbit
    f64 totalMass = 0.0; ///< Mass of this body + parent
    f64 eccentricity = 0.0; ///< Shape of orbit, 0-1
    f64 r1 = 0.0; ///< Closest distance to focal point
    f64q orientation = f64q(0.0, 0.0, 0.0, 0.0); ///< Orientation of the orbit path
    ui8v4 pathColor = ui8v4(255); ///< Color of the path
    vcore::ComponentID parentNpId = 0; ///< Component ID of parent NamePosition component
    VGBuffer vbo = 0; ///< vbo for the ellipse
};

#endif // OrbitComponent_h__
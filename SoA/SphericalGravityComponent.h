///
/// SphericalGravityComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 5 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component for doing gravity calculations with
/// spherical masses.
///

#pragma once

#ifndef SphericalGravityComponent_h__
#define SphericalGravityComponent_h__

#include "stdafx.h"

class SphericalGravityComponent {
public:
    void init(f64 Radius, f64 Mass) {
        radius = Radius;
        mass = Mass;
    }

    f64 radius = 0.0; ///< Radius in KM
    f64 mass = 0.0; ///< Mass in KG
};

#endif // SphericalGravityComponent_h__
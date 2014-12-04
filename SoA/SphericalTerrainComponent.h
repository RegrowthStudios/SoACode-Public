///
/// SphericalTerrainComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 3 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines the component for creating spherical terrain
/// for planets and stuff.
///

#pragma once

#ifndef SphericalTerrainComponent_h__
#define SphericalTerrainComponent_h__

#include "stdafx.h"

class SphericalTerrainComponent {
public:
    void init(f64 Radius) {
        radius = Radius;
    }

    f64 radius = 0.0;
};

#endif // SphericalTerrainComponent_h__
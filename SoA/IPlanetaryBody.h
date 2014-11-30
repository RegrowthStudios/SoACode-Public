///
/// IPlanetaryBody.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// A spherical body that has its own rotation about an axis
///

#pragma once

#ifndef IPlanetaryBody_h__
#define IPlanetaryBody_h__

#include "IOrbitalBody.h"

class IPlanetaryBody : public IOrbitalBody {
public:
    IPlanetaryBody();
    virtual ~IPlanetaryBody();

    /// Updates the properties of the orbital body
    /// @time: Time in sec since the beginning of this session
    virtual void update(f64 time);

    /// Draws the Orbital Body
    virtual void draw() = 0;
protected:
    f64q poleAxis_; ///< Axis of rotation
    f64 rotationalSpeed_MS_ = 0.0; ///< Rotational speed about _poleAxis in radians
    f64 currentRotation_ = 0.0; ///< Current rotation about _poleAxis in radians
};

#endif // IPlanetaryBody_h__
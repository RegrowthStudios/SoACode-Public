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
    virtual void draw(const Camera* camera) override = 0;

    // Getters
    const f64& getRadius() const { return radius_; }
protected:
    f64 radius_ = 0.0;
    f64q axisOrientation_; ///< Axis of rotation
    f64q currentOrientation_; ///< Current orientation of the body taking in to acount axis and rotation
    f64 angularSpeed_RS_ = 0.0; ///< Rotational speed about _poleAxis in radians per second
    f64 currentRotation_ = 0.0; ///< Current rotation about _poleAxis in radians
};

#endif // IPlanetaryBody_h__
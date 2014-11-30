///
/// OrbitalBody.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines an object in space that orbits something
///

#pragma once

#ifndef OrbitalBody_h__
#define OrbitalBody_h__

#pragma once

/// One space unit in meters
#define METERS_PER_SU 1024

// TODO(Ben) ECS
class IOrbitalBody {
public:
    IOrbitalBody();
    virtual ~IOrbitalBody();

    /// Updates the properties of the orbital body
    /// @time: Time in sec since the beginning of this session
    virtual void update(f64 time);

    /// Draws the Orbital Body
    virtual void draw() = 0;

protected:
    IOrbitalBody* _parentBody = nullptr; ///< The parent object. If it is nullptr, then the parent is space itself

    // TODO(Ben): ECS this bitch
    f64v3 centerPosition_SU_ = f64v3(0.0); ///< Center position of object relative to _parentBody
    f64v3 orbitCenter_SU_ = f64v3(0.0); ///< Center position of orbit relative to _parentBody
    f64 orbitSpeed_SUS_ = 0.0; ///< Speed in SU per second around the orbit path
    f64 orbitSemiMajor_SU_ = 0.0;
    f64 orbitSemiMinor_SU_ = 0.0;
    f64 initialOrbitRadians_ = 0.0;
    f64 currentOrbitRadians_ = 0.0;
    f64q orbitRotation_; ///< Describes the orientation of the orbit relative to space

    // TODO(Ben): ECS this bitch
    f64 mass_ = 1.0;

};

#endif // OrbitalBody_h__
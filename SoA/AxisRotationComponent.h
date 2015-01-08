///
/// AxisRotationComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 3 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines a component for axis rotation
///

#pragma once

#ifndef AxisRotationComponent_h__
#define AxisRotationComponent_h__

#include "stdafx.h"

class AxisRotationComponent {
public:
    /// Initializes the component
    void init(f64 AngularSpeed_RS, f64 CurrentRotation, f64q AxisOrientation);
    /// Updates the component
    /// @param time: Time in seconds
    void update(double time);

    f64q axisOrientation; ///< Axis of rotation
    f64q currentOrientation; ///< Current orientation with axis and rotation
    f64q invCurrentOrientation; ///< Inverse of currentOrientation
    f64 angularSpeed_RS = 0.0; ///< Rotational speed about axis in radians per second
    f64 currentRotation = 0.0; ///< Current rotation about axis in radians
};

#endif // AxisRotationComponent_h__
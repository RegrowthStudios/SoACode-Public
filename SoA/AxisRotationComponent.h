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
    void init(f64 AngularSpeed_RS, f64 CurrentRotation, f64q AxisOrientation) {
        angularSpeed_RS = AngularSpeed_RS;
        currentRotation = CurrentRotation;
        axisOrientation = AxisOrientation;
    }
    /// Updates the component
    /// @param time: Time in seconds
    inline void update(double time) {
        // Calculate rotation
        currentRotation = angularSpeed_RS * time;

        // Calculate the axis rotation quat
        f64v3 eulerAngles(0, currentRotation, 0);
        f64q rotationQuat = f64q(eulerAngles);

        // Calculate total orientation
        currentOrientation = axisOrientation * rotationQuat;
    }

    f64q axisOrientation; ///< Axis of rotation
    f64q currentOrientation; ///< Current orientation with axis and rotation
    f64 angularSpeed_RS = 0.0; ///< Rotational speed about axis in radians per second
    f64 currentRotation = 0.0; ///< Current rotation about axis in radians
};

#endif // AxisRotationComponent_h__
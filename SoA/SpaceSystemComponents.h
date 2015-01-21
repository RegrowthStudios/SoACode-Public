///
/// SpaceSystemComponents.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 20 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component definitions for SpaceSystem
///

#pragma once

#ifndef SpaceSystemComponents_h__
#define SpaceSystemComponents_h__

struct AxisRotationComponent {
    f64q axisOrientation; ///< Axis of rotation
    f64q currentOrientation; ///< Current orientation with axis and rotation
    f64q invCurrentOrientation; ///< Inverse of currentOrientation
    f64 angularSpeed_RS = 0.0; ///< Rotational speed about axis in radians per second
    f64 currentRotation = 0.0; ///< Current rotation about axis in radians
};

#endif // SpaceSystemComponents_h__
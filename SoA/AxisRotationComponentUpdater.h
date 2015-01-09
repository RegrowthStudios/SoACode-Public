///
/// AxisRotationComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updates AxisRotationComponents
///

#pragma once

#ifndef AxisRotationComponentUpdater_h__
#define AxisRotationComponentUpdater_h__

class SpaceSystem;

class AxisRotationComponentUpdater {
public:
    /// Updates the components
    /// @param spaceSystem: The ECS space system
    /// @param time: Time in seconds
    void update(SpaceSystem* spaceSystem, f64 time);
};

#endif // AxisRotationComponentUpdater_h__
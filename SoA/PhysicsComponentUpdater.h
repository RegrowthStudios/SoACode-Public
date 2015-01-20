///
/// PhysicsComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updates physics components
///

#pragma once

#ifndef PhysicsComponentUpdater_h__
#define PhysicsComponentUpdater_h__

class GameSystem;
class SpaceSystem;

class PhysicsComponentUpdater {
public:
    /// Updates physics components
    /// @param gameSystem: Game ECS
    /// @param spaceSystem: Space ECS.
    void update(OUT GameSystem* gameSystem, const SpaceSystem* spaceSystem);
};

#endif // PhysicsComponentUpdater_h__

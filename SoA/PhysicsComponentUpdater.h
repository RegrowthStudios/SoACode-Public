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
struct PhysicsComponent;

#include <Vorb/ecs/ECS.h>

// TODO(Ben): Timestep
class PhysicsComponentUpdater {
public:
    /// Updates physics components
    /// @param gameSystem: Game ECS
    /// @param spaceSystem: Space ECS.
    void update(GameSystem* gameSystem, SpaceSystem* spaceSystem);

    /// Calculates the acceleration vector due to gravity
    /// @relativePosition: Relative position of object to attractor
    /// @mass: Mass of the attractor
    /// @return the acceleration vector
    static f64v3 calculateGravityAcceleration(f64v3 relativePosition, f64 mass);
private:
    void updateVoxelPhysics(GameSystem* gameSystem, SpaceSystem* spaceSystem,
                            PhysicsComponent& pyCmp, vcore::EntityID entity);
    void updateSpacePhysics(GameSystem* gameSystem, SpaceSystem* spaceSystem,
                            PhysicsComponent& pyCmp, vcore::EntityID entity);
    
};

#endif // PhysicsComponentUpdater_h__

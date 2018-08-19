///
/// PhysicsComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
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
struct VoxelPositionComponent;

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
                            PhysicsComponent& pyCmp, vecs::EntityID entity);
    void updateSpacePhysics(GameSystem* gameSystem, SpaceSystem* spaceSystem,
                            PhysicsComponent& pyCmp, vecs::EntityID entity);
    void transitionPosX(VoxelPositionComponent& vpCmp, PhysicsComponent& pyCmp, float voxelRadius);
    void transitionNegX(VoxelPositionComponent& vpCmp, PhysicsComponent& pyCmp, float voxelRadius);
    void transitionPosZ(VoxelPositionComponent& vpCmp, PhysicsComponent& pyCmp, float voxelRadius);
    void transitionNegZ(VoxelPositionComponent& vpCmp, PhysicsComponent& pyCmp, float voxelRadius);
    
};

#endif // PhysicsComponentUpdater_h__

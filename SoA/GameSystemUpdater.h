///
/// GameSystemUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updates a GameSystem ECS
///

#pragma once

#ifndef GameSystemUpdater_h__
#define GameSystemUpdater_h__

#include "PhysicsComponentUpdater.h"
#include "CollisionComponentUpdater.h"

class GameSystem;
class SpaceSystem;
class VoxelPositionComponent;

class GameSystemUpdater {
public:
    /// Updates the game system, and also updates voxel components for space system
    /// planet transitions.
    /// @param gameSystem: Game ECS
    /// @param spaceSystem: Space ECS. Only SphericalVoxelComponents are modified.
    void update(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem);
    /// Checks to see if there should be any voxel planet transitions, and possibly
    /// adds or removes spherical voxel components from SpaceSystem
    /// @param gameSystem: Game ECS
    /// @param spaceSystem: Space ECS. Only SphericalVoxelComponents are modified.
    void updateVoxelPlanetTransitions(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem);
private:
    PhysicsComponentUpdater physicsUpdater;
    CollisionComponentUpdater collisionUpdater;
    /// Updates movement components
    /// @param gameSystem: Game ECS
    /// TODO(Ben): Don't think this is the right func
    void updateMoveInput(OUT GameSystem* gameSystem);
    /// Calculates voxel position from relative space position
    /// @param relPos: relative position of the entity against the world center
    /// @param radius: Radius of the world
    /// @param vpcmp: Voxel position component of the entity
    void computeVoxelPosition(const f64v3& relPos, f32 radius, OUT VoxelPositionComponent& vpcmp);

    int m_frameCounter = 0; ///< Counts frames for updateVoxelPlanetTransitions updates
};

#endif // GameSystemUpdater_h__

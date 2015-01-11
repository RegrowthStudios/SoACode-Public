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

class GameSystem;
class SpaceSystem;

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
    /// Updates physics components
    /// @param gameSystem: Game ECS
    /// @param spaceSystem: Space ECS.
    void updatePhysics(OUT GameSystem* gameSystem, const SpaceSystem* spaceSystem);
    /// Updates collision components
    /// @param gameSystem: Game ECS
    void updateCollision(OUT GameSystem* gameSystem);
    /// Updates movement components
    /// @param gameSystem: Game ECS
    void updateMoveInput(OUT GameSystem* gameSystem);

    int m_frameCounter = 0; ///< Counts frames for updateVoxelPlanetTransitions updates
};

#endif // GameSystemUpdater_h__

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

#include "CollisionComponentUpdater.h"
#include "FreeMoveComponentUpdater.h"
#include "FrustumComponentUpdater.h"
#include "PhysicsComponentUpdater.h"
#include <Vorb/Events.hpp>
#include <Vorb/VorbPreDecl.inl>

class GameSystem;
class InputManager;
class SpaceSystem;
struct VoxelPositionComponent;
class SoaState;
DECL_VVOX(class VoxelPlanetMapData);

class GameSystemUpdater {
public:
    GameSystemUpdater(OUT GameSystem* gameSystem, InputManager* inputManager);
    /// Updates the game system, and also updates voxel components for space system
    /// planet transitions.
    /// @param gameSystem: Game ECS
    /// @param spaceSystem: Space ECS. Only SphericalVoxelComponents are modified.
    void update(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem, const SoaState* soaState);
    /// Checks to see if there should be any voxel planet transitions, and possibly
    /// adds or removes spherical voxel components from SpaceSystem
    /// @param gameSystem: Game ECS
    /// @param spaceSystem: Space ECS. Only SphericalVoxelComponents are modified.
    static void updateVoxelPlanetTransitions(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem, const SoaState* soaState);
private:
    PhysicsComponentUpdater physicsUpdater;
    CollisionComponentUpdater collisionUpdater;
    FreeMoveComponentUpdater freeMoveUpdater;
    FrustumComponentUpdater frustumUpdater;
    /// Calculates voxel position from relative space position
    /// @param relPos: relative position of the entity against the world center
    /// @param radius: Radius of the world
    /// @param mapData: Mapping data for spherical voxel position
    /// @param pos: the resulting voxel grid relative position
    static void computeVoxelPosition(const f64v3& relPos, f32 radius, OUT vvox::VoxelPlanetMapData& mapData, OUT f64v3& pos);

    int m_frameCounter = 0; ///< Counts frames for updateVoxelPlanetTransitions updates

    /// Delegates
    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    IDelegate<ui32>* m_onForwardKeyDown = nullptr;
    IDelegate<ui32>* m_onForwardKeyUp = nullptr;
    IDelegate<ui32>* m_onRightKeyDown = nullptr;
    IDelegate<ui32>* m_onRightKeyUp = nullptr;
    IDelegate<ui32>* m_onLeftKeyDown = nullptr;
    IDelegate<ui32>* m_onLeftKeyUp = nullptr;
    IDelegate<ui32>* m_onBackwardKeyDown = nullptr;
    IDelegate<ui32>* m_onBackwardKeyUp = nullptr;
    IDelegate<ui32>* m_onLeftRollKeyDown = nullptr;
    IDelegate<ui32>* m_onLeftRollKeyUp = nullptr;
    IDelegate<ui32>* m_onRightRollKeyDown = nullptr;
    IDelegate<ui32>* m_onRightRollKeyUp = nullptr;
    IDelegate<ui32>* m_onUpKeyDown = nullptr;
    IDelegate<ui32>* m_onUpKeyUp = nullptr;
    IDelegate<ui32>* m_onDownKeyDown = nullptr;
    IDelegate<ui32>* m_onDownKeyUp = nullptr;
    IDelegate<ui32>* m_onSuperSpeedKeyDown = nullptr;
    IDelegate<ui32>* m_onSuperSpeedKeyUp = nullptr;
};

#endif // GameSystemUpdater_h__

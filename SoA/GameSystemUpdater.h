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

#include "ChunkSphereComponentUpdater.h"
#include "CollisionComponentUpdater.h"
#include "FreeMoveComponentUpdater.h"
#include "FrustumComponentUpdater.h"
#include "InputMapper.h"
#include "PhysicsComponentUpdater.h"
#include "ParkourComponentUpdater.h"
#include "VoxelCoordinateSpaces.h"
#include <Vorb/Events.hpp>
#include <Vorb/VorbPreDecl.inl>

struct SoaState;
class SpaceSystem;
struct VoxelPositionComponent;
struct SoaState;
DECL_VVOX(class VoxelPlanetMapData);

class GameSystemUpdater {
    friend class GameSystemEvents;
public:
    GameSystemUpdater(OUT SoaState* soaState, InputMapper* inputMapper);
    ~GameSystemUpdater();
    /// Updates the game system, and also updates voxel components for space system
    /// planet transitions.
    /// @param gameSystem: Game ECS
    /// @param spaceSystem: Space ECS. Only SphericalVoxelComponents are modified.
    void update(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem, const SoaState* soaState);
private:

    int m_frameCounter = 0; ///< Counts frames for updateVoxelPlanetTransitions updates

    // Events
    std::unique_ptr<GameSystemEvents> m_events = nullptr;

    /// Updaters
    PhysicsComponentUpdater m_physicsUpdater;
    CollisionComponentUpdater m_collisionUpdater;
    FreeMoveComponentUpdater m_freeMoveUpdater;
    ParkourComponentUpdater m_parkourUpdater;
    ChunkSphereComponentUpdater m_chunkSphereUpdater;
    FrustumComponentUpdater m_frustumUpdater;

    const SoaState* m_soaState = nullptr;
    InputMapper* m_inputMapper = nullptr;
};

#endif // GameSystemUpdater_h__

///
/// GameSystemAssemblages.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component and entity assemblages for GameSystem
///

#pragma once

#ifndef GameSystemAssemblages_h__
#define GameSystemAssemblages_h__

#include <Vorb/ecs/Entity.h>

class ChunkAccessor;
class GameSystem;

#include "GameSystemComponents.h"

namespace GameSystemAssemblages {

    /************************************************************************/
    /* Component Factories                                                  */
    /************************************************************************/
    /// Free movement component
    vecs::ComponentID addFreeMoveInput(GameSystem* gameSystem, vecs::EntityID entity,
                                               vecs::ComponentID physicsComponent);
    void removeFreeMoveInput(GameSystem* gameSystem, vecs::EntityID entity);
    /// Physics component
    vecs::ComponentID addPhysics(GameSystem* gameSystem, vecs::EntityID entity,
                                         f32 massKg, const f64v3& initialVel,
                                         vecs::ComponentID spacePositionComponent,
                                         vecs::ComponentID voxelPositionComponent = 0);
    void removePhysics(GameSystem* gameSystem, vecs::EntityID entity);
    /// Space position component
    vecs::ComponentID addSpacePosition(GameSystem* gameSystem, vecs::EntityID entity,
                                               const f64v3& position, const f64q& orientation,
                                               vecs::EntityID parentEntity,
                                               vecs::ComponentID parentGravComponent = 0,
                                               vecs::ComponentID parentSphericalTerrainComponent = 0);
    void removeSpacePosition(GameSystem* gameSystem, vecs::EntityID entity);
    /// AABB Collision component
    vecs::ComponentID addAabbCollidable(GameSystem* gameSystem, vecs::EntityID entity,
                                                const f32v3& box, const f32v3& offset);
    void removeAabbCollidable(GameSystem* gameSystem, vecs::EntityID entity);
    /// Voxel Position Component
    vecs::ComponentID addVoxelPosition(GameSystem* gameSystem, vecs::EntityID entity,
                                               vecs::ComponentID parentVoxelComponent,
                                               const f64q& orientation,
                                               const VoxelPosition3D& gridPosition);
    void removeVoxelPosition(GameSystem* gameSystem, vecs::EntityID entity);
    /// Voxel Position Component
    vecs::ComponentID addChunkSphere(GameSystem* gameSystem, vecs::EntityID entity,
                                     vecs::ComponentID voxelPosition,
                                     const i32v3& centerPosition,
                                     ui32 radius);
    void removeChunkSphere(GameSystem* gameSystem, vecs::EntityID entity);
    /// Frustum Component
    vecs::ComponentID addFrustumComponent(GameSystem* gameSystem, vecs::EntityID entity,
                                                  f32 fov, f32 aspectRatio, f32 znear, f32 zfar,
                                                  vecs::ComponentID spacePosition = 0,
                                                  vecs::ComponentID voxelPosition = 0,
                                                  vecs::ComponentID head = 0);
    void removeFrustumComponent(GameSystem* gameSystem, vecs::EntityID entity);
    /// Head Component
    vecs::ComponentID addHeadComponent(GameSystem* gameSystem, vecs::EntityID entity,
                                               f64 neckLength);
    void removeHeadComponent(GameSystem* gameSystem, vecs::EntityID entity);
}

#endif // GameSystemAssemblages_h__

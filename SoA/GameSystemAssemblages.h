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

class GameSystem;

#include "GameSystemComponents.h"

namespace GameSystemAssemblages {
    /************************************************************************/
    /* Entity Factories                                                     */
    /************************************************************************/
    /// Player entity
    extern vecs::EntityID createPlayer(GameSystem* gameSystem, const f64v3& spacePosition,
                                      const f64q& orientation, f32 massKg, const f64v3& initialVel,
                                      f32 fov, f32 aspectRatio,
                                      vecs::EntityID parentEntity,
                                      vecs::ComponentID parentGravComponent,
                                      vecs::ComponentID parentSphericalTerrainComponent,
                                      f32 znear = 0.01, f32 zfar = 100000.0f);
    extern void destroyPlayer(GameSystem* gameSystem, vecs::EntityID playerEntity);

    /************************************************************************/
    /* Component Factories                                                  */
    /************************************************************************/
    /// Free movement component
    extern vecs::ComponentID addFreeMoveInput(GameSystem* gameSystem, vecs::EntityID entity,
                                               vecs::ComponentID physicsComponent);
    extern void removeFreeMoveInput(GameSystem* gameSystem, vecs::EntityID entity);
    /// Physics component
    extern vecs::ComponentID addPhysics(GameSystem* gameSystem, vecs::EntityID entity,
                                         f32 massKg, const f64v3& initialVel,
                                         vecs::ComponentID spacePositionComponent,
                                         vecs::ComponentID voxelPositionComponent = 0);
    extern void removePhysics(GameSystem* gameSystem, vecs::EntityID entity);
    /// Space position component
    extern vecs::ComponentID addSpacePosition(GameSystem* gameSystem, vecs::EntityID entity,
                                               const f64v3& position, const f64q& orientation,
                                               vecs::EntityID parentEntity,
                                               vecs::ComponentID parentGravComponent = 0,
                                               vecs::ComponentID parentSphericalTerrainComponent = 0);
    extern void removeSpacePosition(GameSystem* gameSystem, vecs::EntityID entity);
    /// AABB Collision component
    extern vecs::ComponentID addAabbCollidable(GameSystem* gameSystem, vecs::EntityID entity,
                                                const f32v3& box, const f32v3& offset);
    extern void removeAabbCollidable(GameSystem* gameSystem, vecs::EntityID entity);
    /// Voxel Position Component
    extern vecs::ComponentID addVoxelPosition(GameSystem* gameSystem, vecs::EntityID entity,
                                               vecs::ComponentID parentVoxelComponent,
                                               const f64q& orientation,
                                               const VoxelPosition3D& gridPosition);
    extern void removeVoxelPosition(GameSystem* gameSystem, vecs::EntityID entity);
    /// Frustum Component
    extern vecs::ComponentID addFrustumComponent(GameSystem* gameSystem, vecs::EntityID entity,
                                                  f32 fov, f32 aspectRatio, f32 znear, f32 zfar,
                                                  vecs::ComponentID spacePosition = 0,
                                                  vecs::ComponentID voxelPosition = 0,
                                                  vecs::ComponentID head = 0);
    extern void removeFrustumComponent(GameSystem* gameSystem, vecs::EntityID entity);
    /// Head Component
    extern vecs::ComponentID addHeadComponent(GameSystem* gameSystem, vecs::EntityID entity,
                                               f64 neckLength);
    extern void removeHeadComponent(GameSystem* gameSystem, vecs::EntityID entity);
}

#endif // GameSystemAssemblages_h__

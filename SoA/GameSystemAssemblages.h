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
    extern vcore::EntityID createPlayer(GameSystem* gameSystem, const f64v3& spacePosition,
                                      const f64q& orientation, f32 massKg, const f64v3& initialVel,
                                      f32 fov, f32 aspectRatio, vcore::ComponentID parentGravComponent,
                                      vcore::ComponentID parentSphericalTerrainComponent,
                                      f32 znear = 0.01, f32 zfar = 100000.0f);
    extern void destroyPlayer(GameSystem* gameSystem, vcore::EntityID playerEntity);

    /************************************************************************/
    /* Component Factories                                                  */
    /************************************************************************/
    /// Free movement component
    extern vcore::ComponentID addFreeMoveInput(GameSystem* gameSystem, vcore::EntityID entity,
                                               vcore::ComponentID physicsComponent);
    extern void removeFreeMoveInput(GameSystem* gameSystem, vcore::EntityID entity);
    /// Physics component
    extern vcore::ComponentID addPhysics(GameSystem* gameSystem, vcore::EntityID entity,
                                         f32 massKg, const f64v3& initialVel,
                                         vcore::ComponentID spacePositionComponent,
                                         vcore::ComponentID voxelPositionComponent = 0);
    extern void removePhysics(GameSystem* gameSystem, vcore::EntityID entity);
    /// Space position component
    extern vcore::ComponentID addSpacePosition(GameSystem* gameSystem, vcore::EntityID entity,
                                               const f64v3& position, const f64q& orientation,
                                               vcore::ComponentID parentGravComponent = 0,
                                               vcore::ComponentID parentSphericalTerrainComponent = 0);
    extern void removeSpacePosition(GameSystem* gameSystem, vcore::EntityID entity);
    /// AABB Collision component
    extern vcore::ComponentID addAabbCollidable(GameSystem* gameSystem, vcore::EntityID entity,
                                                const f32v3& box, const f32v3& offset);
    extern void removeAabbCollidable(GameSystem* gameSystem, vcore::EntityID entity);
    /// Voxel Position Component
    extern vcore::ComponentID addVoxelPosition(GameSystem* gameSystem, vcore::EntityID entity,
                                               vcore::ComponentID parentVoxelComponent,
                                               const f64q& orientation,
                                               const VoxelPosition3D& gridPosition);
    extern void removeVoxelPosition(GameSystem* gameSystem, vcore::EntityID entity);
    /// Frustum Component
    extern vcore::ComponentID addFrustumComponent(GameSystem* gameSystem, vcore::EntityID entity,
                                                  f32 fov, f32 aspectRatio, f32 znear, f32 zfar,
                                                  vcore::ComponentID spacePosition = 0,
                                                  vcore::ComponentID voxelPosition = 0,
                                                  vcore::ComponentID head = 0);
    extern void removeFrustumComponent(GameSystem* gameSystem, vcore::EntityID entity);
    /// Head Component
    extern vcore::ComponentID addHeadComponent(GameSystem* gameSystem, vcore::EntityID entity,
                                               f64 neckLength);
    extern void removeHeadComponent(GameSystem* gameSystem, vcore::EntityID entity);
}

#endif // GameSystemAssemblages_h__

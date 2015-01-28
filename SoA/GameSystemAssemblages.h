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
    extern vcore::EntityID createPlayer(OUT GameSystem* gameSystem, const f64v3& spacePosition,
                                      const f64q& orientation, f32 massKg, const f64v3& initialVel,
                                      f32 fov, f32 aspectRatio, f32 znear = 0.01, f32 zfar = 100000.0f);
    extern void destroyPlayer(OUT GameSystem* gameSystem, vcore::EntityID playerEntity);

    /************************************************************************/
    /* Component Factories                                                  */
    /************************************************************************/
    /// Free movement component
    extern vcore::ComponentID addFreeMoveInput(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                               vcore::ComponentID physicsComponent);
    extern void removeFreeMoveInput(OUT GameSystem* gameSystem, vcore::EntityID entity);
    /// Physics component
    extern vcore::ComponentID addPhysics(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                         f32 massKg, const f64v3& initialVel,
                                         vcore::ComponentID spacePositionComponent,
                                         OPT vcore::ComponentID voxelPositionComponent = 0);
    extern void removePhysics(OUT GameSystem* gameSystem, vcore::EntityID entity);
    /// Space position component
    extern vcore::ComponentID addSpacePosition(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                               const f64v3& position, const f64q& orientation);
    extern void removeSpacePosition(OUT GameSystem* gameSystem, vcore::EntityID entity);
    /// AABB Collision component
    extern vcore::ComponentID addAabbCollidable(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                const f32v3& box, const f32v3& offset);
    extern void removeAabbCollidable(OUT GameSystem* gameSystem, vcore::EntityID entity);
    /// Voxel Position Component
    extern vcore::ComponentID addVoxelPosition(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                               vcore::ComponentID parentVoxelComponent,
                                               const VoxelPosition& position,
                                               const f64q& orientation,
                                               vvox::VoxelPlanetMapData mapData);
    extern void removeVoxelPosition(OUT GameSystem* gameSystem, vcore::EntityID entity);
    /// Frustum Component
    extern vcore::ComponentID addFrustumComponent(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                                  f32 fov, f32 aspectRatio, f32 znear, f32 zfar,
                                                  vcore::ComponentID spacePosition = 0,
                                                  vcore::ComponentID voxelPosition = 0,
                                                  vcore::ComponentID head = 0);
    extern void removeFrustumComponent(OUT GameSystem* gameSystem, vcore::EntityID entity);
    /// Head Component
    extern vcore::ComponentID addHeadComponent(OUT GameSystem* gameSystem, vcore::EntityID entity,
                                               f64 neckLength);
    extern void removeHeadComponent(OUT GameSystem* gameSystem, vcore::EntityID entity);
}

#endif // GameSystemAssemblages_h__

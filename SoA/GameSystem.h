///
/// GameSystem.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Entity Component System for the main game entities.
///

#pragma once

#ifndef GameSystem_h__
#define GameSystem_h__

#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/ecs/ECS.h>

#include "GameSystemComponents.h"

class GameSystem : public vecs::ECS {
public:
    GameSystem();
    vecs::ComponentTable<AabbCollidableComponent> aabbCollidable;
    vecs::ComponentTable<FreeMoveInputComponent> freeMoveInput;
    vecs::ComponentTable<PhysicsComponent> physics;
    vecs::ComponentTable<SpacePositionComponent> spacePosition;
    vecs::ComponentTable<VoxelPositionComponent> voxelPosition;
    vecs::ComponentTable<FrustumComponent> frustum;
    vecs::ComponentTable<HeadComponent> head;
};

#endif // GameSystem_h__

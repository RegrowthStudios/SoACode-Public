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

class GameSystem : public vcore::ECS {
public:
    GameSystem();
    vcore::ComponentTable<AabbCollidableComponent> aabbCollidable;
    vcore::ComponentTable<FreeMoveInputComponent> freeMoveInput;
    vcore::ComponentTable<PhysicsComponent> physics;
    vcore::ComponentTable<SpacePositionComponent> spacePosition;
    vcore::ComponentTable<VoxelPositionComponent> voxelPosition;
    vcore::ComponentTable<FrustumComponent> frustum;
    vcore::ComponentTable<HeadComponent> head;
};

#endif // GameSystem_h__

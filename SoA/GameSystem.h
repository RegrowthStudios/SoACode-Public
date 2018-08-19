///
/// GameSystem.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
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

#define GAME_SYSTEM_CT_AABBCOLLIDABLE_NAME "AABBCollidable"
#define GAME_SYSTEM_CT_FREEMOVEINPUT_NAME "FreeMoveInput"
#define GAME_SYSTEM_CT_PARKOURINPUT_NAME "Parkour"
#define GAME_SYSTEM_CT_PHYSICS_NAME "Physics"
#define GAME_SYSTEM_CT_SPACEPOSITION_NAME "SpacePosition"
#define GAME_SYSTEM_CT_VOXELPOSITION_NAME "VoxelPosition"
#define GAME_SYSTEM_CT_FRUSTUM_NAME "Frustum"
#define GAME_SYSTEM_CT_HEAD_NAME "Head"
#define GAME_SYSTEM_CT_INVENTORY_NAME "Inventory"
#define GAME_SYSTEM_CT_CHUNKSPHERE_NAME "ChunkSphere"
#define GAME_SYSTEM_CT_ATTRIBUTES_NAME "Attributes"

class GameSystem : public vecs::ECS {
public:
    GameSystem();

    AABBCollidableComponentTable aabbCollidable;
    FreeMoveInputComponentTable freeMoveInput;
    ParkourInputComponentTable parkourInput;
    PhysicsComponentTable physics;
    SpacePositionComponentTable spacePosition;
    VoxelPositionComponentTable voxelPosition;
    FrustumComponentTable frustum;
    HeadComponentTable head;
    InventoryComponentTable inventory;
    ChunkSphereComponentTable chunkSphere;
    AttributeComponentTable attributes;

    vecs::ComponentID getComponent(const nString& name, vecs::EntityID eID);

private:
    VORB_NON_COPYABLE(GameSystem);
};

#endif // GameSystem_h__

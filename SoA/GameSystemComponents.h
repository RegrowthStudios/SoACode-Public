///
/// GameSystemComponents.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Components for game system
///

#pragma once

#ifndef GameSystemComponents_h__
#define GameSystemComponents_h__

#include <Vorb/io/Keg.h>
#include <Vorb/ecs/Entity.h>

#include <Vorb/ecs/ECS.h>
#include <Vorb/ecs/ComponentTable.hpp>

#include "BlockData.h"
#include "ChunkHandle.h"
#include "Frustum.h"
#include "VoxelCoordinateSpaces.h"

class ChunkAccessor;
class ChunkGrid;

struct BlockCollisionData {
    BlockCollisionData(BlockID id, ui16 index) : id(id), index(index), neighborCollideFlags(0) {}
    BlockID id;
    ui16 index;
    union {
        struct {
            bool left : 1;
            bool right : 1;
            bool bottom : 1;
            bool top : 1;
            bool back : 1;
            bool front : 1;
        };
        ui8 neighborCollideFlags;
    };
};

struct AabbCollidableComponent {
    vecs::ComponentID physics;
    std::map<ChunkID, std::vector<BlockCollisionData>> voxelCollisions;
    // TODO(Ben): Entity-Entity collision
    f32v3 box = f32v3(0.0f); ///< x, y, z widths in blocks
    f32v3 offset = f32v3(0.0f); ///< x, y, z offsets in blocks
};
typedef vecs::ComponentTable<AabbCollidableComponent> AABBCollidableComponentTable;
KEG_TYPE_DECL(AabbCollidableComponent);

struct AttributeComponent {
    f64 strength = 0.0;
    f64 perception = 0.0;
    f64 endurance = 0.0;
    f64 charisma = 0.0;
    f64 intelligence = 0.0;
    f64 agility = 0.0;
    f64 height = 4.0;
};
typedef vecs::ComponentTable<AttributeComponent> AttributeComponentTable;
KEG_TYPE_DECL(AttributeComponent);

struct ParkourInputComponent {
    // Bitfield inputs
    union {
        struct {
            bool moveForward : 1; ///< True when moving forward
            bool moveBackward : 1; ///< True when moving backward
            bool moveLeft : 1; ///< True when moving left
            bool moveRight : 1; ///< True when moving right
            bool jump : 1; ///< True when attempting to jump
            bool crouch : 1; ///< True when attempting to crouch
            bool parkour : 1; ///< True when parkouring
            bool sprint : 1; ///< True when sprinting
        };
        ui8 moveFlags = 0;
    };
    vecs::ComponentID aabbCollidable;
    vecs::ComponentID physicsComponent;
    vecs::ComponentID attributeComponent;
    vecs::ComponentID headComponent;
};
typedef vecs::ComponentTable<ParkourInputComponent> ParkourInputComponentTable;

struct FreeMoveInputComponent {
    // Bitfield inputs
    union {
        struct {
            bool tryMoveForward : 1; ///< True when moving forward
            bool tryMoveBackward : 1; ///< True when moving backward
            bool tryMoveLeft : 1; ///< True when moving left
            bool tryMoveRight : 1; ///< True when moving right
            bool tryMoveUp : 1; ///< True when attempting to go up
            bool tryMoveDown : 1; ///< True when attempting to go down
            bool superSpeed : 1; ///< True when super speed is active
            bool tryRollLeft : 1; ///< True when trying to roll left along cam Z
            bool tryRollRight : 1; ///< True when trying to roll right along cam Z
        };
        ui16 moveFlags = 0;
    };
    vecs::ComponentID physicsComponent = 0;
    float speed = 0.3f;
};
typedef vecs::ComponentTable<FreeMoveInputComponent> FreeMoveInputComponentTable;
//KEG_TYPE_DECL(FreeMoveInputComponent);

struct SpacePositionComponent {
    f64v3 position = f64v3(0.0);
    f64q orientation;
    vecs::EntityID parentEntity = 0;
    vecs::ComponentID parentGravity = 0; ///< Gravity component of parent system body
    vecs::ComponentID parentSphericalTerrain = 0; ///< ST component of parent system body
};
typedef vecs::ComponentTable<SpacePositionComponent> SpacePositionComponentTable;
KEG_TYPE_DECL(SpacePositionComponent);

typedef f64v3 VoxelPosition;

struct VoxelPositionComponent {
    f64q orientation;
    f64v3 eulerAngles = f64v3(0.0); ///< Pitch, Yaw, Roll in radians.
    VoxelPosition3D gridPosition;
    vecs::ComponentID parentVoxel = 0;
};
typedef vecs::ComponentTable<VoxelPositionComponent> VoxelPositionComponentTable;
KEG_TYPE_DECL(VoxelPositionComponent);

struct ChunkSphereComponent {
    vecs::ComponentID voxelPosition;

    std::vector<ChunkHandle> activeChunks;

    // TODO(Ben): Chunk position?
    i32v3 offset = i32v3(0);
    i32v3 centerPosition = i32v3(0);
    ChunkGrid* chunkGrid = nullptr;
    ChunkHandle* handleGrid = nullptr;
    // For fast 1 chunk shift
    std::vector<i32v3> acquireOffsets;
    WorldCubeFace currentCubeFace = FACE_NONE;

    i32 radius = 0;
    i32 width = 0;
    i32 layer = 0;
    i32 size = 0;
};
class ChunkSphereComponentTable : public vecs::ComponentTable<ChunkSphereComponent> {
public:
    virtual void disposeComponent(vecs::ComponentID cID, vecs::EntityID eID VORB_MAYBE_UNUSED) override {
        ChunkSphereComponent& cmp = _components[cID].second;
        delete[] cmp.handleGrid;
        cmp.handleGrid = nullptr;
        cmp.chunkGrid = nullptr;
    }
};

struct PhysicsComponent {
    f64v3 velocity = f64v3(0.0);
    f32 mass;
    vecs::ComponentID spacePosition = 0; ///< Optional
    vecs::ComponentID voxelPosition = 0; ///< Optional
};
typedef vecs::ComponentTable<PhysicsComponent> PhysicsComponentTable;
KEG_TYPE_DECL(PhysicsComponent);

struct FrustumComponent {
    Frustum frustum;
    vecs::ComponentID spacePosition = 0; ///< Optional
    vecs::ComponentID voxelPosition = 0; ///< Optional
    vecs::ComponentID head = 0; ///< Optional
};
typedef vecs::ComponentTable<FrustumComponent> FrustumComponentTable;
KEG_TYPE_DECL(FrustumComponent);

struct HeadComponent {
    vecs::ComponentID voxelPosition;
    f64q relativeOrientation;
    f64v3 eulerAngles = f64v3(0.0); ///< Pitch, Yaw, Roll in radians.
    f64v3 angleToApply = f64v3(0.0);
    f64v3 relativePosition = f64v3(0.0); ///< Position in voxel units relative to entity position
    f64 neckLength = 0.0; ///< Neck length in voxel units
};
typedef vecs::ComponentTable<HeadComponent> HeadComponentTable;
KEG_TYPE_DECL(HeadComponent);

struct InventoryComponent {
    std::vector<ItemStack> items; ///< Who needs fast lookups?
};
typedef vecs::ComponentTable<InventoryComponent> InventoryComponentTable;

#endif // GameSystemComponents_h__

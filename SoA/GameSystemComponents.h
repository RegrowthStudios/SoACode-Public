///
/// GameSystemComponents.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Components for game system
///

#pragma once

#ifndef GameSystemComponents_h__
#define GameSystemComponents_h__

#include <Vorb/ecs/Entity.h>
#include <Vorb/ecs/ComponentTable.hpp>

#include "ChunkHandle.h"
#include "Frustum.h"
#include "VoxelCoordinateSpaces.h"

class ChunkAccessor;

struct AabbCollidableComponent {
    f32v3 box = f32v3(0.0f); ///< x, y, z widths in blocks
    f32v3 offset = f32v3(0.0f); ///< x, y, z offsets in blocks
};

struct ParkourInputComponent {
    // Bitfield inputs
    union {
        struct {
            bool tryMoveForward : 1; ///< True when moving forward
            bool tryMoveBackward : 1; ///< True when moving backward
            bool tryMoveLeft : 1; ///< True when moving left
            bool tryMoveRight : 1; ///< True when moving right
            bool tryJump : 1; ///< True when attempting to jump
            bool tryCrouch : 1; ///< True when attempting to crouch
            bool tryParkour : 1; ///< True when parkouring
            bool trySprint : 1; ///< True when sprinting
        };
        ui8 moveFlags = 0;
    };
    vecs::ComponentID physicsComponent;
    float acceleration;
    float maxSpeed;
};

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

struct SpacePositionComponent {
    vecs::EntityID parentEntity = 0;
    vecs::ComponentID parentGravityID = 0; ///< Gravity component of parent system body
    vecs::ComponentID parentSphericalTerrainID = 0; ///< ST component of parent system body
    f64v3 position = f64v3(0.0);
    f64q orientation;
};

typedef f64v3 VoxelPosition;

struct VoxelPositionComponent {
    f64q orientation;
    VoxelPosition3D gridPosition;
    vecs::ComponentID parentVoxelComponent = 0;
};

struct ChunkSphereComponent {
    vecs::ComponentID voxelPosition;

    std::vector<ChunkHandle> activeChunks;

    // TODO(Ben): Chunk position?
    i32v3 offset = i32v3(0);
    i32v3 centerPosition = i32v3(0);
    ui32 radius = 0;
    ChunkAccessor* accessor = nullptr;
    ChunkHandle* handleGrid = nullptr;
    // For fast 1 chunk shift
    std::vector<i32v3> acquireOffsets;

    i32 width = 0;
    i32 layer = 0;
    i32 size = 0;
};
class ChunkSphereComponentTable : public vecs::ComponentTable<ChunkSphereComponent> {
public:
    virtual void disposeComponent(vecs::ComponentID cID, vecs::EntityID eID) override {
        ChunkSphereComponent& cmp = _components[cID].second;
        delete[] cmp.handleGrid;
    }
};

struct PhysicsComponent {
    vecs::ComponentID spacePositionComponent = 0; ///< Optional
    vecs::ComponentID voxelPositionComponent = 0; ///< Optional
    f64v3 velocity = f64v3(0.0);
    f32 mass;
};

struct FrustumComponent {
    vecs::ComponentID spacePositionComponent = 0; ///< Optional
    vecs::ComponentID voxelPositionComponent = 0; ///< Optional
    vecs::ComponentID headComponent = 0; ///< Optional
    Frustum frustum;
};

struct HeadComponent {
    f64q relativeOrientation;
    f64v3 relativePosition = f64v3(0.0); ///< Position in voxel units
    f64 neckLength = 0.0; ///< Neck length in voxel units
};

#endif // GameSystemComponents_h__
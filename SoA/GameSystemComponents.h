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

#include "Frustum.h"
#include "VoxelCoordinateSpaces.h"

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
    vcore::ComponentID physicsComponent;
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
    vcore::ComponentID physicsComponent = 0;
    float speed = 0.3f;
};

struct SpacePositionComponent {
    f64v3 position = f64v3(0.0);
    f64q orientation;
};

typedef f64v3 VoxelPosition;

struct VoxelPositionComponent {
    f64q orientation;
    VoxelPosition3D gridPosition;
    vcore::ComponentID parentVoxelComponent = 0;
};

struct PhysicsComponent {
    vcore::ComponentID spacePositionComponent = 0; ///< Optional
    vcore::ComponentID voxelPositionComponent = 0; ///< Optional
    f64v3 velocity = f64v3(0.0);
    f32 mass;
};

struct FrustumComponent {
    vcore::ComponentID spacePositionComponent = 0; ///< Optional
    vcore::ComponentID voxelPositionComponent = 0; ///< Optional
    vcore::ComponentID headComponent = 0; ///< Optional
    Frustum frustum;
};

struct HeadComponent {
    f64q relativeOrientation;
    f64v3 relativePosition = f64v3(0.0); ///< Position in voxel units
    f64 neckLength = 0.0; ///< Neck length in voxel units
};

#endif // GameSystemComponents_h__
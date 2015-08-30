#include "stdafx.h"
#include "ParkourComponentUpdater.h"

#define GLM_FORCE_RADIANS

#include "GameSystem.h"
#include "SpaceSystem.h"

void ParkourComponentUpdater::update(GameSystem* gameSystem) {
    for (auto& it : gameSystem->parkourInput) {
        auto& parkour = it.second;
        auto& physics = gameSystem->physics.get(parkour.physicsComponent);
        if (physics.voxelPosition == 0) return;

        auto& attributes = gameSystem->attributes.get(parkour.attributeComponent);
        auto& voxelPosition = gameSystem->voxelPosition.get(physics.voxelPosition);
        auto& head = gameSystem->head.get(parkour.headComponent);

        // TODO(Ben): Timestep
        // TODO(Ben): Account mass
        f64 acceleration = 1.0 + attributes.agility * 0.01;
        f64 maxSpeed = 0.5 + attributes.agility * 0.005;
        if (parkour.crouch) {
            maxSpeed *= 0.3f;
        } else if (parkour.sprint) {
            maxSpeed *= 2.0f;
        }

        f64v3 targetVel(0.0);
        int inputCount = 0;

        // Get move direction
        // TODO(Ben): Gamepad support
        if (parkour.moveForward) {
            targetVel.z = 1.0f;
            inputCount++;
        } else if (parkour.moveBackward) {
            targetVel.z = -1.0f;
            inputCount++;
        }

        if (parkour.moveLeft) {
            targetVel.x = 1.0f;
            inputCount++;
        } else if (parkour.moveRight) {
            targetVel.x = -1.0f;
            inputCount++;
        }

        // Get angles
        f64v3& euler = voxelPosition.eulerAngles;

        if (inputCount != 0) {
            // Normalize for diagonal
            if (inputCount == 2) {
                targetVel = vmath::normalize(targetVel);
            }
            // Use head yaw for body when moving
            euler.y += head.eulerAngles.y;
            head.eulerAngles.y = 0.0f;
        }

        // Check for pitch (Should be no pitch)
        if (euler.x != 0.0) {
            euler.x *= 0.95;
            if (ABS(euler.x) < 0.01) {
                euler.x = 0.0;
            }
        }
        // Check for roll (Should be no roll)
        if (euler.z != 0.0) {
            euler.z *= 0.95;
            if (ABS(euler.z) < 0.01) {
                euler.z = 0.0;
            }
        }

        voxelPosition.orientation = f64q(euler);

        targetVel *= maxSpeed;
        targetVel = voxelPosition.orientation * targetVel;

        static const f64 step = 0.1;
        f64v3 dVel = targetVel - physics.velocity;
        f64 l = vmath::length(dVel);
        if (l < step) {
            physics.velocity = targetVel;
        } else {
            physics.velocity += dVel * step;
        }
    }
}

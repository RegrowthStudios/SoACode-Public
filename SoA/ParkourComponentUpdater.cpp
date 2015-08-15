#include "stdafx.h"
#include "ParkourComponentUpdater.h"

#include "GameSystem.h"
#include "SpaceSystem.h"

void ParkourComponentUpdater::update(GameSystem* gameSystem) {
    for (auto& it : gameSystem->parkourInput) {
        auto& parkour = it.second;
        auto& physics = gameSystem->physics.get(parkour.physicsComponent);
        if (physics.voxelPositionComponent == 0) return;

        auto& attributes = gameSystem->attributes.get(parkour.attributeComponent);
        auto& voxelPosition = gameSystem->voxelPosition.get(physics.voxelPositionComponent);

        // TODO(Ben): Timestep
        // TODO(Ben): Account mass
        f64 acceleration = 1.0 + attributes.agility * 0.01;
        f64 maxSpeed = 0.5 + attributes.agility * 0.005;

        f64v3 targetVel(0.0);

        // Get move direction
        // TODO(Ben): Gamepad support
        if (parkour.moveForward) {
            targetVel.z = 1.0f;
        } else if (parkour.moveBackward) {
            targetVel.z = -1.0f;
        }

        if (parkour.moveLeft) {
            targetVel.x = -1.0f;
        } else if (parkour.moveRight) {
            targetVel.x = 1.0f;
        }

        // Normalize for diagonal
        if (targetVel.x && targetVel.z) {
            targetVel = glm::normalize(targetVel);
        }

        targetVel *= maxSpeed;
        targetVel = voxelPosition.orientation * targetVel;

        static const f64 step = 0.1;
        f64v3 dVel = targetVel - physics.velocity;
        f64 l = glm::length(dVel);
        if (l < step) {
            physics.velocity = targetVel;
        } else {
            physics.velocity += (dVel / l) * step;
        }
    }
}

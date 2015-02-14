#include "stdafx.h"
#include "FreeMoveComponentUpdater.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include "GameSystem.h"

void FreeMoveComponentUpdater::update(GameSystem* gameSystem) {

    f64v3 forward, right, up;

    for (auto& it : gameSystem->freeMoveInput) {
        auto& fmcmp = it.second;
        auto& physcmp = gameSystem->physics.get(fmcmp.physicsComponent);

        f64q* orientation;
        // If there is a voxel component, we use voxel position
        if (physcmp.voxelPositionComponent) {
            orientation = &gameSystem->voxelPosition.get(physcmp.voxelPositionComponent).orientation;
        } else {
            orientation = &gameSystem->spacePosition.get(physcmp.spacePositionComponent).orientation;
        }

        f64 speed = (f64)fmcmp.speed;
        if (fmcmp.superSpeed) {
            speed *= 10000.0; // temporary
        }
        // Calculate velocity vector from inputs and speed
        physcmp.velocity = f64v3(0.0);
        if (fmcmp.tryMoveForward) {
            forward = *orientation * f64v3(0.0, 0.0, 1.0);
            physcmp.velocity += forward * speed;
        } else if (fmcmp.tryMoveBackward) {
            forward = *orientation * f64v3(0.0, 0.0, 1.0);
            physcmp.velocity -= forward * speed;
        }

        if (fmcmp.tryMoveRight) {
            right = *orientation * f64v3(-1.0, 0.0, 0.0);
            physcmp.velocity += right * speed;
        } else if (fmcmp.tryMoveLeft) {
            right = *orientation * f64v3(-1.0, 0.0, 0.0);
            physcmp.velocity -= right * speed;
        }

        if (fmcmp.tryMoveUp) {
            up = *orientation * f64v3(0.0, 1.0, 0.0);
            physcmp.velocity += up * speed;
        } else if (fmcmp.tryMoveDown) {
            up = *orientation * f64v3(0.0, 1.0, 0.0);
            physcmp.velocity -= up * speed;
        }

        #define ROLL_SPEED 0.7
        if (fmcmp.tryRollLeft) {
            forward = *orientation * f64v3(0.0, 0.0, 1.0);
            *orientation = glm::angleAxis(-ROLL_SPEED, forward) * (*orientation);
        } else if (fmcmp.tryRollRight) {
            forward = *orientation * f64v3(0.0, 0.0, 1.0);
            *orientation = glm::angleAxis(ROLL_SPEED, forward) * (*orientation);
        }
    }
}

void FreeMoveComponentUpdater::rotateFromMouse(GameSystem* gameSystem, FreeMoveInputComponent& cmp, float dx, float dy, float speed) {

    auto& physcmp = gameSystem->physics.get(cmp.physicsComponent);

    f64q* orientation;
    // If there is a voxel component, we use voxel position
    if (physcmp.voxelPositionComponent) {
        orientation = &gameSystem->voxelPosition.get(physcmp.voxelPositionComponent).orientation;
    } else {
        orientation = &gameSystem->spacePosition.get(physcmp.spacePositionComponent).orientation;
    }

    f64v3 right = *orientation * f64v3(1.0, 0.0, 0.0);
    f64v3 up = *orientation * f64v3(0.0, 1.0, 0.0);

    f64q upQuat = glm::angleAxis((f64)(dy * speed), right);
    f64q rightQuat = glm::angleAxis((f64)(dx * speed), up);

    *orientation = upQuat * rightQuat * (*orientation);
}

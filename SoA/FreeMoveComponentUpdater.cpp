#include "stdafx.h"
#include "FreeMoveComponentUpdater.h"

#include "SpaceSystem.h"
#include "GameSystem.h"
#include "Constants.h"

void FreeMoveComponentUpdater::update(GameSystem* gameSystem, SpaceSystem* spaceSystem) {
    //TODO(Ben): A lot of temporary code here
    f64v3 forward, right, up;

    for (auto& it : gameSystem->freeMoveInput) {
        auto& fmcmp = it.second;
        auto& physcmp = gameSystem->physics.get(fmcmp.physicsComponent);

        f64q* orientation;
        f64 acceleration = (f64)fmcmp.speed * 0.01;
        // If there is a voxel component, we use voxel position
        if (physcmp.voxelPosition) {
            // No acceleration on voxels
            physcmp.velocity = f64v3(0.0);
            acceleration = 1.0;
            auto& vpCmp = gameSystem->voxelPosition.get(physcmp.voxelPosition);
            //f64 radius = spaceSystem->sphericalGravity.get(vpCmp.parentVoxel).radius;
            orientation = &vpCmp.orientation;
            if (fmcmp.superSpeed) {
                static const f64 SS_Mult = 0.01;
                acceleration *= glm::min(600.0, glm::max(2.0, (SS_Mult * vpCmp.gridPosition.pos.y))); // temporary
            }
        } else {
            auto& spCmp = gameSystem->spacePosition.get(physcmp.spacePosition);
            f64 radius = spaceSystem->sphericalGravity.get(spCmp.parentGravity).radius;
            orientation = &spCmp.orientation;
            acceleration *= KM_PER_VOXEL;
            if (fmcmp.superSpeed) {
                static const f64 SS_Mult = 0.1;
                acceleration *= glm::max(2.0, (SS_Mult * (glm::length(spCmp.position) - radius))); // temporary, assumes a parent
            }
        }
       
        // Calculate velocity vector from inputs and speed
        if (fmcmp.tryMoveForward) {
            forward = *orientation * f64v3(0.0, 0.0, 1.0);
            physcmp.velocity += forward * acceleration;
        } else if (fmcmp.tryMoveBackward) {
            forward = *orientation * f64v3(0.0, 0.0, 1.0);
            physcmp.velocity -= forward * acceleration;
        }

        if (fmcmp.tryMoveRight) {
            right = *orientation * f64v3(-1.0, 0.0, 0.0);
            physcmp.velocity += right * acceleration;
        } else if (fmcmp.tryMoveLeft) {
            right = *orientation * f64v3(-1.0, 0.0, 0.0);
            physcmp.velocity -= right * acceleration;
        }

        if (fmcmp.tryMoveUp) {
            up = *orientation * f64v3(0.0, 1.0, 0.0);
            physcmp.velocity += up * acceleration;
        } else if (fmcmp.tryMoveDown) {
            up = *orientation * f64v3(0.0, 1.0, 0.0);
            physcmp.velocity -= up * acceleration;
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
    if (physcmp.voxelPosition) {
        orientation = &gameSystem->voxelPosition.get(physcmp.voxelPosition).orientation;
    } else {
        orientation = &gameSystem->spacePosition.get(physcmp.spacePosition).orientation;
    }

    f64v3 right = *orientation * f64v3(1.0, 0.0, 0.0);
    f64v3 up = *orientation * f64v3(0.0, 1.0, 0.0);

    f64q upQuat = glm::angleAxis((f64)(dy * speed), right);
    f64q rightQuat = glm::angleAxis((f64)(dx * speed), up);

    *orientation = upQuat * rightQuat * (*orientation);
}

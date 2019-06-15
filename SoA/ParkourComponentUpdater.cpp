#include "stdafx.h"
#include "ParkourComponentUpdater.h"

#define GLM_FORCE_RADIANS

#include "GameSystem.h"
#include "SpaceSystem.h"
#include "VoxelUtils.h"
#include "SoAState.h"

void ParkourComponentUpdater::update(GameSystem* gameSystem, SpaceSystem* spaceSystem VORB_UNUSED, const SoaState *soaState) {
    for (auto& it : gameSystem->parkourInput) {
        auto& parkour = it.second;
        auto& physics = gameSystem->physics.get(parkour.physicsComponent);
        if (physics.voxelPosition == 0) return;

        auto& attributes = gameSystem->attributes.get(parkour.attributeComponent);
        auto& voxelPosition = gameSystem->voxelPosition.get(physics.voxelPosition);
        auto& head = gameSystem->head.get(parkour.headComponent);
        auto& aabbCollidable = gameSystem->aabbCollidable.get(parkour.aabbCollidable);

        f64 deltaTime;

        if(m_lastTime<0.0f)
            deltaTime=0.0f;
        else
            deltaTime=soaState->time-m_lastTime;
        m_lastTime=soaState->time;

        // TODO(Ben): Timestep
        // TODO(Ben): Account mass
        // f64 acceleration = 1.0 + attributes.agility * 0.01;
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
                targetVel = glm::normalize(targetVel);
            }
            // Use head yaw for body when moving
            euler.y += head.eulerAngles.y;
            head.eulerAngles.y = 0.0f;
            head.relativeOrientation = f64q(head.eulerAngles);
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

        targetVel *= (maxSpeed*deltaTime);
        targetVel = voxelPosition.orientation * targetVel;

        static const f64 step = 0.1;
        f64v3 dVel = targetVel - f64v3(physics.velocity.x, 0.0f, physics.velocity.z);
        f64 l = glm::length(dVel);
        if (l < step) {
            physics.velocity.x = targetVel.x;
            physics.velocity.z = targetVel.z;
        } else {
            physics.velocity += dVel * step;
        }

        // Collision
        if (aabbCollidable.voxelCollisions.size() && voxelPosition.parentVoxel) {
            
            const f64v3 MIN_DISTANCE = f64v3(aabbCollidable.box) * 0.5 + 0.5;

            for (auto& it : aabbCollidable.voxelCollisions) {
                for (auto& cd : it.second) {
                    f64v3 aabbPos = voxelPosition.gridPosition.pos + f64v3(aabbCollidable.offset);

                    f64v3 vpos = f64v3(it.first.x, it.first.y, it.first.z) * (f64)CHUNK_WIDTH + f64v3(getPosFromBlockIndex(cd.index)) + 0.5;
                        
                    f64v3 dp = vpos - aabbPos;
                    f64v3 adp(glm::abs(dp));

                   // std::cout << MIN_DISTANCE.y - adp.y << std::endl;

                    // Check slow feet collision first
                    if (dp.y < 0 && MIN_DISTANCE.y - adp.y < 0.55 && !cd.top) {
                        voxelPosition.gridPosition.y += (MIN_DISTANCE.y - adp.y) * 0.01;
                        if (physics.velocity.y < 0) physics.velocity.y = 0.0;
                        continue;
                    }
                    if (adp.y > adp.z && adp.y > adp.x) {
                        // Y collision
                        if (dp.y < 0) {
                            if (!cd.top) {
                                voxelPosition.gridPosition.y += MIN_DISTANCE.y - adp.y;
                                if (physics.velocity.y < 0) physics.velocity.y = 0.0;
                                continue;
                            }
                        } else {
                            if (!cd.bottom) {
                                voxelPosition.gridPosition.y -= MIN_DISTANCE.y - adp.y;
                                if (physics.velocity.y > 0) physics.velocity.y = 0.0;
                                continue;
                            }
                        }
                    }
                    if (adp.z > adp.x) {
                        // Z collision
                        if (dp.z < 0) {
                            if (!cd.front) {
                                voxelPosition.gridPosition.z += MIN_DISTANCE.z - adp.z;
                                if (physics.velocity.z < 0) physics.velocity.z = 0.0;
                                continue;
                            }
                        } else {
                            if (!cd.back) {
                                voxelPosition.gridPosition.z -= MIN_DISTANCE.z - adp.z;
                                if (physics.velocity.z > 0) physics.velocity.z = 0.0;
                                continue;
                            }
                        }
                    }
                    // X collision
                    if (dp.x < 0) {
                        if (!cd.right) {
                            voxelPosition.gridPosition.x += MIN_DISTANCE.x - adp.x;
                            if (physics.velocity.x < 0) physics.velocity.x = 0.0;
                        }
                    } else {
                        if (!cd.left) {
                            voxelPosition.gridPosition.x -= MIN_DISTANCE.x - adp.x;
                            if (physics.velocity.x > 0) physics.velocity.x = 0.0;
                        }
                    }
                }
            }
        }
    }
}

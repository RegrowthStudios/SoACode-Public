#include "stdafx.h"
#include "PhysicsComponentUpdater.h"

#include "GameSystem.h"
#include "GameSystemAssemblages.h"
#include "SpaceSystem.h"
#include "TerrainPatch.h"
#include "VoxelSpaceConversions.h"
#include "VoxelSpaceUtils.h"
#include "global.h"
#include "soaUtils.h"

#include <Vorb/utils.h>

// TODO(Ben): This is temporary for gravity
#define FPS 60.0
// Exit is slightly bigger to prevent oscillation
#define ENTRY_RADIUS_MULT 1.05
#define EXIT_RADIUS_MULT 1.051

// TODO(Ben): Timestep
void PhysicsComponentUpdater::update(GameSystem* gameSystem, SpaceSystem* spaceSystem) {
  
    for (auto& it : gameSystem->physics) {
        auto& cmp = it.second;
        // Voxel position dictates space position
        if (cmp.voxelPositionComponent) {
            updateVoxelPhysics(gameSystem, spaceSystem, cmp, it.first);
        } else {
            updateSpacePhysics(gameSystem, spaceSystem, cmp, it.first);
        } 
    }
}

f64v3 PhysicsComponentUpdater::calculateGravityAcceleration(f64v3 relativePosition, f64 mass) {
    f64 dist2 = glm::dot(relativePosition, relativePosition); // Get distance^2
    relativePosition /= sqrt(dist2); // Normalize position
    f64 fgrav = M_G * mass / (dist2 * M_PER_KM * M_PER_KM); // Calculate force
    return relativePosition * ((fgrav / M_PER_KM) / FPS); // Return acceleration vector
}

void PhysicsComponentUpdater::updateVoxelPhysics(GameSystem* gameSystem, SpaceSystem* spaceSystem,
                                                 PhysicsComponent& pyCmp, vcore::EntityID entity) {


    // Get the position component
    auto& spCmp = gameSystem->spacePosition.get(pyCmp.spacePositionComponent);
    // Check for removal of spherical voxel component
    auto& stCmp = spaceSystem->m_sphericalTerrainCT.get(spCmp.parentSphericalTerrainId);
    if (stCmp.sphericalVoxelComponent == 0) {
        // We need to transition to space
        pyCmp.voxelPositionComponent = 0;
        // TODO(Ben): Orient this
        pyCmp.velocity = f64v3(0.0);
        GameSystemAssemblages::removeVoxelPosition(gameSystem, entity);
        return;
    }

    auto& vpcmp = gameSystem->voxelPosition.get(pyCmp.voxelPositionComponent);
    auto& svcmp = spaceSystem->m_sphericalVoxelCT.get(vpcmp.parentVoxelComponent);
    auto& npcmp = spaceSystem->m_namePositionCT.get(svcmp.namePositionComponent);
    auto& arcmp = spaceSystem->m_axisRotationCT.get(svcmp.axisRotationComponent);
    // Apply gravity
    if (spCmp.parentGravityId) {
        auto& gravCmp = spaceSystem->m_sphericalGravityCT.get(spCmp.parentGravityId);
        f64 height = (vpcmp.gridPosition.pos.y + svcmp.voxelRadius) * M_PER_VOXEL;
        f64 fgrav = M_G * gravCmp.mass / (height * height);
        // We don't account mass since we only calculate force on the object
        pyCmp.velocity.y -= (fgrav / M_PER_KM) / FPS;
    }

    // Update position
    vpcmp.gridPosition.pos += pyCmp.velocity;
    // Check transition to new face
    bool didTransition = false;
    if (vpcmp.gridPosition.pos.x < -svcmp.voxelRadius) {
        didTransition = true;
        std::cout << "-X transition";
        transitionNegX(vpcmp, pyCmp, svcmp.voxelRadius);
    } else if (vpcmp.gridPosition.pos.x > svcmp.voxelRadius) {
        didTransition = true;
        std::cout << "+X transition";
        transitionPosX(vpcmp, pyCmp, svcmp.voxelRadius);
    } else if (vpcmp.gridPosition.pos.z < -svcmp.voxelRadius) {
        didTransition = true;
        std::cout << "-Z transition";
        transitionNegZ(vpcmp, pyCmp, svcmp.voxelRadius);
    } else if (vpcmp.gridPosition.pos.z > svcmp.voxelRadius) {
        didTransition = true;
        std::cout << "+Z transition";
        transitionPosZ(vpcmp, pyCmp, svcmp.voxelRadius);
    }

    // Compute the relative space position and orientation from voxel position and orientation
    spCmp.position = arcmp.currentOrientation * VoxelSpaceConversions::voxelToWorld(vpcmp.gridPosition, svcmp.voxelRadius) * KM_PER_VOXEL;
    // TODO(Ben): This is expensive as fuck. Make sure you only do this for components that actually need it
    spCmp.orientation = arcmp.currentOrientation * VoxelSpaceUtils::calculateVoxelToSpaceQuat(vpcmp.gridPosition, svcmp.voxelRadius) * vpcmp.orientation;

    // Check transition to Space
    // TODO(Ben): This assumes a single player entity!
    if (spCmp.parentSphericalTerrainId) {
        auto& stCmp = spaceSystem->m_sphericalTerrainCT.get(spCmp.parentSphericalTerrainId);

        f64 distance = glm::length(spCmp.position);
        if (distance > stCmp.sphericalTerrainData->radius * EXIT_RADIUS_MULT) {
            if (stCmp.needsVoxelComponent) {
                stCmp.needsVoxelComponent = false;
                stCmp.alpha = 0.0f;
            }
        } else if (didTransition) {
            stCmp.transitionFace = vpcmp.gridPosition.face;
        }
    } else {
        // This really shouldnt happen
        std::cerr << "Missing parent spherical terrain ID in updateVoxelPhysics\n";
    }
}

void PhysicsComponentUpdater::updateSpacePhysics(GameSystem* gameSystem, SpaceSystem* spaceSystem,
                                                 PhysicsComponent& pyCmp, vcore::EntityID entity) {

    // Get the position component
    auto& spCmp = gameSystem->spacePosition.get(pyCmp.spacePositionComponent);
    // Apply gravity

    // TODO(Ben): Optimize and fix with timestep
    if (spCmp.parentGravityId) {
        auto& gravCmp = spaceSystem->m_sphericalGravityCT.get(spCmp.parentGravityId);
        pyCmp.velocity += calculateGravityAcceleration(-spCmp.position, gravCmp.mass);
    } else {
        // TODO(Ben): Check gravity on all planets? check transition to parent?
    }

    // Update position
    spCmp.position += pyCmp.velocity; // * timestep

    // Check transition to planet
    // TODO(Ben): This assumes a single player entity!
    if (spCmp.parentSphericalTerrainId) {
        auto& stCmp = spaceSystem->m_sphericalTerrainCT.get(spCmp.parentSphericalTerrainId);
        auto& npCmp = spaceSystem->m_namePositionCT.get(stCmp.namePositionComponent);

        f64 distance = glm::length(spCmp.position);
        if (distance <= stCmp.sphericalTerrainData->radius * ENTRY_RADIUS_MULT) {
            // Mark the terrain component as needing voxels
            if (!stCmp.needsVoxelComponent) {
                auto& arCmp = spaceSystem->m_axisRotationCT.getFromEntity(stCmp.axisRotationComponent);
                stCmp.startVoxelPosition = VoxelSpaceConversions::worldToVoxel(arCmp.invCurrentOrientation * spCmp.position * VOXELS_PER_KM,
                                                                               stCmp.sphericalTerrainData->radius * VOXELS_PER_KM);
                stCmp.needsVoxelComponent = true;
                stCmp.alpha = TERRAIN_DEC_START_ALPHA;
            } else if (!pyCmp.voxelPositionComponent && stCmp.sphericalVoxelComponent) { // Check if we need to create the voxelPosition component

                auto& arCmp = spaceSystem->m_axisRotationCT.getFromEntity(stCmp.axisRotationComponent);
                // Calculate voxel relative orientation
                f64q voxOrientation = glm::inverse(VoxelSpaceUtils::calculateVoxelToSpaceQuat(stCmp.startVoxelPosition,
                    stCmp.sphericalTerrainData->radius * VOXELS_PER_KM)) * arCmp.invCurrentOrientation * spCmp.orientation;

                // Make the voxel position component
                vcore::ComponentID vpid = GameSystemAssemblages::addVoxelPosition(gameSystem, entity,
                                                                                  stCmp.sphericalVoxelComponent,
                                                                                  voxOrientation,
                                                                                  stCmp.startVoxelPosition);
                pyCmp.voxelPositionComponent = vpid;

                // TODO(Ben): Calculate velocity change properly
                //pyCmp.velocity = voxOrientation * pyCmp.velocity;
                pyCmp.velocity = f64v3(0); // VOXELS_PER_KM;

                // Update dependencies for frustum
                gameSystem->frustum.getFromEntity(entity).voxelPositionComponent = vpid;
            }
        }
    }
}

#define VOXEL_PUSH CHUNK_WIDTH

void PhysicsComponentUpdater::transitionPosX(VoxelPositionComponent& vpCmp, PhysicsComponent& pyCmp, float voxelRadius) {
    // Push in by a chunk
    float rad = voxelRadius - VOXEL_PUSH;
    // We could use lookup tables for this, but this is easier
    switch (vpCmp.gridPosition.face) {
        case FACE_TOP:
            vpCmp.orientation = f64q(f64v3(0.0, -M_PI / 2.0, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_RIGHT;
            vpCmp.gridPosition.pos.x = -vpCmp.gridPosition.pos.z;
            vpCmp.gridPosition.pos.z = -rad;
            break;
        case FACE_LEFT:
            vpCmp.gridPosition.face = FACE_FRONT;
            vpCmp.gridPosition.pos.x = -rad;
            break;
        case FACE_RIGHT:
            vpCmp.gridPosition.face = FACE_BACK;
            vpCmp.gridPosition.pos.x = -rad;
            break;
        case FACE_FRONT:
            vpCmp.gridPosition.face = FACE_RIGHT;
            vpCmp.gridPosition.pos.x = -rad;
            break;
        case FACE_BACK:
            vpCmp.gridPosition.face = FACE_LEFT;
            vpCmp.gridPosition.pos.x = -rad;
            break;
        case FACE_BOTTOM:
            vpCmp.orientation = f64q(f64v3(0.0, M_PI / 2.0, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_RIGHT;
            vpCmp.gridPosition.pos.x = vpCmp.gridPosition.pos.z;
            vpCmp.gridPosition.pos.z = rad;
            break;
        default:
            std::cerr << "Invalid face in PhysicsComponentUpdater::transitionPosX\n";
            break;
    }
}

void PhysicsComponentUpdater::transitionNegX(VoxelPositionComponent& vpCmp, PhysicsComponent& pyCmp, float voxelRadius) {
    // Push in by a chunk
    float rad = voxelRadius - VOXEL_PUSH;
    // We could use lookup tables for this, but this is easier
    switch (vpCmp.gridPosition.face) {
        case FACE_TOP:
            vpCmp.orientation = f64q(f64v3(0.0, M_PI / 2.0, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_LEFT;
            vpCmp.gridPosition.pos.x = vpCmp.gridPosition.pos.z;
            vpCmp.gridPosition.pos.z = -rad;
            break;
        case FACE_LEFT:
            vpCmp.gridPosition.face = FACE_BACK;
            vpCmp.gridPosition.pos.x = rad;
            break;
        case FACE_RIGHT:
            vpCmp.gridPosition.face = FACE_FRONT;
            vpCmp.gridPosition.pos.x = rad;
            break;
        case FACE_FRONT:
            vpCmp.gridPosition.face = FACE_LEFT;
            vpCmp.gridPosition.pos.x = rad;
            break;
        case FACE_BACK:
            vpCmp.gridPosition.face = FACE_RIGHT;
            vpCmp.gridPosition.pos.x = rad;
            break;
        case FACE_BOTTOM:
            vpCmp.orientation = f64q(f64v3(0.0, -M_PI / 2.0, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_LEFT;
            vpCmp.gridPosition.pos.x = -vpCmp.gridPosition.pos.z;
            vpCmp.gridPosition.pos.z = rad;
            break;
        default:
            std::cerr << "Invalid face in PhysicsComponentUpdater::transitionPosX\n";
            break;
    }
}

void PhysicsComponentUpdater::transitionPosZ(VoxelPositionComponent& vpCmp, PhysicsComponent& pyCmp, float voxelRadius) {
    // Push in by a chunk
    float rad = voxelRadius - VOXEL_PUSH;
    // We could use lookup tables for this, but this is easier
    switch (vpCmp.gridPosition.face) {
        case FACE_TOP:
            vpCmp.gridPosition.face = FACE_FRONT;
            vpCmp.gridPosition.pos.z = -rad;
            break;
        case FACE_LEFT:
            vpCmp.orientation = f64q(f64v3(0.0, M_PI / 2.0, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_BOTTOM;
            vpCmp.gridPosition.pos.z = -vpCmp.gridPosition.pos.x;
            vpCmp.gridPosition.pos.x = -rad;
            break;
        case FACE_RIGHT:
            vpCmp.orientation = f64q(f64v3(0.0, -M_PI / 2.0, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_BOTTOM;
            vpCmp.gridPosition.pos.z = vpCmp.gridPosition.pos.x;
            vpCmp.gridPosition.pos.x = rad;
            break;
        case FACE_FRONT:
            vpCmp.gridPosition.face = FACE_BOTTOM;
            vpCmp.gridPosition.pos.z = -rad;
            break;
        case FACE_BACK:
            vpCmp.orientation = f64q(f64v3(0.0, M_PI, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_BOTTOM;
            vpCmp.gridPosition.pos.z = -rad;
            vpCmp.gridPosition.pos.x = -vpCmp.gridPosition.pos.x;
            break;
        case FACE_BOTTOM:
            vpCmp.orientation = f64q(f64v3(0.0, M_PI, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_BACK;
            vpCmp.gridPosition.pos.z = -rad;
            vpCmp.gridPosition.pos.x = -vpCmp.gridPosition.pos.x;
            break;
        default:
            std::cerr << "Invalid face in PhysicsComponentUpdater::transitionPosX\n";
            break;
    }
}

void PhysicsComponentUpdater::transitionNegZ(VoxelPositionComponent& vpCmp, PhysicsComponent& pyCmp, float voxelRadius) {
    // Push in by a chunk
    float rad = voxelRadius - VOXEL_PUSH;
    // We could use lookup tables for this, but this is easier
    switch (vpCmp.gridPosition.face) {
        case FACE_TOP:
            vpCmp.orientation = f64q(f64v3(0.0, M_PI, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_BACK;
            vpCmp.gridPosition.pos.z = -rad;
            vpCmp.gridPosition.pos.x = -vpCmp.gridPosition.pos.x;
            break;
        case FACE_LEFT:
            vpCmp.orientation = f64q(f64v3(0.0, -M_PI / 2.0, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_TOP;
            vpCmp.gridPosition.pos.z = vpCmp.gridPosition.pos.x;
            vpCmp.gridPosition.pos.x = -rad;
            break;
        case FACE_RIGHT:
            vpCmp.orientation = f64q(f64v3(0.0, M_PI / 2.0, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_TOP;
            vpCmp.gridPosition.pos.z = -vpCmp.gridPosition.pos.x;
            vpCmp.gridPosition.pos.x = rad;
            break;
        case FACE_FRONT:
            vpCmp.gridPosition.face = FACE_TOP;
            vpCmp.gridPosition.pos.z = rad;
            break;
        case FACE_BACK:
            vpCmp.orientation = f64q(f64v3(0.0, M_PI, 0.0)) * vpCmp.orientation;
            vpCmp.gridPosition.face = FACE_TOP;
            vpCmp.gridPosition.pos.z = -rad;
            vpCmp.gridPosition.pos.x = -vpCmp.gridPosition.pos.x;
            break;
        case FACE_BOTTOM:
            vpCmp.gridPosition.face = FACE_FRONT;
            vpCmp.gridPosition.pos.z = rad;
            break;
        default:
            std::cerr << "Invalid face in PhysicsComponentUpdater::transitionPosX\n";
            break;
    }
}

#undef VOXEL_PUSH
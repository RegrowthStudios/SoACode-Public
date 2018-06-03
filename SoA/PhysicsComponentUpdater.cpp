#include "stdafx.h"
#include "PhysicsComponentUpdater.h"

#include "GameSystem.h"
#include "GameSystemAssemblages.h"
#include "SpaceSystem.h"
#include "TerrainPatch.h"
#include "VoxelSpaceConversions.h"
#include "VoxelSpaceUtils.h"
#include "soaUtils.h"

#include <Vorb/utils.h>

// TODO(Ben): This is temporary for gravity
#define FPS 60.0
// Exit is slightly bigger to prevent oscillation
#define ENTRY_RADIUS_MULT 1.02
#define EXIT_RADIUS_MULT 1.0205

// TODO(Ben): Timestep
void PhysicsComponentUpdater::update(GameSystem* gameSystem, SpaceSystem* spaceSystem) {
  
    for (auto& it : gameSystem->physics) {
        auto& cmp = it.second;
        // Voxel position dictates space position
        if (cmp.voxelPosition) {
            updateVoxelPhysics(gameSystem, spaceSystem, cmp, it.first);
        } else {
            updateSpacePhysics(gameSystem, spaceSystem, cmp, it.first);
        } 
    }
}

f64v3 PhysicsComponentUpdater::calculateGravityAcceleration(f64v3 relativePosition, f64 mass) {
    f64 dist2 = vmath::dot(relativePosition, relativePosition); // Get distance^2
    relativePosition /= sqrt(dist2); // Normalize position
    f64 fgrav = M_G * mass / (dist2 * M_PER_KM * M_PER_KM); // Calculate force
    return relativePosition * ((fgrav / M_PER_KM) / FPS); // Return acceleration vector
}

// TODO(Ben): This is a clusterfuck
void PhysicsComponentUpdater::updateVoxelPhysics(GameSystem* gameSystem, SpaceSystem* spaceSystem,
                                                 PhysicsComponent& pyCmp, vecs::EntityID entity) {

    // Get the position component
    auto& spCmp = gameSystem->spacePosition.get(pyCmp.spacePosition);
    // Check for removal of spherical voxel component
    auto& stCmp = spaceSystem->sphericalTerrain.get(spCmp.parentSphericalTerrain);
    if (stCmp.sphericalVoxelComponent == 0) {
        // We need to transition to space
        pyCmp.voxelPosition = 0;
        // TODO(Ben): Orient this
        pyCmp.velocity = f64v3(0.0);
        GameSystemAssemblages::removeVoxelPosition(gameSystem, entity);
        GameSystemAssemblages::removeChunkSphere(gameSystem, entity);
        return;
    }

    auto& vpcmp = gameSystem->voxelPosition.get(pyCmp.voxelPosition);
    auto& svcmp = spaceSystem->sphericalVoxel.get(vpcmp.parentVoxel);
    auto& npcmp = spaceSystem->namePosition.get(svcmp.namePositionComponent);
    auto& arcmp = spaceSystem->axisRotation.get(svcmp.axisRotationComponent);

    // Apply gravity
    if (spCmp.parentGravity) {
        auto& gravCmp = spaceSystem->sphericalGravity.get(spCmp.parentGravity);
        f64 height = (vpcmp.gridPosition.pos.y + svcmp.voxelRadius) * M_PER_VOXEL;
        f64 fgrav = M_G * gravCmp.mass / (height * height);
        ChunkID id(VoxelSpaceConversions::voxelToChunk(vpcmp.gridPosition));
        ChunkHandle chunk = svcmp.chunkGrids[vpcmp.gridPosition.face].accessor.acquire(id);
        // Don't apply gravity in non generated chunks.
        if (chunk->genLevel == GEN_DONE) {
            pyCmp.velocity.y -= (fgrav * 0.1) / FPS;
        }
        chunk.release();
    }
    // Update position
    vpcmp.gridPosition.pos += pyCmp.velocity;
    // Store old position in case of transition
    VoxelPositionComponent oldVPCmp = vpcmp;
    SpacePositionComponent oldSPCmp = spCmp;

    // Check transition to new face
    bool didTransition = false;
    if (vpcmp.gridPosition.pos.x < -svcmp.voxelRadius) {
        didTransition = true;
        transitionNegX(vpcmp, pyCmp, (f32)svcmp.voxelRadius);
    } else if (vpcmp.gridPosition.pos.x > svcmp.voxelRadius) {
        didTransition = true;
        transitionPosX(vpcmp, pyCmp, (f32)svcmp.voxelRadius);
    } else if (vpcmp.gridPosition.pos.z < -svcmp.voxelRadius) {
        didTransition = true;
        transitionNegZ(vpcmp, pyCmp, (f32)svcmp.voxelRadius);
    } else if (vpcmp.gridPosition.pos.z > svcmp.voxelRadius) {
        didTransition = true;
        transitionPosZ(vpcmp, pyCmp, (f32)svcmp.voxelRadius);
    }

    // Compute the relative space position and orientation from voxel position and orientation
    spCmp.position = arcmp.currentOrientation * VoxelSpaceConversions::voxelToWorld(vpcmp.gridPosition, svcmp.voxelRadius) * KM_PER_VOXEL;

    // TODO(Ben): This is expensive as fuck. Make sure you only do this for components that actually need it
    spCmp.orientation = arcmp.currentOrientation * VoxelSpaceUtils::calculateVoxelToSpaceQuat(vpcmp.gridPosition, svcmp.voxelRadius) * vpcmp.orientation;

    // Check transitions
    // TODO(Ben): This assumes a single player entity!
    if (spCmp.parentSphericalTerrain) {
        auto& stCmp = spaceSystem->sphericalTerrain.get(spCmp.parentSphericalTerrain);

        f64 distance = vmath::length(spCmp.position);
        // Check transition to Space
        if (distance > stCmp.sphericalTerrainData->radius * EXIT_RADIUS_MULT) {
            if (stCmp.needsVoxelComponent) {
                stCmp.needsVoxelComponent = false;
                // Begin fade transition
                stCmp.alpha = TERRAIN_INC_START_ALPHA;
            }
        } else if (didTransition) { // Check face transition
            // Check face-to-face transition cases
            if (stCmp.transitionFace == FACE_NONE && !stCmp.isFaceTransitioning) {
                stCmp.transitionFace = vpcmp.gridPosition.face;
                stCmp.isFaceTransitioning = true;
                vpcmp = oldVPCmp;
                spCmp = oldSPCmp;
            } else if (stCmp.isFaceTransitioning) {
                // Check for transition end
                if (stCmp.faceTransTime < 0.2f) {
                    stCmp.isFaceTransitioning = false;
                } else {
                    vpcmp = oldVPCmp;
                    spCmp = oldSPCmp;
                }
            }
        }
    } else {
        // This really shouldn't happen
        std::cerr << "Missing parent spherical terrain ID in updateVoxelPhysics\n";
    }
}

void PhysicsComponentUpdater::updateSpacePhysics(GameSystem* gameSystem, SpaceSystem* spaceSystem,
                                                 PhysicsComponent& pyCmp, vecs::EntityID entity) {

    // Get the position component
    auto& spCmp = gameSystem->spacePosition.get(pyCmp.spacePosition);

    // Apply gravity
    // TODO(Ben): Optimize and fix with timestep
    if (spCmp.parentGravity) {
        auto& gravCmp = spaceSystem->sphericalGravity.get(spCmp.parentGravity);
        pyCmp.velocity += calculateGravityAcceleration(-spCmp.position, gravCmp.mass);
    } else {
        // TODO(Ben): Check gravity on all planets? check transition to parent?
    }

    // Update position
    spCmp.position += pyCmp.velocity; // * timestep

    // Check transition to planet
    // TODO(Ben): This assumes a single player entity!
    if (spCmp.parentSphericalTerrain) {
        auto& stCmp = spaceSystem->sphericalTerrain.get(spCmp.parentSphericalTerrain);
        auto& npCmp = spaceSystem->namePosition.get(stCmp.namePositionComponent);

        f64 distance = vmath::length(spCmp.position);
        if (distance <= stCmp.sphericalTerrainData->radius * ENTRY_RADIUS_MULT) {
            // Mark the terrain component as needing voxels
            if (!stCmp.needsVoxelComponent) {
                auto& arCmp = spaceSystem->axisRotation.getFromEntity(stCmp.axisRotationComponent);
                stCmp.startVoxelPosition = VoxelSpaceConversions::worldToVoxel(arCmp.invCurrentOrientation * spCmp.position * VOXELS_PER_KM,
                                                                               stCmp.sphericalTerrainData->radius * VOXELS_PER_KM);
                stCmp.needsVoxelComponent = true;
                // Start the fade transition
                stCmp.alpha = TERRAIN_DEC_START_ALPHA;
            } else if (!pyCmp.voxelPosition && stCmp.sphericalVoxelComponent) { // Check if we need to create the voxelPosition component

                auto& arCmp = spaceSystem->axisRotation.getFromEntity(stCmp.axisRotationComponent);
                // Calculate voxel relative orientation
                f64q voxOrientation = vmath::inverse(VoxelSpaceUtils::calculateVoxelToSpaceQuat(stCmp.startVoxelPosition,
                    stCmp.sphericalTerrainData->radius * VOXELS_PER_KM)) * arCmp.invCurrentOrientation * spCmp.orientation;

                // Make the voxel position component
                vecs::ComponentID vpid = GameSystemAssemblages::addVoxelPosition(gameSystem, entity,
                                                                                  stCmp.sphericalVoxelComponent,
                                                                                  voxOrientation,
                                                                                  stCmp.startVoxelPosition);
                
                // Make the Chunk Sphere component
                GameSystemAssemblages::addChunkSphere(gameSystem, entity, vpid, VoxelSpaceConversions::voxelToChunk(stCmp.startVoxelPosition), 7);
                
                auto& hcmp = gameSystem->head.getFromEntity(entity);
                hcmp.voxelPosition = vpid;
                pyCmp.voxelPosition = vpid;

                // TODO(Ben): Calculate velocity change properly
                //pyCmp.velocity = voxOrientation * pyCmp.velocity;
                pyCmp.velocity = f64v3(0); // VOXELS_PER_KM;

                // Update dependencies for frustum
                gameSystem->frustum.getFromEntity(entity).voxelPosition = vpid;
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
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

    // Compute the relative space position and orientation from voxel position and orientation
    spCmp.position = arcmp.currentOrientation * VoxelSpaceConversions::voxelToWorld(vpcmp.gridPosition, svcmp.voxelRadius) * KM_PER_VOXEL;
    // TODO(Ben): This is expensive as fuck. Make sure you only do this for components that actually need it
    spCmp.orientation = arcmp.currentOrientation * VoxelSpaceUtils::calculateVoxelToSpaceQuat(vpcmp.gridPosition, svcmp.voxelRadius) * vpcmp.orientation;

    // Check transition to Space
    // TODO(Ben): This assumes a single player entity!
    if (spCmp.parentSphericalTerrainId) {
        auto& stCmp = spaceSystem->m_sphericalTerrainCT.get(spCmp.parentSphericalTerrainId);

        f64 distance = glm::length(spCmp.position);
        if (distance > stCmp.sphericalTerrainData->radius * EXIT_RADIUS_MULT && stCmp.needsVoxelComponent) {
            stCmp.needsVoxelComponent = false;
            stCmp.alpha = 0.0f;
        }
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

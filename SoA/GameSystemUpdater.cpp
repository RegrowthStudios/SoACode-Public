#include "stdafx.h"
#include "GameSystemUpdater.h"

#include "GameSystem.h"
#include "SpaceSystem.h"
#include "SphericalTerrainPatch.h"
#include <Vorb/IntersectionUtils.inl>


void GameSystemUpdater::update(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem) {
    // Update entity tables
    updatePhysics(gameSystem, spaceSystem);
    updateCollision(gameSystem);
    updateMoveInput(gameSystem);

    // Update voxel planet transitions every 60 frames
    m_frameCounter++;
    if (m_frameCounter == 60) {
        updateVoxelPlanetTransitions(gameSystem, spaceSystem);
    }
}

void GameSystemUpdater::updateVoxelPlanetTransitions(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem) {
#define LOAD_DIST_MULT 1.05
    for (auto& it : gameSystem->spacePositionCT) {
        auto& spcmp = it.second;
        for (auto& sit : spaceSystem->m_sphericalTerrainCT) {
            auto& stcmp = sit.second;
            auto& npcmp = spaceSystem->m_namePositionCT.get(stcmp.namePositionComponent);
            // Calculate distance
            // TODO(Ben): Use ^2 distance as optimization to avoid sqrt
            f64v3 relPos = spcmp.position - npcmp.position;
            f64 distance = glm::length(spcmp.position - npcmp.position);
            // Check for voxel transition
            if (distance < stcmp.sphericalTerrainData->getRadius() * LOAD_DIST_MULT && !spcmp.voxelPositionComponent) {
                // We need to transition to the voxels
                vcore::ComponentID vpid = gameSystem->voxelPositionCT.add(it.first);
                auto& vpcmp = gameSystem->voxelPositionCT.get(vpid);
                spcmp.voxelPositionComponent = vpid;
                // Calculate voxel position
                auto& rotcmp = spaceSystem->m_axisRotationCT.getFromEntity(sit.first);
                computeVoxelPosition(relPos, (f32)stcmp.sphericalTerrainData->getRadius(), vpcmp);
            } else if (spcmp.voxelPositionComponent) {
                // We need to transition to space
                gameSystem->voxelPositionCT.remove(it.first);
                spcmp.voxelPositionComponent = 0;
                // TODO(Ben): Refcount SVC
            }
        }
    }
    m_frameCounter = 0;
}

// TODO(Ben): Timestep
void GameSystemUpdater::updatePhysics(OUT GameSystem* gameSystem, const SpaceSystem* spaceSystem) {
    for (auto& it : gameSystem->physicsCT) {
        auto& cmp = it.second;
        // Get the position component
        auto& spcmp = gameSystem->spacePositionCT.get(cmp.spacePositionComponent);
        // Voxel position dictates space position
        if (cmp.voxelPositionComponent) {
            auto& vpcmp = gameSystem->voxelPositionCT.get(cmp.voxelPositionComponent);
            vpcmp.position += cmp.velocity;
            // TODO(Ben): Compute spcmp.position from vpcmp.position
        } else {
            spcmp.position += cmp.velocity; // * timestep
        }
    }
}

void GameSystemUpdater::updateCollision(OUT GameSystem* gameSystem) {
    for (auto& it : gameSystem->aabbCollidableCT) {
        //TODO(Ben): this
    }
}

void GameSystemUpdater::updateMoveInput(OUT GameSystem* gameSystem) {
    for (auto& it : gameSystem->moveInputCT) {

    }
}

void GameSystemUpdater::computeVoxelPosition(const f64v3& relPos, f32 radius, OUT VoxelPositionComponent& vpcmp) {
    f32v3 cornerPos[2];
    float min;
    f32v3 start = f32v3(glm::normalize(relPos) * 2.0);
    f32v3 dir = f32v3(-glm::normalize(relPos));
    cornerPos[0] = f32v3(-radius, -radius, -radius);
    cornerPos[1] = f32v3(radius, radius, radius);
    if (!IntersectionUtils::boxIntersect(cornerPos, dir,
        start, min)) {
        std::cerr << "Failed to find grid position\n";
    }

    f32v3 gridHit = start + dir * min;
    const float eps = 0.01;

    vpcmp.mapData.rotation = 0;
    if (abs(gridHit.x - (-radius)) < eps) { //-X
        vpcmp.mapData.face = (int)CubeFace::LEFT;
        vpcmp.mapData.ipos = gridHit.z;
        vpcmp.mapData.jpos = gridHit.y;
    } else if (abs(gridHit.x - radius) < eps) { //X
        vpcmp.mapData.face = (int)CubeFace::RIGHT;
        vpcmp.mapData.ipos = gridHit.z;
        vpcmp.mapData.jpos = gridHit.y;
    } else if (abs(gridHit.y - (-radius)) < eps) { //-Y
        vpcmp.mapData.face = (int)CubeFace::BOTTOM;
        vpcmp.mapData.ipos = gridHit.x;
        vpcmp.mapData.jpos = gridHit.z;
    } else if (abs(gridHit.y - radius) < eps) { //Y
        vpcmp.mapData.face = (int)CubeFace::TOP;
        vpcmp.mapData.ipos = gridHit.x;
        vpcmp.mapData.jpos = gridHit.z;
    } else if (abs(gridHit.z - (-radius)) < eps) { //-Z
        vpcmp.mapData.face = (int)CubeFace::BACK;
        vpcmp.mapData.ipos = gridHit.x;
        vpcmp.mapData.jpos = gridHit.y;
    } else if (abs(gridHit.z - radius) < eps) { //Z
        vpcmp.mapData.face = (int)CubeFace::FRONT;
        vpcmp.mapData.ipos = gridHit.x;
        vpcmp.mapData.jpos = gridHit.y;
    }
}
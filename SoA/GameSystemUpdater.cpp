#include "stdafx.h"
#include "GameSystemUpdater.h"

#include "GameSystem.h"
#include "GameSystemFactories.h"
#include "InputManager.h"
#include "Inputs.h"
#include "SpaceSystem.h"
#include "SphericalTerrainPatch.h"

#include "GameSystemEvents.hpp"

#include <Vorb/FastConversion.inl>
#include <Vorb/IntersectionUtils.inl>

GameSystemUpdater::GameSystemUpdater(OUT GameSystem* gameSystem, InputManager* inputManager) {

    // Hook up wasd events
    m_onForwardKeyDown = inputManager->subscribe(INPUT_FORWARD, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnForwardKeyDown(gameSystem));
    m_onForwardKeyUp = inputManager->subscribe(INPUT_FORWARD, InputManager::EventType::UP, (IDelegate<ui32>*)new OnForwardKeyUp(gameSystem));
    m_onLeftKeyDown = inputManager->subscribe(INPUT_LEFT, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnLeftKeyDown(gameSystem));
    m_onLeftKeyUp = inputManager->subscribe(INPUT_LEFT, InputManager::EventType::UP, (IDelegate<ui32>*)new OnLeftKeyUp(gameSystem));
    m_onRightKeyDown = inputManager->subscribe(INPUT_RIGHT, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnRightKeyDown(gameSystem));
    m_onRightKeyUp = inputManager->subscribe(INPUT_RIGHT, InputManager::EventType::UP, (IDelegate<ui32>*)new OnRightKeyUp(gameSystem));
    m_onBackwardKeyDown = inputManager->subscribe(INPUT_BACKWARD, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnBackwardKeyDown(gameSystem));
    m_onBackwardKeyUp = inputManager->subscribe(INPUT_BACKWARD, InputManager::EventType::UP, (IDelegate<ui32>*)new OnBackwardKeyUp(gameSystem));
}

void GameSystemUpdater::update(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem) {
    // Update entity tables
    physicsUpdater.update(gameSystem, spaceSystem);
    collisionUpdater.update(gameSystem);
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
                
                // Calculate voxel position
                auto& rotcmp = spaceSystem->m_axisRotationCT.getFromEntity(sit.first);
                vvox::VoxelPlanetMapData mapData;
                f64v3 pos;
                computeVoxelPosition(rotcmp.invCurrentOrientation * relPos, (f32)stcmp.sphericalTerrainData->getRadius(), mapData, pos);

                // We need to transition to the voxels
                vcore::ComponentID vpid = GameSystemFactories::addVoxelPosition(gameSystem, it.first, pos, f64q(), mapData);

                spcmp.voxelPositionComponent = vpid;
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

void GameSystemUpdater::updateMoveInput(OUT GameSystem* gameSystem) {
  //  for (auto& it : gameSystem->moveInputCT) {

  //  }
}

void GameSystemUpdater::computeVoxelPosition(const f64v3& relPos, f32 radius, OUT vvox::VoxelPlanetMapData& mapData, OUT f64v3& pos) {
#define VOXELS_PER_KM 2000.0

    f64v3 voxelRelPos = relPos * VOXELS_PER_KM;
    f32 voxelRadius = (float)(radius * VOXELS_PER_KM);

    f32v3 cornerPos[2];
    float min;
    float distance = (float)glm::length(voxelRelPos);
    f32v3 start = f32v3(glm::normalize(voxelRelPos) * 2.0);
    f32v3 dir = f32v3(-glm::normalize(voxelRelPos));
    cornerPos[0] = f32v3(-voxelRadius, -voxelRadius, -voxelRadius);
    cornerPos[1] = f32v3(voxelRadius, voxelRadius, voxelRadius);
    if (!IntersectionUtils::boxIntersect(cornerPos, dir,
        start, min)) {
        std::cerr << "Failed to find grid position\n";
    }

    f32v3 gridHit = start + dir * min;
    const float eps = 0.01;

    mapData.rotation = 0;
    if (abs(gridHit.x - (-voxelRadius)) < eps) { //-X
        mapData.face = (int)CubeFace::LEFT;
        pos.z = gridHit.z;
        pos.x = gridHit.y;
    } else if (abs(gridHit.x - voxelRadius) < eps) { //X
        mapData.face = (int)CubeFace::RIGHT;
        pos.z = gridHit.z;
        pos.x = gridHit.y;
    } else if (abs(gridHit.y - (-voxelRadius)) < eps) { //-Y
        mapData.face = (int)CubeFace::BOTTOM;
        pos.z = gridHit.x;
        pos.x = gridHit.z;
    } else if (abs(gridHit.y - voxelRadius) < eps) { //Y
        mapData.face = (int)CubeFace::TOP;
        pos.z = gridHit.x;
        pos.x = gridHit.z;
    } else if (abs(gridHit.z - (-voxelRadius)) < eps) { //-Z
        mapData.face = (int)CubeFace::BACK;
        pos.z = gridHit.x;
        pos.x = gridHit.y;
    } else if (abs(gridHit.z - voxelRadius) < eps) { //Z
        mapData.face = (int)CubeFace::FRONT;
        pos.z = gridHit.x;
        pos.x = gridHit.y;
    }
    pos.y = distance - voxelRadius;
    mapData.ipos = fastFloor(gridHit.x / (float)CHUNK_WIDTH);
    mapData.jpos = fastFloor(gridHit.y / (float)CHUNK_WIDTH);
}
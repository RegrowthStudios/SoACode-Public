#include "stdafx.h"
#include "GameSystemUpdater.h"

#include "FreeMoveComponentUpdater.h"
#include "GameSystem.h"
#include "GameSystemEvents.hpp"
#include "GameSystemAssemblages.h"
#include "InputManager.h"
#include "Inputs.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SpaceSystemAssemblages.h"
#include "SphericalTerrainPatch.h"
#include "VoxelSpaceUtils.h"
#include "VoxelSpaceConversions.h"

#include <Vorb/utils.h>
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
   
    m_onUpKeyDown = inputManager->subscribe(INPUT_JUMP, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnUpKeyDown(gameSystem));
    m_onUpKeyUp = inputManager->subscribe(INPUT_JUMP, InputManager::EventType::UP, (IDelegate<ui32>*)new OnUpKeyUp(gameSystem));
    m_onDownKeyDown = inputManager->subscribe(INPUT_CROUCH, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnDownKeyDown(gameSystem));
    m_onDownKeyUp = inputManager->subscribe(INPUT_CROUCH, InputManager::EventType::UP, (IDelegate<ui32>*)new OnDownKeyUp(gameSystem));

    m_onLeftRollKeyDown = inputManager->subscribe(INPUT_LEFT_ROLL, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnLeftRollKeyDown(gameSystem));
    m_onLeftRollKeyUp = inputManager->subscribe(INPUT_LEFT_ROLL, InputManager::EventType::UP, (IDelegate<ui32>*)new OnLeftRollKeyUp(gameSystem));
    m_onRightRollKeyDown = inputManager->subscribe(INPUT_RIGHT_ROLL, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnRightRollKeyDown(gameSystem));
    m_onRightRollKeyUp = inputManager->subscribe(INPUT_RIGHT_ROLL, InputManager::EventType::UP, (IDelegate<ui32>*)new OnRightRollKeyUp(gameSystem));

    m_onSuperSpeedKeyDown = inputManager->subscribe(INPUT_MEGA_SPEED, InputManager::EventType::DOWN, (IDelegate<ui32>*)new OnSuperSpeedKeyDown(gameSystem));
    m_onSuperSpeedKeyUp = inputManager->subscribe(INPUT_MEGA_SPEED, InputManager::EventType::UP, (IDelegate<ui32>*)new OnSuperSpeedKeyUp(gameSystem));

    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onMotion, [=](Sender s, const vui::MouseMotionEvent& e) {
        for (auto& it : gameSystem->freeMoveInput) {
            FreeMoveComponentUpdater::rotateFromMouse(gameSystem, it.second, -e.dx, e.dy, 0.1f);
        }
    });
}

void GameSystemUpdater::update(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem, const SoaState* soaState) {

#define CHECK_FRAMES 6

    // Update entity tables
    physicsUpdater.update(gameSystem, spaceSystem);
    collisionUpdater.update(gameSystem);
    freeMoveUpdater.update(gameSystem);
    frustumUpdater.update(gameSystem);

    // Update voxel planet transitions every CHECK_FRAMES frames
    m_frameCounter++;
    if (m_frameCounter == CHECK_FRAMES) {
        updateVoxelPlanetTransitions(gameSystem, spaceSystem, soaState);
        m_frameCounter = 0;
    }
}

void GameSystemUpdater::updateVoxelPlanetTransitions(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem, const SoaState* soaState) {
#define LOAD_DIST_MULT 1.05
    for (auto& it : gameSystem->spacePosition) {
        bool inVoxelRange = false;
        auto& spcmp = it.second;
        auto& pycmp = gameSystem->physics.getFromEntity(it.first);
        for (auto& sit : spaceSystem->m_sphericalTerrainCT) {
            auto& stcmp = sit.second;
            auto& npcmp = spaceSystem->m_namePositionCT.get(stcmp.namePositionComponent);
            // Calculate distance
            // TODO(Ben): Use ^2 distance as optimization to avoid sqrt
            f64v3 relPos = spcmp.position - npcmp.position;
            f64 distance = glm::length(spcmp.position - npcmp.position);
            // Check for voxel transition
            if (distance < stcmp.sphericalTerrainData->getRadius() * LOAD_DIST_MULT) {
                inVoxelRange = true;
                if (!pycmp.voxelPositionComponent) {
                    // We need to transition to a voxel component
                    // Calculate voxel position
                    auto& rotcmp = spaceSystem->m_axisRotationCT.getFromEntity(sit.first);
                    ChunkPosition2D chunkGridPos;
                    VoxelPosition3D vGridPos;
                    computeVoxelPosition(rotcmp.invCurrentOrientation * relPos, (f32)stcmp.sphericalTerrainData->getRadius(), chunkGridPos, vGridPos.pos);
                    vGridPos.face = chunkGridPos.face;

                    // Check for the spherical voxel component
                    vcore::ComponentID svid = spaceSystem->m_sphericalVoxelCT.getComponentID(sit.first);
                    // For now, add and remove SphericalVoxel and FarTerrain component together
                    if (svid == 0) {
                        svid = SpaceSystemAssemblages::addSphericalVoxelComponent(spaceSystem, sit.first,
                                                                                spaceSystem->m_sphericalTerrainCT.getComponentID(sit.first),
                                                                                chunkGridPos, vGridPos.pos, soaState);
                        SpaceSystemAssemblages::addFarTerrainComponent(spaceSystem, sit.first,
                                                                       &stcmp);
                    } else {
                        spaceSystem->m_sphericalVoxelCT.get(svid).refCount++;
                    }

                    f64q voxOrientation = glm::inverse(VoxelSpaceUtils::calculateVoxelToSpaceQuat(VoxelSpaceConversions::chunkToVoxel(chunkGridPos),
                        stcmp.sphericalTerrainData->getRadius() * 2000.0)) * rotcmp.invCurrentOrientation * spcmp.orientation;

                    // We need to transition to the voxels
                    vcore::ComponentID vpid = GameSystemAssemblages::addVoxelPosition(gameSystem, it.first, svid, voxOrientation, vGridPos);
                    pycmp.voxelPositionComponent = vpid;
                    
                    // Update dependencies for frustum
                    gameSystem->frustum.getFromEntity(it.first).voxelPositionComponent = vpid;
                }
            }
        }
        // If we are in range of no planets, delete voxel position component
        if (!inVoxelRange && pycmp.voxelPositionComponent) {
            // We need to transition to space
            gameSystem->deleteComponent("VoxelPosition", it.first);
            pycmp.voxelPositionComponent = 0;
            
            vcore::ComponentID svid = spaceSystem->m_sphericalVoxelCT.getComponentID(it.first);
            auto& svcmp = spaceSystem->m_sphericalVoxelCT.get(svid);
            svcmp.refCount--;
            if (svcmp.refCount == 0) {
                spaceSystem->deleteComponent("SphericalVoxel", it.first);
            }

            // Update dependencies for frustum
            gameSystem->frustum.getFromEntity(it.first).voxelPositionComponent = 0;
        }
    }
}

void GameSystemUpdater::computeVoxelPosition(const f64v3& relPos, f32 radius, OUT ChunkPosition2D& gridPos, OUT f64v3& pos) {
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
        throw 3252;
    }

    f32v3 gridHit = start + dir * min;
    const float eps = 2.0f;

    if (abs(gridHit.x - (-voxelRadius)) < eps) { //-X
        gridPos.face = WorldCubeFace::FACE_LEFT;
        pos.z = -gridHit.y;
        pos.x = gridHit.z;
    } else if (abs(gridHit.x - voxelRadius) < eps) { //X
        gridPos.face = WorldCubeFace::FACE_RIGHT;
        pos.z = -gridHit.y;
        pos.x = -gridHit.z;
    } else if (abs(gridHit.y - (-voxelRadius)) < eps) { //-Y
        gridPos.face = WorldCubeFace::FACE_BOTTOM;
        pos.z = -gridHit.z;
        pos.x = gridHit.x;
    } else if (abs(gridHit.y - voxelRadius) < eps) { //Y
        gridPos.face = WorldCubeFace::FACE_TOP;
        pos.z = gridHit.z;
        pos.x = gridHit.x;
    } else if (abs(gridHit.z - (-voxelRadius)) < eps) { //-Z
        gridPos.face = WorldCubeFace::FACE_BACK;
        pos.z = -gridHit.y;
        pos.x = -gridHit.x;
    } else if (abs(gridHit.z - voxelRadius) < eps) { //Z
        gridPos.face = WorldCubeFace::FACE_FRONT;
        pos.z = -gridHit.y;
        pos.x = gridHit.x;
    } else {
        std::cerr << "ERROR: FAILED TO FIND A FACE ON SPACE -> WORLD TRANSITION";
        throw 44352;
    }
    pos.y = distance - voxelRadius;
    gridPos.pos = f64v2(fastFloor(pos.x / (float)CHUNK_WIDTH),
                        fastFloor(pos.z / (float)CHUNK_WIDTH));
}
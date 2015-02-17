#include "stdafx.h"
#include "GameSystemUpdater.h"

#include "FreeMoveComponentUpdater.h"
#include "GameSystem.h"
#include "GameSystemAssemblages.h"
#include "GameSystemEvents.hpp"
#include "InputManager.h"
#include "Inputs.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SpaceSystemAssemblages.h"
#include "SphericalTerrainGpuGenerator.h"
#include "TerrainPatch.h"
#include "VoxelSpaceConversions.h"
#include "VoxelSpaceUtils.h"

#include <Vorb/utils.h>
#include <Vorb/IntersectionUtils.inl>

#define KM_PER_VOXEL 0.0005
#define VOXELS_PER_KM 2000.0

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
    m_freeMoveUpdater.update(gameSystem);
    m_physicsUpdater.update(gameSystem, spaceSystem);
    m_collisionUpdater.update(gameSystem);
    m_frustumUpdater.update(gameSystem);

    // Update voxel planet transitions every CHECK_FRAMES frames
    m_frameCounter++;
    if (m_frameCounter == CHECK_FRAMES) {
        updateVoxelPlanetTransitions(gameSystem, spaceSystem, soaState);
        m_frameCounter = 0;
    }
}

void GameSystemUpdater::updateVoxelPlanetTransitions(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem, const SoaState* soaState) {
#define LOAD_DIST_MULT 1.05
    // TODO(Ben): There is some client specific stuff mixed in with server stuff.
    // TODO(Ben): This is n^2 and therefore expensive
    for (auto& it : gameSystem->spacePosition) {
        bool inVoxelRange = false;
        auto& spcmp = it.second;
        auto& pycmp = gameSystem->physics.getFromEntity(it.first);
        for (auto& sit : spaceSystem->m_sphericalTerrainCT) {
            if (!sit.second.gpuGenerator) continue; /// Have to check for deleted component
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
                    VoxelPosition3D vGridPos = VoxelSpaceConversions::worldToVoxel(rotcmp.invCurrentOrientation * relPos * VOXELS_PER_KM, stcmp.sphericalTerrainData->getRadius() * VOXELS_PER_KM);
                    std::cout << (int)vGridPos.face << std::endl;
                    // Check for the spherical voxel component
                    vcore::ComponentID svid = spaceSystem->m_sphericalVoxelCT.getComponentID(sit.first);
                    // For now, add and remove SphericalVoxel and FarTerrain component together
                    if (svid == 0) {
                        // TODO(Ben): FarTerrain should be clientSide only
                        auto ftCmpId = SpaceSystemAssemblages::addFarTerrainComponent(spaceSystem, sit.first, sit.second,
                                                                       vGridPos.face);
                        
                        svid = SpaceSystemAssemblages::addSphericalVoxelComponent(spaceSystem, sit.first,
                                                                                  spaceSystem->m_sphericalTerrainCT.getComponentID(sit.first),
                                                                                  ftCmpId,
                                                                                  sit.second.axisRotationComponent, sit.second.namePositionComponent,
                                                                                  vGridPos, soaState);
                      
                        sit.second.active = false;
                    } else {
                        spaceSystem->m_sphericalVoxelCT.get(svid).refCount++;
                    }

                    f64q voxOrientation = glm::inverse(VoxelSpaceUtils::calculateVoxelToSpaceQuat(vGridPos,
                        stcmp.sphericalTerrainData->getRadius() * 2000.0)) * rotcmp.invCurrentOrientation * spcmp.orientation;

                    // We need to transition to the voxels
                    vcore::ComponentID vpid = GameSystemAssemblages::addVoxelPosition(gameSystem, it.first, svid, voxOrientation, vGridPos);
                    pycmp.voxelPositionComponent = vpid;
                    
                    // Update dependencies for frustum
                    gameSystem->frustum.getFromEntity(it.first).voxelPositionComponent = vpid;
                }
            } else {
                sit.second.active = true;
            }
        }

        // If we are in range of no planets, delete voxel position component
        if (!inVoxelRange && pycmp.voxelPositionComponent) {

            // Get voxel position handle
            auto& vpCmp = gameSystem->voxelPosition.get(pycmp.voxelPositionComponent);
            // Get parent spherical voxel
            auto& svcmp = spaceSystem->m_sphericalVoxelCT.get(vpCmp.parentVoxelComponent);
            // Hacky search for entity....
            vcore::EntityID planetEntity;
            for (auto& sit = spaceSystem->m_sphericalVoxelCT.begin(); sit != spaceSystem->m_sphericalVoxelCT.end(); sit++) {
                if (sit->second.namePositionComponent == svcmp.namePositionComponent) {
                    planetEntity = sit->first;
                    break;
                }
            }

            svcmp.refCount--;
            if (svcmp.refCount == 0) {
                SpaceSystemAssemblages::removeSphericalVoxelComponent(spaceSystem, planetEntity);
                SpaceSystemAssemblages::removeFarTerrainComponent(spaceSystem, planetEntity);

                auto& stCmp = spaceSystem->m_sphericalTerrainCT.getFromEntity(planetEntity);
                stCmp.active = true;
            }

            // Delete the voxelPositionComponent
            pycmp.voxelPositionComponent = 0;
            gameSystem->deleteComponent("VoxelPosition", it.first);

            // Update dependencies for frustum
            gameSystem->frustum.getFromEntity(it.first).voxelPositionComponent = 0;
        }
    }
}
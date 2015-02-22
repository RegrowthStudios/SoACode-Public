#include "stdafx.h"
#include "GameSystemUpdater.h"

#include "FreeMoveComponentUpdater.h"
#include "GameSystem.h"
#include "GameSystemAssemblages.h"
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

GameSystemUpdater::GameSystemUpdater(OUT SoaState* soaState, InputManager* inputManager) :
    m_soaState(soaState),
    m_inputManager(inputManager) {

    GameSystem* gameSystem = soaState->gameSystem.get();
    // Forward event
    addEvent(INPUT_FORWARD, InputManager::EventType::DOWN, [=](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveForward = true;
        }
    });
    addEvent(INPUT_FORWARD, InputManager::EventType::UP, [=](Sender s, ui32 a) -> void {
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveForward = false;
        }
    });
    // Left event
    addEvent(INPUT_LEFT, InputManager::EventType::DOWN, [=](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveLeft = true;
        }
    });
    addEvent(INPUT_LEFT, InputManager::EventType::UP, [=](Sender s, ui32 a) -> void {
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveLeft = false;
        }
    });
    // Right event
    addEvent(INPUT_RIGHT, InputManager::EventType::DOWN, [=](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveRight = true;
        }
    });
    addEvent(INPUT_RIGHT, InputManager::EventType::UP, [=](Sender s, ui32 a) -> void {
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveRight = false;
        }
    });
    // Backward event
    addEvent(INPUT_BACKWARD, InputManager::EventType::DOWN, [=](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveBackward = true;
        }
    });
    addEvent(INPUT_BACKWARD, InputManager::EventType::UP, [=](Sender s, ui32 a) -> void {
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveBackward = false;
        }
    });
    // Jump event
    addEvent(INPUT_JUMP, InputManager::EventType::DOWN, [=](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveUp = true;
        }
    });
    addEvent(INPUT_JUMP, InputManager::EventType::UP, [=](Sender s, ui32 a) -> void {
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveUp = false;
        }
    });
    // Crouch event
    addEvent(INPUT_CROUCH, InputManager::EventType::DOWN, [=](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveDown = true;
        }
    });
    addEvent(INPUT_CROUCH, InputManager::EventType::UP, [=](Sender s, ui32 a) -> void {
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryMoveDown = false;
        }
    });
    // Left roll event
    addEvent(INPUT_LEFT_ROLL, InputManager::EventType::DOWN, [=](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryRollLeft = true;
        }
    });
    addEvent(INPUT_LEFT_ROLL, InputManager::EventType::UP, [=](Sender s, ui32 a) -> void {
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryRollLeft = false;
        }
    });
    // Right roll event
    addEvent(INPUT_RIGHT_ROLL, InputManager::EventType::DOWN, [=](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryRollRight = true;
        }
    });
    addEvent(INPUT_RIGHT_ROLL, InputManager::EventType::UP, [=](Sender s, ui32 a) -> void {
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.tryRollRight = false;
        }
    });
    // Mega speed event
    addEvent(INPUT_MEGA_SPEED, InputManager::EventType::DOWN, [=](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.superSpeed = true;
        }
    });
    addEvent(INPUT_MEGA_SPEED, InputManager::EventType::UP, [=](Sender s, ui32 a) -> void {
        for (auto& it : gameSystem->freeMoveInput) {
            it.second.superSpeed = false;
        }
    });

    m_hooks.addAutoHook(&vui::InputDispatcher::mouse.onMotion, [=](Sender s, const vui::MouseMotionEvent& e) {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : gameSystem->freeMoveInput) {
            FreeMoveComponentUpdater::rotateFromMouse(gameSystem, it.second, -e.dx, e.dy, 0.1f);
        }
    });
}

GameSystemUpdater::~GameSystemUpdater() {
    for (auto& evnt : m_events) {
        m_inputManager->unsubscribe(evnt.axisID, evnt.eventType, evnt.f);
    }
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
            if (distance < stcmp.sphericalTerrainData->radius * LOAD_DIST_MULT) {
                inVoxelRange = true;
                if (!pycmp.voxelPositionComponent) {
                    // We need to transition to a voxel component
                    // Calculate voxel position
                    auto& rotcmp = spaceSystem->m_axisRotationCT.getFromEntity(sit.first);
                    VoxelPosition3D vGridPos = VoxelSpaceConversions::worldToVoxel(rotcmp.invCurrentOrientation * relPos * VOXELS_PER_KM, stcmp.sphericalTerrainData->radius * VOXELS_PER_KM);
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
                        stcmp.sphericalTerrainData->radius * 2000.0)) * rotcmp.invCurrentOrientation * spcmp.orientation;

                    // We need to transition to the voxels
                    vcore::ComponentID vpid = GameSystemAssemblages::addVoxelPosition(gameSystem, it.first, svid, voxOrientation, vGridPos);
                    pycmp.voxelPositionComponent = vpid;
                    pycmp.velocity = voxOrientation * pycmp.velocity;
                    pycmp.velocity *= 0; // VOXELS_PER_KM;
                    
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
            pycmp.velocity *= KM_PER_VOXEL; // Add parent velocity
            gameSystem->deleteComponent("VoxelPosition", it.first);

            // Update dependencies for frustum
            gameSystem->frustum.getFromEntity(it.first).voxelPositionComponent = 0;
        }
    }
}

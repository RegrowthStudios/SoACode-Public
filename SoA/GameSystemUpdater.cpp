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

    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [=](Sender s, const vui::MouseMotionEvent& e) {
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

    // Update entity tables
    m_freeMoveUpdater.update(gameSystem);
    m_physicsUpdater.update(gameSystem, spaceSystem);
    m_collisionUpdater.update(gameSystem);
    m_frustumUpdater.update(gameSystem);
}

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

//TODO(Ben): Temporary?
void GameSystemUpdater::inputForward(Sender s, ui32 a) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveForward = true;
    }
}

GameSystemUpdater::GameSystemUpdater(OUT SoaState* soaState, InputMapper* inputManager) :
    m_soaState(soaState),
    m_inputManager(inputManager) {

    // Forward event
    forwardDel = makeDelegate(*this, &GameSystemUpdater::inputForward);
    m_inputManager->subscribe(INPUT_FORWARD, InputMapper::EventType::DOWN, forwardDel);
    m_events.emplace_back(INPUT_FORWARD, InputMapper::EventType::DOWN, forwardDel);


   /* addEvent(INPUT_FORWARD, InputManager::EventType::DOWN, [this](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveForward = true;
        }
    });*/
    addEvent(INPUT_FORWARD, InputMapper::EventType::UP, [this](Sender s, ui32 a) -> void {
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveForward = false;
        }
    });
    // Left event
    addEvent(INPUT_LEFT, InputMapper::EventType::DOWN, [this](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveLeft = true;
        }
    });
    addEvent(INPUT_LEFT, InputMapper::EventType::UP, [this](Sender s, ui32 a) -> void {
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveLeft = false;
        }
    });
    // Right event
    addEvent(INPUT_RIGHT, InputMapper::EventType::DOWN, [this](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveRight = true;
        }
    });
    addEvent(INPUT_RIGHT, InputMapper::EventType::UP, [this](Sender s, ui32 a) -> void {
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveRight = false;
        }
    });
    // Backward event
    addEvent(INPUT_BACKWARD, InputMapper::EventType::DOWN, [this](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveBackward = true;
        }
    });
    addEvent(INPUT_BACKWARD, InputMapper::EventType::UP, [this](Sender s, ui32 a) -> void {
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveBackward = false;
        }
    });
    // Jump event
    addEvent(INPUT_JUMP, InputMapper::EventType::DOWN, [this](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveUp = true;
        }
    });
    addEvent(INPUT_JUMP, InputMapper::EventType::UP, [this](Sender s, ui32 a) -> void {
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveUp = false;
        }
    });
    // Crouch event
    addEvent(INPUT_CROUCH, InputMapper::EventType::DOWN, [this](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveDown = true;
        }
    });
    addEvent(INPUT_CROUCH, InputMapper::EventType::UP, [this](Sender s, ui32 a) -> void {
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryMoveDown = false;
        }
    });
    // Left roll event
    addEvent(INPUT_LEFT_ROLL, InputMapper::EventType::DOWN, [this](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryRollLeft = true;
        }
    });
    addEvent(INPUT_LEFT_ROLL, InputMapper::EventType::UP, [this](Sender s, ui32 a) -> void {
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryRollLeft = false;
        }
    });
    // Right roll event
    addEvent(INPUT_RIGHT_ROLL, InputMapper::EventType::DOWN, [this](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryRollRight = true;
        }
    });
    addEvent(INPUT_RIGHT_ROLL, InputMapper::EventType::UP, [this](Sender s, ui32 a) -> void {
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.tryRollRight = false;
        }
    });
    // Mega speed event
    addEvent(INPUT_MEGA_SPEED, InputMapper::EventType::DOWN, [this](Sender s, ui32 a) -> void {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.superSpeed = true;
        }
    });
    addEvent(INPUT_MEGA_SPEED, InputMapper::EventType::UP, [this](Sender s, ui32 a) -> void {
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            it.second.superSpeed = false;
        }
    });

    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [this](Sender s, const vui::MouseMotionEvent& e) {
        if (!m_soaState->isInputEnabled) return;
        for (auto& it : m_soaState->gameSystem->freeMoveInput) {
            FreeMoveComponentUpdater::rotateFromMouse(m_soaState->gameSystem.get(), it.second, -e.dx, e.dy, 0.1f);
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

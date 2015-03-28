#include "stdafx.h"
#include "GameSystemEvents.h"

#include "GameSystemComponents.h"
#include "GameSystemUpdater.h"
#include "GameplayScreen.h"
#include "Inputs.h"
#include "SoAState.h"

// To cut down on code
#define ADD_INPUT(input, name) \
addEvent(INPUT_##input, InputMapper::DOWN, &GameSystemEvents::on##name##Down); \
addEvent(INPUT_##input, InputMapper::UP, &GameSystemEvents::on##name##Up);

GameSystemEvents::GameSystemEvents(GameSystemUpdater* owner) :
    m_owner(owner),
    m_inputMapper(m_owner->m_inputMapper),
    m_soaState(m_owner->m_soaState) {
    
    subscribeEvents();
}

GameSystemEvents::~GameSystemEvents() {
    unsubscribeEvents();
}

void GameSystemEvents::subscribeEvents() {
    if (m_isSubscribed) return;
    // Add all the inputs
    ADD_INPUT(FORWARD, Forward);
    ADD_INPUT(BACKWARD, Backward);
    ADD_INPUT(LEFT, Left);
    ADD_INPUT(RIGHT, Right);
    ADD_INPUT(JUMP, Jump);
    ADD_INPUT(CROUCH, Crouch);
    ADD_INPUT(LEFT_ROLL, LeftRoll);
    ADD_INPUT(RIGHT_ROLL, RightRoll);
    ADD_INPUT(MEGA_SPEED, MegaSpeed);

    vui::InputDispatcher::mouse.onMotion += makeDelegate(*this, &GameSystemEvents::onMouseMotion);
}

void GameSystemEvents::unsubscribeEvents() {
    if (!m_isSubscribed) return;

    for (auto& evnt : m_events) {
        m_inputMapper->unsubscribe(evnt.id, evnt.eventType, evnt.l);
    }
    std::vector<EventData>().swap(m_events);

    vui::InputDispatcher::mouse.onMotion -= makeDelegate(*this, &GameSystemEvents::onMouseMotion);
}

void GameSystemEvents::onForwardDown(Sender s, ui32 a) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveForward = true;
        std::cout << "MOVING FORWARD\n";
    }
}

void GameSystemEvents::onForwardUp(Sender s, ui32 a) {
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveForward = false;
    }
}

void GameSystemEvents::onBackwardDown(Sender s, ui32 a) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveBackward = true;
    }
}

void GameSystemEvents::onBackwardUp(Sender s, ui32 a) {
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveBackward = false;
    }
}

void GameSystemEvents::onLeftDown(Sender s, ui32 a) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveLeft = true;
    }
}

void GameSystemEvents::onLeftUp(Sender s, ui32 a) {
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveLeft = false;
    }
}

void GameSystemEvents::onRightDown(Sender s, ui32 a) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveRight = true;
    }
}

void GameSystemEvents::onRightUp(Sender s, ui32 a) {
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveRight = false;
    }
}

void GameSystemEvents::onJumpDown(Sender s, ui32 a) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveUp = true;
    }
}

void GameSystemEvents::onJumpUp(Sender s, ui32 a) {
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveUp = false;
    }
}

void GameSystemEvents::onCrouchDown(Sender s, ui32 a) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveDown = true;
    }
}

void GameSystemEvents::onCrouchUp(Sender s, ui32 a) {
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryMoveDown = false;
    }
}

void GameSystemEvents::onLeftRollDown(Sender s, ui32 a) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryRollLeft = true;
    }
}

void GameSystemEvents::onLeftRollUp(Sender s, ui32 a) {
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryRollLeft = false;
    }
}

void GameSystemEvents::onRightRollDown(Sender s, ui32 a) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryRollRight = true;
    }
}

void GameSystemEvents::onRightRollUp(Sender s, ui32 a) {
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.tryRollRight = false;
    }
}

void GameSystemEvents::onMegaSpeedDown(Sender s, ui32 a) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.superSpeed = true;
    }
}

void GameSystemEvents::onMegaSpeedUp(Sender s, ui32 a) {
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        it.second.superSpeed = false;
    }
}

void GameSystemEvents::onMouseMotion(Sender s, const vui::MouseMotionEvent& e) {
    if (!m_soaState->isInputEnabled) return;
    for (auto& it : m_soaState->gameSystem->freeMoveInput) {
        FreeMoveComponentUpdater::rotateFromMouse(m_soaState->gameSystem.get(), it.second, -e.dx, e.dy, 0.1f);
    }
}

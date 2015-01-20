///
/// GameSystemEvents.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Events for GameSystem
///

#pragma once

#ifndef GameSystemEvents_h__
#define GameSystemEvents_h__

#include <Vorb/Events.hpp>

#include "GameSystem.h"

// An ugly macro could make this tiny v
#pragma region wasdEvents

class GameSystemDelegate : IDelegate < ui32 > {
public:
    GameSystemDelegate(GameSystem* gameSystem) : m_gameSystem(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override = 0;
protected:
    GameSystem* m_gameSystem = nullptr;
};


/// Delegate to handle the Forward Key Down event
class OnForwardKeyDown : GameSystemDelegate {
public:
    OnForwardKeyDown(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveForward = true;
        }
    }
};
/// Delegate to handle the Forward Key UP event
class OnForwardKeyUp : GameSystemDelegate {
public:
    OnForwardKeyUp(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveForward = false;
        }
    }
};
/// Delegate to handle the Right Key Down event
class OnRightKeyDown : GameSystemDelegate {
public:
    OnRightKeyDown(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveRight = true;
        }
    }
};
/// Delegate to handle the Right Key UP event
class OnRightKeyUp : GameSystemDelegate {
public:
    OnRightKeyUp(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveRight = false;
        }
    }
};
/// Delegate to handle the Left Key Down event
class OnLeftKeyDown : GameSystemDelegate {
public:
    OnLeftKeyDown(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveLeft = true;
        }
    }
};
/// Delegate to handle the Left Key UP event
class OnLeftKeyUp : GameSystemDelegate {
public:
    OnLeftKeyUp(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveLeft = false;
        }
    }
};
/// Delegate to handle the Backward Key Down event
class OnBackwardKeyDown : GameSystemDelegate {
public:
    OnBackwardKeyDown(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveBackward = true;
        }
    }
};
/// Delegate to handle the Backward Key UP event
class OnBackwardKeyUp : GameSystemDelegate {
public:
    OnBackwardKeyUp(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveBackward = false;
        }
    }
};

#pragma endregion wasd

/// Delegate to handle the Up Key Down event
class OnUpKeyDown : GameSystemDelegate {
public:
    OnUpKeyDown(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveUp = true;
        }
    }
};
/// Delegate to handle the Up Key UP event
class OnUpKeyUp : GameSystemDelegate {
public:
    OnUpKeyUp(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveUp = false;
        }
    }
};

/// Delegate to handle the Down Key Down event
class OnDownKeyDown : GameSystemDelegate {
public:
    OnDownKeyDown(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveDown = true;
        }
    }
};
/// Delegate to handle the Down Key UP event
class OnDownKeyUp : GameSystemDelegate {
public:
    OnDownKeyUp(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryMoveDown = false;
        }
    }
};

#pragma region freeMove
/// Delegate to handle the LeftRoll Key Down event
class OnLeftRollKeyDown : GameSystemDelegate {
public:
    OnLeftRollKeyDown(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryRollLeft = true;
        }
    }
};
/// Delegate to handle the LeftRoll Key UP event
class OnLeftRollKeyUp : GameSystemDelegate {
public:
    OnLeftRollKeyUp(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryRollLeft = false;
        }
    }
};

/// Delegate to handle the RightRoll Key Down event
class OnRightRollKeyDown : GameSystemDelegate {
public:
    OnRightRollKeyDown(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryRollRight = true;
        }
    }
};
/// Delegate to handle the RightRoll Key UP event
class OnRightRollKeyUp : GameSystemDelegate {
public:
    OnRightRollKeyUp(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.tryRollRight = false;
        }
    }
};

/// Delegate to handle the SuperSpeed Key Down event
class OnSuperSpeedKeyDown : GameSystemDelegate {
public:
    OnSuperSpeedKeyDown(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.superSpeed = true;
        }
    }
};
/// Delegate to handle the SuperSpeed Key UP event
class OnSuperSpeedKeyUp : GameSystemDelegate {
public:
    OnSuperSpeedKeyUp(GameSystem* gameSystem) : GameSystemDelegate(gameSystem) {}

    virtual void invoke(Sender sender, ui32 key) override {
        for (auto& it : m_gameSystem->freeMoveInputCT) {
            it.second.superSpeed = false;
        }
    }
};

#pragma endregion freeMove

#endif // GameSystemEvents_h__

///
/// GameplayScreenEvents.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 23 Mar 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles events for GameplayScreen
///

#pragma once

#ifndef GameplayScreenEvents_h__
#define GameplayScreenEvents_h__

#include <Vorb/Events.hpp>

#include "InputMapper.h"

class GameSystemUpdater;
struct SoaState;

class GameSystemEvents {
public:
    GameSystemEvents(GameSystemUpdater* owner);
    ~GameSystemEvents();

    void subscribeEvents();
    void unsubscribeEvents();

private:

    void onForwardDown(Sender s, ui32 a);
    void onForwardUp(Sender s, ui32 a);
    void onBackwardDown(Sender s, ui32 a);
    void onBackwardUp(Sender s, ui32 a);
    void onLeftDown(Sender s, ui32 a);
    void onLeftUp(Sender s, ui32 a);
    void onRightDown(Sender s, ui32 a);
    void onRightUp(Sender s, ui32 a);
    void onJumpDown(Sender s, ui32 a);
    void onJumpUp(Sender s, ui32 a);
    void onCrouchDown(Sender s, ui32 a);
    void onCrouchUp(Sender s, ui32 a);
    void onLeftRollDown(Sender s, ui32 a);
    void onLeftRollUp(Sender s, ui32 a);
    void onRightRollDown(Sender s, ui32 a);
    void onRightRollUp(Sender s, ui32 a);
    void onMegaSpeedDown(Sender s, ui32 a);
    void onMegaSpeedUp(Sender s, ui32 a);
    void onMouseMotion(Sender s, const vui::MouseMotionEvent& e);

    /// Struct that holds event info for unsubscribing
    struct EventData {
        EventData(InputMapper::InputID iid, InputMapper::Listener L, bool IsDown) :
            id(iid), l(L), isDown(IsDown) {
            // Empty
        }
        InputMapper::InputID id;
        InputMapper::Listener l;
        bool isDown;
    };

    /// Adds an event and tracks it for unsubscribing
    template<typename F>
    void addEvent(i32 axisID, F downF, F upF) {
        InputMapper::Listener l;
        if (downF) {
            l = makeDelegate(*this, downF);
            m_inputMapper->get(axisID).downEvent += l;
            m_events.emplace_back(axisID, l, true);
        }
        if (upF) {
            l = makeDelegate(*this, upF);
            m_inputMapper->get(axisID).upEvent += l;
            m_events.emplace_back(axisID, l, false);
        }
    }

    std::vector<EventData> m_events;

    bool m_isSubscribed = false;
    GameSystemUpdater* m_owner = nullptr;
    InputMapper* m_inputMapper = nullptr;
    const SoaState* m_soaState = nullptr;
};

#endif // GameplayScreenEvents_h__

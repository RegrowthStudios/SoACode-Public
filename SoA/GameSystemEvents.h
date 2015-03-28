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
class SoaState;

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
        EventData(InputMapper::InputID iid, InputMapper::EventType etype, InputMapper::Listener L) :
            id(iid), eventType(etype), l(L) {
            // Empty
        }
        InputMapper::InputID id;
        InputMapper::EventType eventType;
        InputMapper::Listener l;
    };

    /// Adds an event and tracks it for unsubscribing
    template<typename F>
    void addEvent(i32 axisID, InputMapper::EventType eventType, F f) {
        InputMapper::Listener l = makeDelegate(*this, f);
        m_inputMapper->subscribe(axisID, eventType, l);
        m_events.emplace_back(axisID, eventType, l);
    }

    std::vector<EventData> m_events;

    bool m_isSubscribed = false;
    GameSystemUpdater* m_owner = nullptr;
    InputMapper* m_inputMapper = nullptr;
    const SoaState* m_soaState = nullptr;
};

#endif // GameplayScreenEvents_h__

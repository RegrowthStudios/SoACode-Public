///
/// GameSystemUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updates a GameSystem ECS
///

#pragma once

#ifndef GameSystemUpdater_h__
#define GameSystemUpdater_h__

#include "CollisionComponentUpdater.h"
#include "FreeMoveComponentUpdater.h"
#include "FrustumComponentUpdater.h"
#include "InputMapper.h"
#include "PhysicsComponentUpdater.h"
#include "VoxelCoordinateSpaces.h"
#include <Vorb/Events.hpp>
#include <Vorb/VorbPreDecl.inl>

class SoaState;
class SpaceSystem;
struct VoxelPositionComponent;
class SoaState;
DECL_VVOX(class VoxelPlanetMapData);

class GameSystemUpdater {
    friend class GameSystemEvents;
public:
    GameSystemUpdater(OUT SoaState* soaState, InputMapper* inputManager);
    ~GameSystemUpdater();
    /// Updates the game system, and also updates voxel components for space system
    /// planet transitions.
    /// @param gameSystem: Game ECS
    /// @param spaceSystem: Space ECS. Only SphericalVoxelComponents are modified.
    void update(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem, const SoaState* soaState);
private:
    /// Struct that holds event info for destruction
    struct EventData {
        EventData(i32 aid, InputMapper::EventType etype, InputMapper::Listener F) :
            axisID(aid), eventType(etype), f(F) {
            // Empty
        }
        i32 axisID;
        InputMapper::EventType eventType;
        InputMapper::Listener f;
    };

    /// Adds an event and tracks it for destruction
    /// @param axisID: The axis to subscribe the functor to.
    /// @param eventType: The event to subscribe the functor to.
    /// @param f: The functor to subscribe to the axes' event.
    template<typename F>
    void addEvent(i32 axisID, InputMapper::EventType eventType, F f) {
        InputMapper::Listener* rv = m_inputManager->subscribeFunctor(axisID, eventType, f);
        m_events.emplace_back(axisID, eventType, *rv);
    }

    void inputForward(Sender s, ui32 a);
    Delegate<Sender, ui32> forwardDel;

    int m_frameCounter = 0; ///< Counts frames for updateVoxelPlanetTransitions updates

    // Events
    //std::unique_ptr<GameSystemEvents> m_events = nullptr;
    AutoDelegatePool m_hooks; ///< Input hooks reservoir
    std::vector<EventData> m_events;

    /// Updaters
    PhysicsComponentUpdater m_physicsUpdater;
    CollisionComponentUpdater m_collisionUpdater;
    FreeMoveComponentUpdater m_freeMoveUpdater;
    FrustumComponentUpdater m_frustumUpdater;

    const SoaState* m_soaState = nullptr;
    InputMapper* m_inputManager = nullptr;
};

#endif // GameSystemUpdater_h__

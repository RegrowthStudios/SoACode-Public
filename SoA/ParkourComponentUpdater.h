///
/// ParkourComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Updates ParkourComponents
///

#pragma once

#include <Vorb/types.h>

#ifndef ParkourComponentUpdater_h__
#define ParkourComponentUpdater_h__

class GameSystem;
class SpaceSystem;
struct SoaState;

class ParkourComponentUpdater {
public:
    ParkourComponentUpdater():m_lastTime(-1.0f) {}

    void update(GameSystem* gameSystem, SpaceSystem* spaceSystem, const SoaState *soaState);

    f64 m_lastTime;
};

#endif // ParkourComponentUpdater_h__


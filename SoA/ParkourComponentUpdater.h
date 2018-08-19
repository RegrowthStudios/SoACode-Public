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

#ifndef ParkourComponentUpdater_h__
#define ParkourComponentUpdater_h__

class GameSystem;
class SpaceSystem;
class ParkourComponentUpdater {
public:
    void update(GameSystem* gameSystem, SpaceSystem* spaceSystem);
};

#endif // ParkourComponentUpdater_h__


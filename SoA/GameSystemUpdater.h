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

class GameSystem;

class GameSystemUpdater {
public:
    void update(OUT GameSystem* gameSystem);

    void updateVoxelPlanetTransitions(OUT GameSystem* gameSystem);
private:
    void updatePhysics(OUT GameSystem* gameSystem);
    void updateCollision(OUT GameSystem* gameSystem);
    void updateMoveInput(OUT GameSystem* gameSystem);

    int m_frameCounter = 0;
};

#endif // GameSystemUpdater_h__

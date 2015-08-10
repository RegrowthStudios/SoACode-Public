//
// ChunkSphereAcquirer.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 8 Aug 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// Acquires a sphere of chunks around an agent.
//

#pragma once

#ifndef ChunkSphereAcquirer_h__
#define ChunkSphereAcquirer_h__

#include "ChunkHandle.h"
#include "GameSystemComponents.h"

class GameSystem;
class SpaceSystem;
class ChunkAccessor;

class ChunkSphereComponentUpdater {
public:
    void update(GameSystem* gameSystem, SpaceSystem* spaceSystem);

    void setRadius(ChunkSphereComponent& cmp, ui32 radius);

private:
    void shiftDirection(ChunkSphereComponent& cmp, int axis1, int axis2, int axis3, int offset);
    // Submits a gen query and connects to neighbors
    ChunkHandle submitAndConnect(ChunkSphereComponent& cmp, const i32v3& chunkPos);
    void releaseAndDisconnect(ChunkHandle& h);
    void releaseHandles(ChunkSphereComponent& cmp);
    void initSphere(ChunkSphereComponent& cmp);
};

#endif // ChunkSphereAcquirer_h__

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
#include "GameSystem.h"

class ChunkAccessor;

class ChunkSphereComponentUpdater {
public:
    void update(GameSystem* gameSystem);

    static void setRadius(ChunkSphereComponent& cmp, ui32 radius);
};

#endif // ChunkSphereAcquirer_h__

///
/// SoaState.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// The main game state for SoA
///

#pragma once

#ifndef SoAState_h__
#define SoAState_h__

#include "SpaceSystem.h"
#include "GameSystem.h"

#include "BlockPack.h"
#include "ChunkAllocator.h"
#include "ClientState.h"
#include "Item.h"

#include "ECSTemplates.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/ecs/Entity.h>
#include <Vorb/VorbPreDecl.inl>

class PlanetGenLoader;
class SoaOptions;
DECL_VIO(class IOManager);

struct SoaState {
    SoaState() {}

    SpaceSystem* spaceSystem = nullptr;
    GameSystem* gameSystem = nullptr;

    // TODO(Ben): Clean up this dumping ground
    PagedChunkAllocator chunkAllocator;

    ECSTemplateLibrary templateLib;
    
    // TODO(Ben): Move somewhere else.
    ClientState clientState;

    vio::IOManager* systemIoManager = nullptr;

    vcore::ThreadPool<WorkerData>* threadPool = nullptr;

    SoaOptions* options = nullptr; // Lives in App

    BlockPack blocks;
    ItemPack items;
    
    vio::IOManager saveFileIom;

    f64 time = 0.0;
    bool isInputEnabled = true;
    float timeStep = 0.016f;
private:
    VORB_NON_COPYABLE(SoaState);
};

#endif // SoAState_h__

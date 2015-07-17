///
/// SoAEngine.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles initialization and destruction of SoAState
///

#pragma once

#ifndef SoAEngine_h__
#define SoAEngine_h__

#include <Vorb/ecs/Entity.h>
#include <Vorb/VorbPreDecl.inl>
#include "OptionsController.h"
#include "SpaceSystemLoader.h"

class GameSystem;
class BlockPack;
class SpaceSystem;
struct SoaState;
struct PlanetGenData;

DECL_VCORE(class RPCManager)

#pragma once
class SoaEngine {
public:

    /// Initializes the default SoaOptions
    static void initOptions(SoaOptions& options);

    /// Initializes SoaState resources
    static void initState(SoaState* state);
    
    /// Loads and initializes the SpaceSystem
    static bool loadSpaceSystem(SoaState* state, const nString& filePath);

    /// Loads and initializes the GameSystem
    static bool loadGameSystem(SoaState* state);

    /// Sets block IDs for planet data
    static void initVoxelGen(PlanetGenData* genData, const BlockPack& blocks);

    static void reloadSpaceBody(SoaState* state, vecs::EntityID eid, vcore::RPCManager* glRPC);

    /// Destroys the SoaState completely
    static void destroyAll(SoaState* state);

    static void destroyGameSystem(SoaState* state);

    static void destroySpaceSystem(SoaState* state);

    static OptionsController optionsController;
};

#endif // SoAEngine_h__

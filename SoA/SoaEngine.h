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
struct SoaState;
class SpaceSystem;

DECL_VCORE(class RPCManager)

#pragma once
class SoaEngine {
public:
    /// Parameters for loading a SpaceSystem
    struct SpaceSystemLoadData {
        nString filePath;
    };
    /// Parameters for loading a GameSystem
    struct GameSystemLoadData {
        // More stuff here
    };

    /// Initializes the default SoaOptions
    static void initOptions(SoaOptions& options);

    /// Initializes SoaState resources
    static void initState(SoaState* state);
    
    /// Loads and initializes the SpaceSystem
    /// @param state: The SoaState that holds the space system
    /// @param loadData: Parameters for loading space system
    /// @param glrpc: Optional RPCManager. If nullptr, then loadSpaceSystem will load the shader.
    ///   otherwise, it will ask glrpc to load the shader and block until finished.
    static bool loadSpaceSystem(SoaState* state, const SpaceSystemLoadData& loadData, vcore::RPCManager* glrpc = nullptr);

    /// Loads and initializes the GameSystem
    /// @param state: The SoaState that holds the space system
    /// @param loadData: Parameters for loading game system
    static bool loadGameSystem(SoaState* state, const GameSystemLoadData& loadData);

    /// Sets block IDs for planet data
    static void setPlanetBlocks(SoaState* state);

    static void reloadSpaceBody(SoaState* state, vecs::EntityID eid, vcore::RPCManager* glRPC);

    /// Destroys the SoaState completely
    static void destroyAll(SoaState* state);

    static void destroyGameSystem(SoaState* state);

    static void destroySpaceSystem(SoaState* state);

    static OptionsController optionsController;
private:
    static SpaceSystemLoader m_spaceSystemLoader;
};

#endif // SoAEngine_h__

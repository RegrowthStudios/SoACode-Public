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

class GameSystem;
class SoaState;
class SpaceSystem;
struct SpaceSystemLoadParams;
struct SystemBody;
struct SystemBodyKegProperties;

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
    /// Initializes SoaState resources
    /// @param state: The SoaState
    static void initState(OUT SoaState* state);
    
    /// Loads and initializes the SpaceSystem
    /// @param state: The SoaState that holds the space system
    /// @param loadData: Parameters for loading space system
    /// @param glrpc: Optional RPCManager. If nullptr, then loadSpaceSystem will load the shader.
    ///   otherwise, it will ask glrpc to load the shader and block until finished.
    static bool loadSpaceSystem(OUT SoaState* state, const SpaceSystemLoadData& loadData, vcore::RPCManager* glrpc = nullptr);

    /// Loads and initializes the GameSystem
    /// @param state: The SoaState that holds the space system
    /// @param loadData: Parameters for loading game system
    static bool loadGameSystem(OUT SoaState* state, const GameSystemLoadData& loadData);

    /// Destroys the SoaState
    /// @param state: The SoaState
    static void destroyAll(OUT SoaState* state);

    /// Destroys the Game System
    /// @param state: The SoaState
    static void destroyGameSystem(OUT SoaState* state);

    /// Destroys the Space System
    /// @param state: The SoaState
    static void destroySpaceSystem(OUT SoaState* state);

private:
    /// Loads and adds a star system to the SpaceSystem
    /// @param pr: params
    static void addStarSystem(SpaceSystemLoadParams& pr);

    /// Loads and adds system properties to the params
    /// @param pr: params
    /// @return true on success
    static bool loadSystemProperties(SpaceSystemLoadParams& pr);

    /// Loads and adds body properties to the params
    /// @param pr: params
    /// @param filePath: Path to body
    /// @param sysProps: Keg properties for the system
    /// @param body: The body to fill
    /// @return true on success
    static bool loadBodyProperties(SpaceSystemLoadParams& pr, const nString& filePath,
                                   const SystemBodyKegProperties* sysProps, OUT SystemBody* body);

    static void calculateOrbit(SpaceSystem* spaceSystem, vecs::EntityID entity, f64 parentMass, bool isBinary);

};

#endif // SoAEngine_h__

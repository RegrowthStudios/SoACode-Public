///
/// SpaceSystemLoader.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 4 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Loads the space system
///

#pragma once

#ifndef SpaceSystemLoader_h__
#define SpaceSystemLoader_h__

#include <Vorb/ecs/Entity.h>
#include <Vorb/VorbPreDecl.inl>
#include "VoxPool.h"

struct GasGiantProperties;
struct SystemBody;
struct SystemBodyProperties;
class SpaceSystem;
class PlanetLoader;

DECL_VIO(class IOManager)
DECL_VCORE(class RPCManager)

struct SpaceSystemLoadParams {
    SpaceSystem* spaceSystem = nullptr;
    vio::IOManager* ioManager = nullptr;
    nString dirPath;
    PlanetLoader* planetLoader = nullptr;
    vcore::RPCManager* glrpc = nullptr;
    vcore::ThreadPool<WorkerData>* threadpool = nullptr;

    std::map<nString, SystemBody*> barycenters; ///< Contains all barycenter objects
    std::map<nString, SystemBody*> systemBodies; ///< Contains all system bodies
    std::map<nString, vecs::EntityID> bodyLookupMap;
};

class SpaceSystemLoader {
public:
    /// Loads and adds a star system to the SpaceSystem
    /// @param pr: params
    static void loadStarSystem(SpaceSystemLoadParams& pr);
private:
    /// Loads path color scheme
    /// @param pr: params
    /// @return true on success
    static bool loadPathColors(SpaceSystemLoadParams& pr);

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
                                   const SystemBodyProperties* sysProps, OUT SystemBody* body);

    // Sets up mass parameters for binaries
    static void initBinaries(SpaceSystemLoadParams& pr);
    // Recursive function for binary creation
    static void initBinary(SpaceSystemLoadParams& pr, SystemBody* bary);

    // Initializes orbits and parent connections
    static void initOrbits(SpaceSystemLoadParams& pr);

    static void createGasGiant(SpaceSystemLoadParams& pr,
                               const SystemBodyProperties* sysProps,
                               GasGiantProperties* properties,
                               SystemBody* body);

    static void calculateOrbit(SpaceSystemLoadParams& pr, vecs::EntityID entity, f64 parentMass,
                               SystemBody* body, f64 binaryMassRatio = 0.0);
};

#endif // SpaceSystemLoader_h__

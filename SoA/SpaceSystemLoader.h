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
#include "SystemBodyLoader.h"

struct GasGiantProperties;
struct SystemBody;
struct SystemOrbitProperties;
class SpaceSystem;
class PlanetGenLoader;

DECL_VIO(class IOManager)
DECL_VCORE(class RPCManager)

class SpaceSystemLoader {
public:
    void init(const SoaState* soaState);
    /// Loads and adds a star system to the SpaceSystem
    /// @param pr: params
    void loadStarSystem(const nString& path);
private:
    /// Loads path color scheme
    /// @param pr: params
    /// @return true on success
    bool loadPathColors();

    /// Loads and adds system properties to the params
    /// @param pr: params
    /// @return true on success
    bool loadSystemProperties();

    // Sets up mass parameters for binaries
    void initBinaries();
    // Recursive function for binary creation
    void initBinary(SystemBody* bary);

    // Initializes orbits and parent connections
    void initOrbits();

    void computeRef(SystemBody* body);

    void calculateOrbit(vecs::EntityID entity, f64 parentMass,
                        SystemBody* body, f64 binaryMassRatio = 0.0);

    SystemBodyLoader m_bodyLoader;
    
    const SoaState* m_soaState = nullptr;
    SpaceSystem* m_spaceSystem;
    vio::IOManager* m_ioManager = nullptr;
    vcore::ThreadPool<WorkerData>* m_threadpool = nullptr;
    std::map<nString, SystemBody*> m_barycenters;
    std::map<nString, SystemBody*> m_systemBodies;
    std::map<nString, vecs::EntityID> m_bodyLookupMap;
};

#endif // SpaceSystemLoader_h__

///
/// SphericalTerrainGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Generates spherical terrain and meshes for a planet. 
/// Each planet should own one.
///

#pragma once

#ifndef SphericalTerrainGenerator_h__
#define SphericalTerrainGenerator_h__

class TerrainGenDelegate;

class SphericalTerrainGenerator {
public:
    SphericalTerrainGenerator();
    ~SphericalTerrainGenerator();

    void generateTerrain(TerrainGenDelegate* data);

    void invokeTerrainGen(vcore::RPC* so) {
        m_rpcManager.invoke(so, false);
    }
private:
    /// Generates mesh using heightmap
    void buildMesh(TerrainGenDelegate* data);

    vcore::RPCManager m_rpcManager;
};

#endif // SphericalTerrainGenerator_h__
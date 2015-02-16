///
/// TerrainRpcDispatcher.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 12 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines the TerrainGenDelegate and TerrainRpcDispatcher
///

#pragma once

#ifndef TerrainRpcDispatcher_h__
#define TerrainRpcDispatcher_h__

#include <Vorb/RPC.h>
#include "VoxelCoordinateSpaces.h"

class SphericalTerrainGpuGenerator;
class SphericalTerrainCpuGenerator;
class SphericalTerrainMesh;

class TerrainGenDelegate : public IDelegate < void* > {
public:
    virtual void invoke(Sender sender, void* userData) override;
    void release();
    volatile bool inUse = false;

    vcore::RPC rpc;

    f32v3 startPos;
    WorldCubeFace cubeFace;
    float width;
    bool isSpherical;

    SphericalTerrainMesh* mesh = nullptr;
    SphericalTerrainGpuGenerator* generator = nullptr;
};

class TerrainRpcDispatcher {
public:
    TerrainRpcDispatcher(SphericalTerrainGpuGenerator* generator,
                         SphericalTerrainCpuGenerator* cgenerator) :
                         m_generator(generator),
                         m_cgenerator(cgenerator) {
        for (int i = 0; i < NUM_GENERATORS; i++) {
            m_generators[i].generator = m_generator;
        }
    }
    /// @return a new mesh on success, nullptr on failure
    SphericalTerrainMesh* dispatchTerrainGen(const f32v3& startPos,
                                             float width,
                                             int lod,
                                             WorldCubeFace cubeFace,
                                             bool isSpherical);
private:
    static const int NUM_GENERATORS = 1024;
    int counter = 0;

    SphericalTerrainGpuGenerator* m_generator = nullptr;
    SphericalTerrainCpuGenerator* m_cgenerator = nullptr; /// Temporary, probably only need GPU for this

    TerrainGenDelegate m_generators[NUM_GENERATORS];
};

#endif // TerrainRpcDispatcher_h__
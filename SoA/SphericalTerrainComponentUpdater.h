///
/// SphericalTerrainComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updates SphericalTerrainComponents.
///

#pragma once

#ifndef SphericalTerrainComponentUpdater_h__
#define SphericalTerrainComponentUpdater_h__

class SpaceSystem;
struct SphericalTerrainComponent;
class SphericalTerrainGpuGenerator;
class SphericalTerrainCpuGenerator;
class SphericalTerrainMesh;

#include "SphericalTerrainPatch.h"
#include "VoxelCoordinateSpaces.h"
#include <Vorb/RPC.h>

#define LOAD_DIST 80000.0
// Should be even
#define PATCH_ROW 2  
#define NUM_FACES 6
const int PATCHES_PER_FACE = (PATCH_ROW * PATCH_ROW);
const int TOTAL_PATCHES = PATCHES_PER_FACE * NUM_FACES;

class TerrainGenDelegate : public IDelegate < void* > {
public:
    virtual void invoke(Sender sender, void* userData) override;
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
                                             WorldCubeFace cubeFace);
private:
    static const int NUM_GENERATORS = 1024;
    int counter = 0;

    SphericalTerrainGpuGenerator* m_generator = nullptr;
    SphericalTerrainCpuGenerator* m_cgenerator = nullptr; /// Temporary, probably only need GPU for this

    TerrainGenDelegate m_generators[NUM_GENERATORS];
};

class SphericalTerrainComponentUpdater {
public:
    void update(SpaceSystem* spaceSystem, const f64v3& cameraPos);

    /// Updates openGL specific stuff. Call on render thread
    void glUpdate(SpaceSystem* spaceSystem);

private:
    void initPatches(SphericalTerrainComponent& cmp);
};

#endif // SphericalTerrainComponentUpdater_h__
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
class SphericalTerrainComponent;
class SphericalTerrainGenerator;
class SphericalTerrainMesh;

#include <Vorb/RPC.h>
#include "SphericalTerrainPatch.h"

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
    i32v3 coordMapping;
    float width;

    SphericalTerrainMesh* mesh = nullptr;
    SphericalTerrainGenerator* generator = nullptr;
};

class TerrainRpcDispatcher {
public:
    TerrainRpcDispatcher(SphericalTerrainGenerator* generator) :
        m_generator(generator) {
        for (int i = 0; i < NUM_GENERATORS; i++) {
            m_generators[i].generator = m_generator;
        }
    }
    /// @return a new mesh on success, nullptr on failure
    SphericalTerrainMesh* dispatchTerrainGen(const f32v3& startPos,
                                             const i32v3& coordMapping,
                                             float width,
                                             int lod,
                                             CubeFace cubeFace);
private:
    static const int NUM_GENERATORS = 1024;
    int counter = 0;

    SphericalTerrainGenerator* m_generator = nullptr;

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
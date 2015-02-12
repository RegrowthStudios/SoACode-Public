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

#include "SphericalTerrainPatch.h"
#include "TerrainRpcDispatcher.h"
#include "VoxelCoordinateSpaces.h"
#include <Vorb/RPC.h>

#define LOAD_DIST 80000.0
// Should be even
#define PATCH_ROW 2  
#define NUM_FACES 6
const int PATCHES_PER_FACE = (PATCH_ROW * PATCH_ROW);
const int TOTAL_PATCHES = PATCHES_PER_FACE * NUM_FACES;

class SphericalTerrainComponentUpdater {
public:
    void update(SpaceSystem* spaceSystem, const f64v3& cameraPos);

    /// Updates openGL specific stuff. Call on render thread
    void glUpdate(SpaceSystem* spaceSystem);

private:
    void initPatches(SphericalTerrainComponent& cmp);
};

#endif // SphericalTerrainComponentUpdater_h__
///
/// FarTerrainComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 12 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updater for far terrain
///

#pragma once

#ifndef FarTerrainComponentUpdater_h__
#define FarTerrainComponentUpdater_h__

class SpaceSystem;
struct FarTerrainComponent;

#include "TerrainPatch.h"
#include "TerrainRpcDispatcher.h"
#include "VoxelCoordinateSpaces.h"
#include "SphericalTerrainComponentUpdater.h"
#include <Vorb/RPC.h>

#define FT_PATCH_ROW 16  
#define NUM_FACES 6
const int FT_TOTAL_PATCHES = (FT_PATCH_ROW * FT_PATCH_ROW);

class FarTerrainComponentUpdater {
public:
    void update(SpaceSystem* spaceSystem, const f64v3& cameraPos);

    /// Updates openGL specific stuff. Call on render thread
    void glUpdate(SpaceSystem* spaceSystem);

private:
    void initPatches(FarTerrainComponent& cmp, const f64v3& cameraPos);
    // Checks and possibly shifts the far terrain patch grid to always center on the player
    void checkGridShift(FarTerrainComponent& cmp, const i32v2& newCenter);
};

#endif // FarTerrainComponentUpdater_h__

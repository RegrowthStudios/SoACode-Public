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

#include "SphericalTerrainPatch.h"
#include "TerrainRpcDispatcher.h"
#include "VoxelCoordinateSpaces.h"
#include "SphericalTerrainComponentUpdater.h"
#include <Vorb/RPC.h>

class FarTerrainComponentUpdater {
public:
    void update(SpaceSystem* spaceSystem, const f64v3& cameraPos);

    /// Updates openGL specific stuff. Call on render thread
    void glUpdate(SpaceSystem* spaceSystem);

private:
    void initPatches(FarTerrainComponent& cmp);
};

#endif // FarTerrainComponentUpdater_h__

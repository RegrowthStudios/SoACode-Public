///
/// SphericalTerrainMeshManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef SphericalTerrainMeshManager_h__
#define SphericalTerrainMeshManager_h__

#include <RPC.h>

class SphericalTerrainMeshManager {
public:
    SphericalTerrainMeshManager();

    void update();

    void addMesh(SphericalTerrainMesh* mesh);

private:
};

#endif // SphericalTerrainMeshManager_h__
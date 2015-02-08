///
/// SphericalTerrainCpuGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Generates spherical terrain data and meshes for a planet using the CPU.
/// Each planet should own one.
///

#pragma once

#ifndef SphericalTerrainCpuGenerator_h__
#define SphericalTerrainCpuGenerator_h__

#include "SphericalTerrainPatch.h"
#include "SphericalTerrainPatchMesher.h"

#pragma once
class SphericalTerrainCpuGenerator {
public:
    SphericalTerrainCpuGenerator(SphericalTerrainMeshManager* meshManager,
                                 PlanetGenData* planetGenData);
    ~SphericalTerrainCpuGenerator();

private:
    SphericalTerrainPatchMesher m_mesher; ///< Creates patch meshes
};

#endif // SphericalTerrainCpuGenerator_h__

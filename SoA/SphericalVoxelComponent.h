///
/// SphericalVoxelComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 5 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component for voxels mapped to a spherical world
///

#pragma once

#ifndef SphericalVoxelComponent_h__
#define SphericalVoxelComponent_h__

#include "IVoxelMapper.h"

#include <Vorb/Entity.h>
#include <Vorb/VorbPreDecl.inl>

class Camera;
class Chunk;
class ChunkIOManager;
class ChunkManager;
class ParticleEngine;
class PhysicsEngine;
class PlanetGenData;
class SphericalTerrainData;
class SphericalTerrainGenerator;

DECL_VVOX(class, VoxelPlanetMapper);
DECL_VIO(class, IOManager);

class SphericalVoxelComponent {
public:
    void init(const SphericalTerrainData* sphericalTerrainData, const vio::IOManager* saveFileIom,
              SphericalTerrainGenerator* terrainGenerator,
              const glm::dvec3 &gpos, vvox::VoxelMapData* startingMapData);


    //chunk manager manages and updates the chunk grid
    ChunkManager* chunkManager = nullptr;
    ChunkIOManager* chunkIo = nullptr;
    PhysicsEngine* physicsEngine = nullptr;
    ParticleEngine* particleEngine = nullptr;

    SphericalTerrainGenerator* generator = nullptr;

    vvox::VoxelPlanetMapper* voxelPlanetMapper = nullptr;

    PlanetGenData* planetGenData = nullptr;
    const SphericalTerrainData* sphericalTerrainData = nullptr;

    const vio::IOManager* saveFileIom = nullptr;

    vcore::ComponentID sphericalTerrainComponent = 0;

    bool enabled = false;
};

#endif // SphericalVoxelComponent_h__
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
    PhysicsEngine* physicsEngine = nullptr;
    ChunkManager* chunkManager = nullptr;
    ChunkIOManager* chunkIo = nullptr;
    ParticleEngine* particleEngine = nullptr;

    SphericalTerrainGenerator* generator = nullptr;

    vvox::VoxelPlanetMapper* voxelPlanetMapper = nullptr;

    PlanetGenData* planetGenData = nullptr;
    const SphericalTerrainData* sphericalTerrainData = nullptr;

    const vio::IOManager* saveFileIom = nullptr;

    vcore::ComponentID sphericalTerrainComponent = 0;
    vcore::ComponentID namePositionComponent = 0;
    vcore::ComponentID axisRotationComponent = 0;

    f64 voxelRadius = 0; ///< Radius of the planet in voxels
};

#endif // SphericalVoxelComponent_h__
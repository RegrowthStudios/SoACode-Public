///
/// SpaceSystemComponents.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 20 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component definitions for SpaceSystem
///

#pragma once

#ifndef SpaceSystemComponents_h__
#define SpaceSystemComponents_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/ecs/Entity.h>
#include <Vorb/graphics/gtypes.h>

#include "VoxelCoordinateSpaces.h"

class ChunkGrid;
class ChunkIOManager;
class ChunkListManager;
class ChunkManager;
class ChunkMemoryManager;
class FarTerrainPatch;
class ParticleEngine;
class PhysicsEngine;
class SphericalTerrainCpuGenerator;
class SphericalTerrainGpuGenerator;
class TerrainPatch;
class TerrainPatchMeshManager;
class TerrainRpcDispatcher;
struct PlanetGenData;
struct TerrainPatchData;

DECL_VVOX(class, VoxelPlanetMapper);
DECL_VIO(class, IOManager);

/// For far and spherical terrain patch blending on transitions
const float TERRAIN_ALPHA_BEFORE_FADE = 3.0f;
const float TERRAIN_DEC_START_ALPHA = TERRAIN_ALPHA_BEFORE_FADE + 2.0f;
const float TERRAIN_INC_START_ALPHA = -TERRAIN_ALPHA_BEFORE_FADE;
const float TERRAIN_ALPHA_STEP = 0.05f;

struct AxisRotationComponent {
    f64q axisOrientation; ///< Axis of rotation
    f64q currentOrientation; ///< Current orientation with axis and rotation
    f64q invCurrentOrientation; ///< Inverse of currentOrientation
    f64 period = 0.0; ///< Period of rotation in seconds
    f64 currentRotation = 0.0; ///< Current rotation about axis in radians
};

struct NamePositionComponent {
    f64v3 position; ///< Position in space, in KM
    nString name; ///< Name of the entity
};

struct SpaceLightComponent {
    vcore::ComponentID parentNpId; ///< Component ID of parent NamePosition component
    color3 color; ///< Color of the light
    f32 intensity; ///< Intensity of the light
};

struct OrbitComponent {
    f64 semiMajor = 0.0; ///< Semi-major of the ellipse
    f64 semiMinor = 0.0; ///< Semi-minor of the ellipse
    f64 orbitalPeriod = 0.0; ///< Period in seconds of a full orbit
    f64 totalMass = 0.0; ///< Mass of this body + parent
    f64 eccentricity = 0.0; ///< Shape of orbit, 0-1
    f64 r1 = 0.0; ///< Closest distance to focal point
    f64q orientation = f64q(0.0, 0.0, 0.0, 0.0); ///< Orientation of the orbit path
    ui8v4 pathColor = ui8v4(255); ///< Color of the path
    vcore::ComponentID parentNpId = 0; ///< Component ID of parent NamePosition component
    VGBuffer vbo = 0; ///< vbo for the ellipse
};

struct SphericalGravityComponent {
    vcore::ComponentID namePositionComponent; ///< Component ID of parent NamePosition component
    f64 radius = 0.0; ///< Radius in KM
    f64 mass = 0.0; ///< Mass in KG
};

// TODO(Ben): std::unique_ptr?
struct SphericalVoxelComponent {
    friend class SphericalVoxelComponentUpdater;

    PhysicsEngine* physicsEngine = nullptr;
    ChunkGrid* chunkGrid = nullptr;
    ChunkListManager* chunkListManager = nullptr;
    ChunkMemoryManager* chunkMemoryManager = nullptr;
    ChunkIOManager* chunkIo = nullptr;
    ParticleEngine* particleEngine = nullptr;

    SphericalTerrainGpuGenerator* generator = nullptr;

    PlanetGenData* planetGenData = nullptr;
    const TerrainPatchData* sphericalTerrainData = nullptr;

    const vio::IOManager* saveFileIom = nullptr;

    vcore::ComponentID sphericalTerrainComponent = 0;
    vcore::ComponentID farTerrainComponent = 0;
    vcore::ComponentID namePositionComponent = 0;
    vcore::ComponentID axisRotationComponent = 0;

    f64 voxelRadius = 0; ///< Radius of the planet in voxels
    int refCount = 1;
};

struct SphericalTerrainComponent {
    vcore::ComponentID namePositionComponent = 0;
    vcore::ComponentID axisRotationComponent = 0;
    vcore::ComponentID sphericalVoxelComponent = 0;
    vcore::ComponentID farTerrainComponent = 0;

    TerrainRpcDispatcher* rpcDispatcher = nullptr;

    TerrainPatch* patches = nullptr; ///< Buffer for top level patches
    TerrainPatchData* sphericalTerrainData = nullptr;

    TerrainPatchMeshManager* meshManager = nullptr;
    SphericalTerrainGpuGenerator* gpuGenerator = nullptr;
    SphericalTerrainCpuGenerator* cpuGenerator = nullptr;

    PlanetGenData* planetGenData = nullptr;
    VoxelPosition3D startVoxelPosition;
    bool needsVoxelComponent = false;
    float alpha = 0.0f; ///< Alpha blending coefficient
};

struct FarTerrainComponent {
    TerrainRpcDispatcher* rpcDispatcher = nullptr;

    FarTerrainPatch* patches = nullptr; ///< Buffer for top level patches
    TerrainPatchData* sphericalTerrainData = nullptr;

    TerrainPatchMeshManager* meshManager = nullptr;
    SphericalTerrainGpuGenerator* gpuGenerator = nullptr;
    SphericalTerrainCpuGenerator* cpuGenerator = nullptr;

    WorldCubeFace face = FACE_NONE;

    PlanetGenData* planetGenData = nullptr;
    i32v2 center = i32v2(0); ///< Center, in units of patch width, where camera is
    i32v2 origin = i32v2(0); ///< Specifies which patch is the origin (back left corner) on the grid
    float alpha = 1.0f; ///< Alpha blending coefficient
    bool shouldFade = false; ///< When this is true we fade out
};

#endif // SpaceSystemComponents_h__
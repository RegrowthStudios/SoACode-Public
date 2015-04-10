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

#include "Constants.h"
#include "VoxPool.h"
#include "VoxelCoordinateSpaces.h"
#include "VoxelLightEngine.h"

class ChunkGrid;
class ChunkIOManager;
class ChunkListManager;
class ChunkManager;
class ChunkMemoryManager;
class ChunkMeshManager;
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

DECL_VVOX(class VoxelPlanetMapper);
DECL_VIO(class IOManager);

/// For far and spherical terrain patch blending on transitions
const f32 TERRAIN_FADE_LENGTH = 2.0f;
const f32 TERRAIN_ALPHA_BEFORE_FADE = 2.0f;
const f32 TERRAIN_DEC_START_ALPHA = TERRAIN_ALPHA_BEFORE_FADE + TERRAIN_FADE_LENGTH;
const f32 TERRAIN_INC_START_ALPHA = -TERRAIN_ALPHA_BEFORE_FADE;
const f32 TERRAIN_ALPHA_STEP = 0.01f;

const f32 START_FACE_TRANS = 1.0f;

struct AtmosphereComponent {
    vecs::ComponentID namePositionComponent = 0;
    f32 planetRadius;
    f32 radius;
    f32 kr = 0.0025f;
    f32 km = 0.0020f;
    f32 esun = 30.0f; // TODO(Ben): This should be dynamic
    f32 g = -0.99f;
    f32 scaleDepth = 0.25f;
    f32v3 invWavelength4 = f32v3(1.0f / powf(0.65f, 4.0f),
                                 1.0f / powf(0.57f, 4.0f),
                                 1.0f / powf(0.475f, 4.0f));
};

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
    vecs::ComponentID parentNpId; ///< Component ID of parent NamePosition component
    color3 color; ///< Color of the light
    f32 intensity; ///< Intensity of the light
};

struct OrbitComponent {
    f64 a = 0.0; ///< Semi-major of the ellipse in KM
    f64 b = 0.0; ///< Semi-minor of the ellipse in KM
    f64 t = 0.0; ///< Period of a full orbit in sec
    f64 parentMass = 0.0; ///< Mass of the parent in KG
    f64 startTrueAnomaly = 0.0; ///< Start true anomaly in rad
    f64 e = 0.0; ///< Shape of orbit, 0-1
    f64 o = 0.0; ///< Longitude of the ascending node in rad
    f64 p = 0.0; ///< Longitude of the periapsis in rad
    f64 i = 0.0; ///< Inclination in rad
    f64 r1 = 0.0; ///< Closest distance to focal point in KM
    f64v3 velocity = f64v3(0.0); ///< Current velocity relative to space in KM/s
    f64v3 relativeVelocity = f64v3(0.0); ///< Current velocity relative to parent in KM/s
    ui8v4 pathColor = ui8v4(255); ///< Color of the path
    vecs::ComponentID npID = 0; ///< Component ID of NamePosition component
    vecs::ComponentID parentOrbId = 0; ///< Component ID of parent OrbitComponent
    VGBuffer vbo = 0; ///< vbo for the ellipse
};

struct SphericalGravityComponent {
    vecs::ComponentID namePositionComponent; ///< Component ID of parent NamePosition component
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
    ChunkMeshManager* chunkMeshManager = nullptr;
    ParticleEngine* particleEngine = nullptr;
    VoxelLightEngine voxelLightEngine;

    SphericalTerrainGpuGenerator* generator = nullptr;

    PlanetGenData* planetGenData = nullptr;
    const TerrainPatchData* sphericalTerrainData = nullptr;

    const vio::IOManager* saveFileIom = nullptr;

    vecs::ComponentID sphericalTerrainComponent = 0;
    vecs::ComponentID farTerrainComponent = 0;
    vecs::ComponentID namePositionComponent = 0;
    vecs::ComponentID axisRotationComponent = 0;

    // WorldCubeFace transitionFace = FACE_NONE;

    /// The threadpool for generating chunks and meshes
    vcore::ThreadPool<WorkerData>* threadPool = nullptr;

    int numCaTasks = 0; /// TODO(Ben): Explore alternative

    f64 voxelRadius = 0; ///< Radius of the planet in voxels
    int refCount = 1;
    ui32 updateCount = 0;
};

struct SphericalTerrainComponent {
    vecs::ComponentID namePositionComponent = 0;
    vecs::ComponentID axisRotationComponent = 0;
    vecs::ComponentID sphericalVoxelComponent = 0;
    vecs::ComponentID farTerrainComponent = 0;

    TerrainRpcDispatcher* rpcDispatcher = nullptr;

    TerrainPatch* patches = nullptr; ///< Buffer for top level patches
    TerrainPatchData* sphericalTerrainData = nullptr;

    TerrainPatchMeshManager* meshManager = nullptr;
    SphericalTerrainGpuGenerator* gpuGenerator = nullptr;
    SphericalTerrainCpuGenerator* cpuGenerator = nullptr;

    PlanetGenData* planetGenData = nullptr;
    VoxelPosition3D startVoxelPosition;
    bool needsVoxelComponent = false;
    WorldCubeFace transitionFace = FACE_NONE;
    f32 alpha = 0.0f; ///< Alpha blending coefficient
    f32 faceTransTime = START_FACE_TRANS; ///< For animation on fade
    bool isFaceTransitioning = false;
    volatile bool needsFaceTransitionAnimation = false;
};

struct GasGiantComponent {
    vecs::ComponentID namePositionComponent = 0;
    vecs::ComponentID axisRotationComponent = 0;
    f64 radius = 0.0;
    f32 oblateness = 1.0f;
    VGTexture colorMap = 0;
};

struct StarComponent {
    vecs::ComponentID namePositionComponent = 0;
    vecs::ComponentID axisRotationComponent = 0;
    f64 radius = 0.0; ///< in KM
    f64 temperature = 0.0; ///< In kelvin
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
    WorldCubeFace transitionFace = FACE_NONE;
    f32 alpha = 1.0f; ///< Alpha blending coefficient
    bool shouldFade = false; ///< When this is true we fade out
};

#endif // SpaceSystemComponents_h__
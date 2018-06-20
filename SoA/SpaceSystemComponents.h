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
#include <Vorb/concurrentqueue.h>

#include <Vorb/io/Keg.h>
#include <Vorb/ecs/ECS.h>
#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/script/Function.h>

#include "Constants.h"
#include "SpaceSystemLoadStructs.h"
#include "VoxPool.h"
#include "VoxelCoordinateSpaces.h"
#include "VoxelLightEngine.h"
#include "ChunkGrid.h"

class BlockPack;
class ChunkIOManager;
class ChunkManager;
class FarTerrainPatch;
class PagedChunkAllocator;
class ParticleEngine;
class PhysicsEngine;
class SphericalHeightmapGenerator;
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
    f32 oblateness = 0.0f;
    f32 kr = 0.0025f;
    f32 km = 0.0020f;
    f32 esun = 30.0f; // TODO(Ben): This should be dynamic
    f32 g = -0.99f;
    f32 scaleDepth = 0.25f;
    f32v3 invWavelength4 = f32v3(1.0f / powf(0.65f, 4.0f),
                                 1.0f / powf(0.57f, 4.0f),
                                 1.0f / powf(0.475f, 4.0f));
};
typedef vecs::ComponentTable<AtmosphereComponent> AtmosphereComponentTable;
KEG_TYPE_DECL(AtmosphereComponent);

struct CloudsComponent {
    vecs::ComponentID namePositionComponent = 0;
    f32 planetRadius;
    f32 height;
    f32v3 color;
    f32v3 scale;
    float density;
};
typedef vecs::ComponentTable<CloudsComponent> CloudsComponentTable;
KEG_TYPE_DECL(CloudsComponent);


struct AxisRotationComponent {
    f64q axisOrientation; ///< Axis of rotation
    f64q currentOrientation; ///< Current orientation with axis and rotation
    f64q invCurrentOrientation; ///< Inverse of currentOrientation
    f64 period = 0.0; ///< Period of rotation in seconds
    f64 currentRotation = 0.0; ///< Current rotation about axis in radians
    f32 tilt = 0.0f;
};
typedef vecs::ComponentTable<AxisRotationComponent> AxisRotationComponentTable;
KEG_TYPE_DECL(AxisRotationComponent);

struct NamePositionComponent {
    f64v3 position = f64v3(0.0); ///< Position in space, in KM
    nString name; ///< Name of the entity
};
typedef vecs::ComponentTable<NamePositionComponent> NamePositionComponentTable;
KEG_TYPE_DECL(NamePositionComponent);

struct SpaceLightComponent {
    vecs::ComponentID npID; ///< Component ID of parent NamePosition component
    color3 color; ///< Color of the light
    f32 intensity; ///< Intensity of the light
};
typedef vecs::ComponentTable<SpaceLightComponent> SpaceLightComponentTable;
KEG_TYPE_DECL(SpaceLightComponent);

struct OrbitComponent {
    f64 a = 0.0; ///< Semi-major of the ellipse in KM
    f64 b = 0.0; ///< Semi-minor of the ellipse in KM
    f64 t = 0.0; ///< Period of a full orbit in sec
    f64 parentMass = 0.0; ///< Mass of the parent in KG
    f64 startMeanAnomaly = 0.0; ///< Start mean anomaly in rad
    f64 e = 0.0; ///< Shape of orbit, 0-1
    f64 o = 0.0; ///< Longitude of the ascending node in rad
    f64 p = 0.0; ///< Longitude of the periapsis in rad
    f64 i = 0.0; ///< Inclination in rad
    f64v3 velocity = f64v3(0.0); ///< Current velocity relative to space in KM/s
    f64v3 relativeVelocity = f64v3(0.0); ///< Current velocity relative to parent in KM/s
    f32v4 pathColor[2]; ///< Color of the path
    vecs::ComponentID npID = 0; ///< Component ID of NamePosition component
    vecs::ComponentID parentOrbId = 0; ///< Component ID of parent OrbitComponent
    VGBuffer vbo = 0; ///< vbo for the ellipse mesh
    VGBuffer vao = 0; ///< vao for the ellipse mesh
    ui32 numVerts = 0; ///< Number of vertices in the ellipse
    struct Vertex {
        f32v3 position;
        f32 angle;
    };
    std::vector<Vertex> verts; ///< Vertices for the ellipse
    f32 currentMeanAnomaly;
    SpaceObjectType type; ///< Type of object
    bool isCalculated = false; ///< True when orbit has been calculated
};
KEG_TYPE_DECL(OrbitComponent);

struct PlanetRing {
    f32 innerRadius;
    f32 outerRadius;
    f64q orientation;
    vio::Path texturePath;
};

struct PlanetRingsComponent {
    vecs::ComponentID namePositionComponent;
    std::vector<PlanetRing> rings;
};
typedef vecs::ComponentTable<PlanetRingsComponent> PlanetRingsComponentTable;
KEG_TYPE_DECL(PlanetRingsComponent);

struct SphericalGravityComponent {
    vecs::ComponentID namePositionComponent; ///< Component ID of parent NamePosition component
    f64 radius = 0.0; ///< Radius in KM
    f64 mass = 0.0; ///< Mass in KG
};
typedef vecs::ComponentTable<SphericalGravityComponent> SphericalGravityComponentTable;
KEG_TYPE_DECL(SphericalGravityComponent);

struct SphericalVoxelComponent {
    ChunkGrid* chunkGrids = nullptr; // should be size 6, one for each face
    ChunkIOManager* chunkIo = nullptr;

    SphericalHeightmapGenerator* generator = nullptr;

    PlanetGenData* planetGenData = nullptr;
    const TerrainPatchData* sphericalTerrainData = nullptr;

    const vio::IOManager* saveFileIom = nullptr;
    const BlockPack* blockPack = nullptr;

    vecs::ComponentID sphericalTerrainComponent = 0;
    vecs::ComponentID farTerrainComponent = 0;
    vecs::ComponentID namePositionComponent = 0;
    vecs::ComponentID axisRotationComponent = 0;

    /// The threadpool for generating chunks and meshes
    vcore::ThreadPool<WorkerData>* threadPool = nullptr;

    int numCaTasks = 0; /// TODO(Ben): Explore alternative

    f64 voxelRadius = 0; ///< Radius of the planet in voxels
    int refCount = 1;
    ui32 updateCount = 0;
};
KEG_TYPE_DECL(SphericalVoxelComponent);

struct SphericalTerrainComponent {
    vecs::ComponentID namePositionComponent = 0;
    vecs::ComponentID axisRotationComponent = 0;
    vecs::ComponentID sphericalVoxelComponent = 0;
    vecs::ComponentID farTerrainComponent = 0;

    TerrainPatch* patches = nullptr; ///< Buffer for top level patches
    TerrainPatchData* sphericalTerrainData = nullptr;

    TerrainPatchMeshManager* meshManager = nullptr;
    SphericalHeightmapGenerator* cpuGenerator = nullptr;

    PlanetGenData* planetGenData = nullptr;
    VoxelPosition3D startVoxelPosition;
    bool needsVoxelComponent = false;
    WorldCubeFace transitionFace = FACE_NONE;
    f32 alpha = 0.0f; ///< Alpha blending coefficient
    f32 faceTransTime = START_FACE_TRANS; ///< For animation on fade
    f64 distance = FLT_MAX;
    f64 radius = 0.0;
    bool isFaceTransitioning = false;
    volatile bool needsFaceTransitionAnimation = false;
};
KEG_TYPE_DECL(SphericalTerrainComponent);


struct GasGiantComponent {
    vecs::ComponentID namePositionComponent = 0;
    vecs::ComponentID axisRotationComponent = 0;
    f64 radius = 0.0;
    f32 oblateness = 1.0f;
    nString colorMapPath = "";
    Array<PlanetRingProperties> rings;
};
typedef vecs::ComponentTable<GasGiantComponent> GasGiantComponentTable;
KEG_TYPE_DECL(GasGiantComponent);

struct StarComponent {
    vecs::ComponentID namePositionComponent = 0;
    vecs::ComponentID axisRotationComponent = 0;
    f64 mass = 0.0; ///< In KG
    f64 radius = 0.0; ///< In KM
    f64 temperature = 0.0; ///< In kelvin
    VGQuery occlusionQuery[2]; ///< TODO(Ben): Delete this
    f32 visibility = 1.0;
};
typedef vecs::ComponentTable<StarComponent> StarComponentTable;
KEG_TYPE_DECL(StarComponent);

struct FarTerrainComponent {
    TerrainRpcDispatcher* rpcDispatcher = nullptr;

    FarTerrainPatch* patches = nullptr; ///< Buffer for top level patches
    TerrainPatchData* sphericalTerrainData = nullptr;

    TerrainPatchMeshManager* meshManager = nullptr;
    SphericalHeightmapGenerator* cpuGenerator = nullptr;
    vcore::ThreadPool<WorkerData>* threadPool = nullptr;

    WorldCubeFace face = FACE_NONE;

    PlanetGenData* planetGenData = nullptr;
    i32v2 center = i32v2(0); ///< Center, in units of patch width, where camera is
    i32v2 origin = i32v2(0); ///< Specifies which patch is the origin (back left corner) on the grid
    WorldCubeFace transitionFace = FACE_NONE;
    f32 alpha = 1.0f; ///< Alpha blending coefficient
    bool shouldFade = false; ///< When this is true we fade out
};
KEG_TYPE_DECL(FarTerrainComponent);

#endif // SpaceSystemComponents_h__

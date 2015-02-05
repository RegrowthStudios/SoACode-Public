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

class ChunkIOManager;
class ChunkManager;
class ParticleEngine;
class PhysicsEngine;
class PlanetGenData;
class PlanetGenData;
class SphericalTerrainData;
class SphericalTerrainGenerator;
class SphericalTerrainMeshManager;
class SphericalTerrainPatch;
class TerrainRpcDispatcher;

DECL_VVOX(class, VoxelPlanetMapper);
DECL_VIO(class, IOManager);

struct AxisRotationComponent {
    f64q axisOrientation; ///< Axis of rotation
    f64q currentOrientation; ///< Current orientation with axis and rotation
    f64q invCurrentOrientation; ///< Inverse of currentOrientation
    f64 angularSpeed_RS = 0.0; ///< Rotational speed about axis in radians per second
    f64 currentRotation = 0.0; ///< Current rotation about axis in radians
};

struct NamePositionComponent {
    f64v3 position; ///< Position in space, in KM
    nString name; ///< Name of the entity
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
    f64 radius = 0.0; ///< Radius in KM
    f64 mass = 0.0; ///< Mass in KG
};

// TODO(Ben): std::unique_ptr?
struct SphericalVoxelComponent {
    PhysicsEngine* physicsEngine = nullptr;
    ChunkManager* chunkManager = nullptr;
    ChunkIOManager* chunkIo = nullptr;
    ParticleEngine* particleEngine = nullptr;

    SphericalTerrainGenerator* generator = nullptr;

    PlanetGenData* planetGenData = nullptr;
    const SphericalTerrainData* sphericalTerrainData = nullptr;

    const vio::IOManager* saveFileIom = nullptr;

    vcore::ComponentID sphericalTerrainComponent = 0;
    vcore::ComponentID namePositionComponent = 0;
    vcore::ComponentID axisRotationComponent = 0;

    f64 voxelRadius = 0; ///< Radius of the planet in voxels
    int refCount = 1;
};

struct SphericalTerrainComponent {
    vcore::ComponentID namePositionComponent = 0;
    vcore::ComponentID axisRotationComponent = 0;

    TerrainRpcDispatcher* rpcDispatcher = nullptr;

    SphericalTerrainPatch* patches = nullptr; ///< Buffer for top level patches
    SphericalTerrainData* sphericalTerrainData = nullptr;

    SphericalTerrainMeshManager* meshManager = nullptr;
    SphericalTerrainGenerator* generator = nullptr;

    PlanetGenData* planetGenData = nullptr;
};

#endif // SpaceSystemComponents_h__
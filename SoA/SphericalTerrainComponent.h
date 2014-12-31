///
/// SphericalTerrainComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 3 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines the component for creating spherical terrain
/// for planets and stuff.
///

#pragma once

#ifndef SphericalTerrainComponent_h__
#define SphericalTerrainComponent_h__

#include "stdafx.h"
#include "SphericalTerrainPatch.h"
#include "SphericalTerrainGenerator.h"

#include <deque>

class Camera;
class NamePositionComponent;
class AxisRotationComponent;
class PlanetGenData;
class SphericalTerrainMeshManager;

class TerrainGenDelegate : public IDelegate<void*> {
public:
    virtual void invoke(void* sender, void* userData);
    volatile bool inUse = false;

    vcore::RPC rpc;

    f32v3 startPos;
    i32v3 coordMapping;
    float width;

    SphericalTerrainMesh* mesh = nullptr;
    SphericalTerrainGenerator* generator = nullptr;
};

class TerrainRpcDispatcher {
public:
    TerrainRpcDispatcher(SphericalTerrainGenerator* generator) :
        m_generator(generator) {
        for (int i = 0; i < NUM_GENERATORS; i++) {
            m_generators[i].generator = m_generator;
        }
    }
    /// @return a new mesh on success, nullptr on failure
    SphericalTerrainMesh* dispatchTerrainGen(const f32v3& startPos,
                                             const i32v3& coordMapping,
                                             float width,
                                             int lod,
                                             CubeFace cubeFace);
private:
    static const int NUM_GENERATORS = 1024;
    int counter = 0;

    SphericalTerrainGenerator* m_generator = nullptr;

    TerrainGenDelegate m_generators[NUM_GENERATORS];
};

class SphericalTerrainComponent {
public:
    /// Initialize the spherical terrain
    /// @param radius: Radius of the planet, must be multiple of 32.
    void init(f64 radius, PlanetGenData* planetGenData,
              vg::GLProgram* normalProgram,
              vg::TextureRecycler* normalMapRecycler);

    void update(const f64v3& cameraPos,
                const NamePositionComponent* npComponent);

    /// Updates openGL specific stuff. Call on render thread
    void glUpdate();


    void draw(const Camera* camera,
              vg::GLProgram* terrainProgram,
              vg::GLProgram* waterProgram,
              const NamePositionComponent* npComponent,
              const AxisRotationComponent* arComponent);
private:
    void initPatches();

    TerrainRpcDispatcher* rpcDispatcher = nullptr;

    SphericalTerrainPatch* m_patches = nullptr; ///< Buffer for top level patches
    SphericalTerrainData* m_sphericalTerrainData = nullptr;

    SphericalTerrainMeshManager* m_meshManager = nullptr;
    SphericalTerrainGenerator* m_generator = nullptr;

    PlanetGenData* m_planetGenData = nullptr;
};

#endif // SphericalTerrainComponent_h__
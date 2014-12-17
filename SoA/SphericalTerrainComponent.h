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

class NamePositionComponent;
class Camera;
class SphericalTerrainMeshManager;

class TerrainGenDelegate : public IDelegate<void*> {
public:
    virtual void invoke(void* sender, void* userData);
    bool inUse = false;
    volatile bool finished = false;
    vcore::RPC rpc;

    f32v3 startPos;
    f32v3 coordMults;
    i32v3 coordMapping;

    SphericalTerrainMesh* mesh = nullptr;
    SphericalTerrainGenerator* generator = nullptr;
    SphericalTerrainMeshManager* meshManager = nullptr;

    float heightData[PATCH_WIDTH][PATCH_WIDTH];
};

class TerrainRpcDispatcher {
public:
    TerrainRpcDispatcher(SphericalTerrainMeshManager* meshManager) :
        m_meshManager(meshManager) {
        // Empty
    }
    /// @return delegate on success, nullptr on failure
    TerrainGenDelegate* dispatchTerrainGen();
private:
    static const int NUM_GENERATORS = 1000;
    int counter = 0;

    SphericalTerrainMeshManager* m_meshManager;

    TerrainGenDelegate m_generators[NUM_GENERATORS];
};

class SphericalTerrainComponent {
public:
    /// Initialize the spherical terrain
    /// @param radius: Radius of the planet, must be multiple of 32.
    void init(f64 radius, MeshManager* meshManager);

    void update(const f64v3& cameraPos,
                const NamePositionComponent* npComponent);

    void draw(const Camera* camera,
              vg::GLProgram* terrainProgram,
              const NamePositionComponent* npComponent);
private:
    void initPatches();

    TerrainRpcDispatcher* rpcDispatcher = nullptr;

    SphericalTerrainPatch* m_patches = nullptr; ///< Buffer for top level patches
    SphericalTerrainData* m_sphericalTerrainData = nullptr;
};

#endif // SphericalTerrainComponent_h__
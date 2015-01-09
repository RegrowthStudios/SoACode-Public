#include "stdafx.h"
#include "SphericalTerrainComponent.h"

#include <Vorb/utils.h>

#include "AxisRotationComponent.h"
#include "Camera.h"
#include "Errors.h"
#include "NamePositionComponent.h"
#include "PlanetLoader.h"
#include "SphericalTerrainMeshManager.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

void TerrainGenDelegate::invoke(void* sender, void* userData) {
    generator->generateTerrain(this);
}

void SphericalTerrainComponent::init(f64 radius, PlanetGenData* planetGenData,
                                     vg::GLProgram* normalProgram,
                                     vg::TextureRecycler* normalMapRecycler) {
    
    planetGenData = planetGenData;
    if (meshManager != nullptr) {
        pError("Tried to initialize SphericalTerrainComponent twice!");
    }

    meshManager = new SphericalTerrainMeshManager(planetGenData,
                                                    normalMapRecycler);
    generator = new SphericalTerrainGenerator(radius, meshManager,
                                                planetGenData,
                                                normalProgram, normalMapRecycler);
    rpcDispatcher = new TerrainRpcDispatcher(generator);
    
    f64 patchWidth = (radius * 2.000) / PATCH_ROW;
    sphericalTerrainData = new SphericalTerrainData(radius, patchWidth);
}


SphericalTerrainMesh* TerrainRpcDispatcher::dispatchTerrainGen(const f32v3& startPos,
                                                               const i32v3& coordMapping,
                                                               float width,
                                                               int lod,
                                                               CubeFace cubeFace) {
    SphericalTerrainMesh* mesh = nullptr;
    // Check if there is a free generator
    if (!m_generators[counter].inUse) {
        auto& gen = m_generators[counter];
        // Mark the generator as in use
        gen.inUse = true;
        gen.rpc.data.f = &gen;
        mesh = new SphericalTerrainMesh(cubeFace);
        // Set the data
        gen.startPos = startPos;
        gen.coordMapping = coordMapping;
        gen.mesh = mesh;
        gen.width = width;
        // Invoke generator
        m_generator->invokePatchTerrainGen(&gen.rpc);
        // Go to next generator
        counter++;
        if (counter == NUM_GENERATORS) counter = 0;
    }
    return mesh;
}
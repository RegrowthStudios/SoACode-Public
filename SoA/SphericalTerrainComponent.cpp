#include "stdafx.h"
#include "SphericalTerrainComponent.h"

#include "AxisRotationComponent.h"
#include "Camera.h"
#include "Errors.h"
#include "NamePositionComponent.h"
#include "PlanetLoader.h"
#include "SphericalTerrainMeshManager.h"
#include "utils.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#define LOAD_DIST 80000.0
// Should be even
#define PATCH_ROW 2  
#define NUM_FACES 6
const int PATCHES_PER_FACE = (PATCH_ROW * PATCH_ROW);
const int TOTAL_PATCHES = PATCHES_PER_FACE * NUM_FACES;

void TerrainGenDelegate::invoke(void* sender, void* userData) {
    generator->generateTerrain(this);
}

void SphericalTerrainComponent::init(f64 radius, PlanetGenData* planetGenData,
                                     vg::GLProgram* normalProgram,
                                     vg::TextureRecycler* normalMapRecycler) {
    
    m_planetGenData = planetGenData;
    if (m_meshManager != nullptr) {
        pError("Tried to initialize SphericalTerrainComponent twice!");
    }

    m_meshManager = new SphericalTerrainMeshManager(m_planetGenData,
                                                    normalMapRecycler);
    m_generator = new SphericalTerrainGenerator(radius, m_meshManager,
                                                m_planetGenData,
                                                normalProgram, normalMapRecycler);
    rpcDispatcher = new TerrainRpcDispatcher(m_generator);
    
    f64 patchWidth = (radius * 2.000) / PATCH_ROW;
    m_sphericalTerrainData = new SphericalTerrainData(radius, patchWidth);
}


SphericalTerrainMesh* TerrainRpcDispatcher::dispatchTerrainGen(const f32v3& startPos,
                                                               const i32v3& coordMapping,
                                                               float width,
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
        m_generator->invokeTerrainGen(&gen.rpc);
        // Go to next generator
        counter++;
        if (counter == NUM_GENERATORS) counter = 0;
    }
    return mesh;
}

void SphericalTerrainComponent::update(const f64v3& cameraPos,
                                       const NamePositionComponent* npComponent) {
    /// Calculate camera distance
    f64v3 relativeCameraPos = cameraPos - npComponent->position;
    f64 distance = glm::length(relativeCameraPos);

    if (distance <= LOAD_DIST) {
        // In range, allocate if needed
        if (!m_patches) {
            initPatches();
        }

        // Update patches
        for (int i = 0; i < TOTAL_PATCHES; i++) {
            m_patches[i].update(relativeCameraPos);
        }
    } else { 
        // Out of range, delete everything
        if (m_patches) {
            delete[] m_patches;
            m_patches = nullptr;
        }
    }
}

void SphericalTerrainComponent::glUpdate() {
    // Generate meshes and terrain
    m_generator->update();
}

void SphericalTerrainComponent::draw(const Camera* camera,
                                     vg::GLProgram* terrainProgram,
                                     vg::GLProgram* waterProgram,
                                     const NamePositionComponent* npComponent,
                                     const AxisRotationComponent* arComponent) {
    if (!m_patches) return;

    f32m4 rotationMatrix = f32m4(glm::toMat4(arComponent->currentOrientation));

    f32m4 VP = camera->getProjectionMatrix() * camera->getViewMatrix();

    

    f64v3 relativeCameraPos = camera->getPosition() - npComponent->position;

    // Draw patches
    m_meshManager->draw(relativeCameraPos, camera->getViewMatrix(), VP,
                        rotationMatrix, terrainProgram, waterProgram);
}

void SphericalTerrainComponent::initPatches() {
    f64 patchWidth = m_sphericalTerrainData->m_patchWidth;

    // Allocate top level patches
    m_patches = new SphericalTerrainPatch[TOTAL_PATCHES];

    int center = PATCH_ROW / 2;
    f64v2 gridPos;
    int index = 0;

    // Init all the top level patches for each of the 6 grids
    for (int face = 0; face < NUM_FACES; face++) {
        for (int z = 0; z < PATCH_ROW; z++) {
            for (int x = 0; x < PATCH_ROW; x++) {
                auto& p = m_patches[index++];
                gridPos.x = (x - center) * patchWidth;
                gridPos.y = (z - center) * patchWidth;
                p.init(gridPos, static_cast<CubeFace>(face),
                       0, m_sphericalTerrainData, patchWidth,
                       rpcDispatcher);
            }
        }
    }
}
#include "stdafx.h"
#include "SphericalTerrainComponent.h"
#include "Camera.h"

#include "NamePositionComponent.h"

void SphericalTerrainComponent::init(f64 radius) {
    m_sphericalTerrainData = new SphericalTerrainData(radius, f64v2(0.0),
                                                    f64v3(0.0));
    m_circumference = 2.0 * M_PI * radius;
}

#define LOAD_DIST 20000.0
// Should be even
#define PATCH_ROW 16  
#define NUM_PATCHES (PATCH_ROW * PATCH_ROW)

void SphericalTerrainComponent::update(const f64v3& cameraPos,
                                       const NamePositionComponent* npComponent) {
    f64v3 cameraVec = cameraPos - npComponent->position;
    double distance = glm::length(cameraVec);

    if (distance <= LOAD_DIST) {
        // In range, allocate if needed
        if (m_patchesGrid.empty()) {
            float patchWidth = m_circumference / 4.0 / PATCH_ROW / 2.0;
            // Set up origin
            m_sphericalTerrainData->m_gridCenter = f64v2(0.0);
            m_sphericalTerrainData->m_gridCenterWorld =
                glm::normalize(cameraVec) * m_sphericalTerrainData->m_radius;

            // Allocate top level patches
            m_patches = new SphericalTerrainPatch[NUM_PATCHES];

            int center = PATCH_ROW / 2;
            f64v2 gridPos;
            int index = 0;
            // Set up shiftable patch grid
            m_patchesGrid.resize(PATCH_ROW);
            for (int z = 0; z < m_patchesGrid.size(); z++) {
                auto& v = m_patchesGrid[z];
                v.resize(PATCH_ROW);
                for (int x = 0; x < v.size(); x++) {
                    v[x] = &m_patches[index++];
                    gridPos.x = (x - center) * patchWidth;
                    gridPos.y = (z - center) * patchWidth;
                    v[x]->init(gridPos, m_sphericalTerrainData, patchWidth);
                }
            }
        }

        // Update patches
        for (int i = 0; i < NUM_PATCHES; i++) {
            m_patches[i].update(cameraPos);
        }
    } else { 
        // Out of range, delete everything
        if (m_patchesGrid.size()) {
            delete[] m_patches;
            m_patches = nullptr;
            m_patchesGrid.clear();
        }
    }
}

void SphericalTerrainComponent::draw(const Camera* camera, vg::GLProgram* terrainProgram,
                                     const NamePositionComponent* npComponent) {
    if (!m_patches) return;

    f32m4 VP = camera->getProjectionMatrix() * camera->getViewMatrix();

    f64v3 relativeCameraPos = camera->getPosition() - npComponent->position;

    printVec("POS: ", relativeCameraPos);

    // Draw patches
    for (int i = 0; i < NUM_PATCHES; i++) {
        m_patches[i].draw(relativeCameraPos, VP, terrainProgram);
    }
}

#include "stdafx.h"
#include "SphericalTerrainComponent.h"
#include "Camera.h"
#include "utils.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "NamePositionComponent.h"

#define LOAD_DIST 20000.0
// Should be even
#define PATCH_ROW 16  
#define NUM_PATCHES (PATCH_ROW * PATCH_ROW)

#define FACE_TOP 0
#define FACE_LEFT 1
#define FACE_RIGHT 2
#define FACE_FRONT 3
#define FACE_BACK 4
#define FACE_BOTOM 5

#define NUM_FACES 6

void SphericalTerrainComponent::init(f64 radius) {
    m_circumference = 2.0 * M_PI * radius;
    f64 patchWidth = m_circumference / 4.0 / (PATCH_ROW / 2.0);

    m_sphericalTerrainData = new SphericalTerrainData(radius, f64v2(0.0),
                                                      f64v3(0.0), patchWidth);
    m_circumference = 2.0 * M_PI * radius;
}

void SphericalTerrainComponent::update(const f64v3& cameraPos,
                                       const NamePositionComponent* npComponent) {
    f64v3 cameraVec = cameraPos - npComponent->position;
    double distance = glm::length(cameraVec);

    if (distance <= LOAD_DIST) {
        // In range, allocate if needed
        if (!m_patches) {
            f64 patchWidth = m_sphericalTerrainData->m_patchWidth;

            // Allocate top level patches
            m_patches = new SphericalTerrainPatch[NUM_PATCHES * NUM_FACES];

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

        updateGrid(cameraPos, npComponent);

        // Update patches
        for (int i = 0; i < NUM_PATCHES; i++) {
            m_patches[i].update(cameraPos);
        }
    } else { 
        // Out of range, delete everything
        if (m_patches) {
            delete[] m_patches;
            m_patches = nullptr;
            m_patchesGrid.clear();
        }
    }
}

void SphericalTerrainComponent::draw(const Camera* camera,
                                     vg::GLProgram* terrainProgram,
                                     const NamePositionComponent* npComponent) {
    if (!m_patches) return;

    f32m4 VP = camera->getProjectionMatrix() * camera->getViewMatrix();

    f64v3 relativeCameraPos = camera->getPosition() - npComponent->position;

    f32v3 cameraNormal = glm::normalize(f32v3(relativeCameraPos));

    f32q rotQuat = quatBetweenVectors(f32v3(0.0f, 1.0f, 0.0f), cameraNormal);
    f32m4 rotMat = glm::toMat4(rotQuat);

    glUniformMatrix4fv(terrainProgram->getUniform("unRot"), 1, GL_FALSE, &rotMat[0][0]);

  //  glUniform3fv(terrainProgram->getUniform("unCameraNormal"), 1, &cameraNormal[0]);
  //  glUniform3fv(terrainProgram->getUniform("unCameraLeft"), 1, &cameraLeft[0]);

    //printVec("POS: ", relativeCameraPos);

    // Draw patches
    for (int i = 0; i < NUM_PATCHES; i++) {
        m_patches[i].draw(relativeCameraPos, VP, terrainProgram);
    }
}

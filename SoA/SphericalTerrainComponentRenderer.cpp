#include "stdafx.h"
#include "SphericalTerrainComponentRenderer.h"

#include "SpaceSystemComponents.h"
#include "Camera.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainMeshManager.h"

#include <glm/gtx/quaternion.hpp>

#define KM_PER_VOXEL 0.0005

void SphericalTerrainComponentRenderer::draw(SphericalTerrainComponent& cmp, const Camera* camera,
                                             const Camera* voxelCamera,
                                             vg::GLProgram* terrainProgram,
                                             vg::GLProgram* waterProgram,
                                             const NamePositionComponent* npComponent,
                                             const AxisRotationComponent* arComponent) {
    if (cmp.patches && !voxelCamera) {
        f32m4 rotationMatrix = f32m4(glm::toMat4(arComponent->currentOrientation));

        f64v3 relativeCameraPos = camera->getPosition() - npComponent->position;

        // Draw spherical patches
        cmp.meshManager->drawSphericalMeshes(relativeCameraPos, camera,
                                             rotationMatrix, terrainProgram, waterProgram);
    }

    if (voxelCamera) {
        glDisable(GL_CULL_FACE);
        f64v3 relativeCameraPos = voxelCamera->getPosition() * KM_PER_VOXEL;
        // Draw far patches
        cmp.meshManager->drawFarMeshes(relativeCameraPos, voxelCamera,
                                       terrainProgram, waterProgram);
    }
}
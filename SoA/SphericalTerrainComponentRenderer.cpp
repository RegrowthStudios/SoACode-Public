#include "stdafx.h"
#include "SphericalTerrainComponentRenderer.h"

#include "SpaceSystemComponents.h"
#include "Camera.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainMeshManager.h"

#include <glm/gtx/quaternion.hpp>

void SphericalTerrainComponentRenderer::draw(SphericalTerrainComponent& cmp, const Camera* camera,
                                             vg::GLProgram* terrainProgram,
                                             vg::GLProgram* waterProgram,
                                             const NamePositionComponent* npComponent,
                                             const AxisRotationComponent* arComponent) {
    if (!cmp.patches) return;

    f32m4 rotationMatrix = f32m4(glm::toMat4(arComponent->currentOrientation));

    f64v3 relativeCameraPos = camera->getPosition() - npComponent->position;
    
    // Draw patches
    cmp.meshManager->draw(relativeCameraPos, camera,
                      rotationMatrix, terrainProgram, waterProgram);
}
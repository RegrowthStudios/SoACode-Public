#include "stdafx.h"
#include "SphericalTerrainComponentRenderer.h"

#include "Camera.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatchMeshManager.h"
#include "TerrainPatch.h"

#include <glm/gtx/quaternion.hpp>

#include <Vorb/io/IOManager.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/utils.h>

SphericalTerrainComponentRenderer::~SphericalTerrainComponentRenderer() {

}

void SphericalTerrainComponentRenderer::draw(SphericalTerrainComponent& cmp,
                                             const Camera* camera,
                                             vg::GLProgram* terrainProgram,
                                             vg::GLProgram* waterProgram,
                                             const f64v3& lightPos,
                                             const SpaceLightComponent* spComponent,
                                             const NamePositionComponent* npComponent,
                                             const AxisRotationComponent* arComponent) {

    if (cmp.patches) {
        f32v3 lightDir = f32v3(glm::normalize(lightPos - npComponent->position));
        f64v3 relativeCameraPos = camera->getPosition() - npComponent->position;
        // Draw spherical patches
        cmp.meshManager->drawSphericalMeshes(relativeCameraPos, camera,
                                             arComponent->currentOrientation,
                                             terrainProgram, waterProgram,
                                             lightDir);
    }
}
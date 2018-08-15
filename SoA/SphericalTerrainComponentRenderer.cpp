#include "stdafx.h"
#include "SphericalTerrainComponentRenderer.h"

#include "Camera.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatchMeshManager.h"
#include "TerrainPatch.h"
#include "soaUtils.h"
#include "ShaderLoader.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/ShaderManager.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/utils.h>

SphericalTerrainComponentRenderer::~SphericalTerrainComponentRenderer() {
    dispose();
}

void SphericalTerrainComponentRenderer::initGL() {
    m_terrainProgram = ShaderLoader::createProgramFromFile("Shaders/SphericalTerrain/SphericalTerrain.vert",
                                                           "Shaders/SphericalTerrain/SphericalTerrain.frag");
    // Set constant uniforms
    m_terrainProgram.use();
    glUniform1i(m_terrainProgram.getUniform("unColorMap"), 1);
    glUniform1i(m_terrainProgram.getUniform("unGrassTexture"), 2);
    glUniform1i(m_terrainProgram.getUniform("unRockTexture"), 3);
     m_terrainProgram.unuse();

    m_waterProgram = ShaderLoader::createProgramFromFile("Shaders/SphericalTerrain/SphericalWater.vert",
                                                         "Shaders/SphericalTerrain/SphericalWater.frag");
    // Set constant uniforms
    m_waterProgram.use();
    glUniform1i(m_waterProgram.getUniform("unNormalMap"), 0);
    glUniform1i(m_waterProgram.getUniform("unColorMap"), 1);
    m_waterProgram.unuse();
}

// TODO: Is this implementation complete?
void SphericalTerrainComponentRenderer::draw(SphericalTerrainComponent& cmp,
                                             const Camera* camera,
                                             const f32v3& lightDir,
                                             const f64v3& position,
                                             const f32 zCoef,
                                             const SpaceLightComponent* spComponent VORB_UNUSED,
                                             const AxisRotationComponent* arComponent,
                                             const AtmosphereComponent* aComponent) {
    if (cmp.patches) {
        
        f64v3 relativeCameraPos = camera->getPosition() - position;

        // Sort meshes
        cmp.meshManager->sortSpericalMeshes(relativeCameraPos);
        // Draw spherical patches
        if (cmp.alpha >= 1.0f) {
            cmp.meshManager->drawSphericalMeshes(relativeCameraPos, camera,
                                                 arComponent->currentOrientation,
                                                 m_terrainProgram, m_waterProgram,
                                                 lightDir,
                                                 1.0f,
                                                 zCoef,
                                                 aComponent,
                                                 true);
        }
    }
}

void SphericalTerrainComponentRenderer::dispose() {
    if (m_terrainProgram.isCreated()) m_terrainProgram.dispose();
    if (m_waterProgram.isCreated()) m_waterProgram.dispose();
}

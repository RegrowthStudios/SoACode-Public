#include "stdafx.h"
#include "TerrainPatchMeshManager.h"

#include "Errors.h"
#include "PlanetLoader.h"
#include "Camera.h"

#include <glm/gtx/quaternion.hpp>

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/TextureRecycler.hpp>

#include "FarTerrainPatch.h"
#include "PlanetData.h"
#include "RenderUtils.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatch.h"
#include "TerrainPatchMesh.h"
#include "soaUtils.h"

void TerrainPatchMeshManager::drawSphericalMeshes(const f64v3& relativePos,
                                                  const Camera* camera,
                                                  const f64q& orientation, vg::GLProgram* program,
                                                  vg::GLProgram* waterProgram,
                                                  const f32v3& lightDir,
                                                  f32 alpha,
                                                  const AtmosphereComponent* aCmp,
                                                  bool drawSkirts) {
    
    static f32 dt = 0.0;
    dt += 0.001;

    f64q invOrientation = glm::inverse(orientation);
    const f64v3 rotpos = invOrientation * relativePos;
    const f32v3 rotLightDir = f32v3(invOrientation * f64v3(lightDir));
    // Convert f64q to f32q
    f32q orientationF32;
    orientationF32.x = (f32)orientation.x;
    orientationF32.y = (f32)orientation.y;
    orientationF32.z = (f32)orientation.z;
    orientationF32.w = (f32)orientation.w;
    // Convert to matrix
    f32m4 rotationMatrix = glm::toMat4(orientationF32);
    f32m4 W(1.0);
    setMatrixTranslation(W, -relativePos);
    W *= rotationMatrix;
    f32m4 WVP = camera->getViewProjectionMatrix() * W;

    if (m_waterMeshes.size()) {
        // Bind textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->liquidColorMap.id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->liquidTexture.id);
        waterProgram->use();
        waterProgram->enableVertexAttribArrays();
        // Set uniforms
        glUniform1f(waterProgram->getUniform("unDt"), dt);
        glUniform1f(waterProgram->getUniform("unDepthScale"), m_planetGenData->liquidDepthScale);
        glUniform1f(waterProgram->getUniform("unFreezeTemp"), m_planetGenData->liquidFreezeTemp / 255.0f);
        glUniform3fv(waterProgram->getUniform("unLightDirWorld"), 1, &rotLightDir[0]);
        glUniform1f(waterProgram->getUniform("unAlpha"), alpha);
        // Set up scattering uniforms
        setScatterUniforms(waterProgram, rotpos, aCmp);

        for (int i = 0; i < m_waterMeshes.size();) {
            auto& m = m_waterMeshes[i];
            if (m->m_shouldDelete) {
                // Only delete here if m_wvbo is 0. See comment [15] in below block
                if (m->m_wvbo) {
                    vg::GpuMemory::freeBuffer(m->m_wvbo);
                } else {
                    delete m;
                }

                m = m_waterMeshes.back();
                m_waterMeshes.pop_back();
              
            } else {
                // TODO(Ben): Horizon and frustum culling for water too
                m->drawWater(WVP, waterProgram);
                i++;
            }
        }
        waterProgram->disableVertexAttribArrays();
        waterProgram->unuse();
    }

    if (m_meshes.size()) {
        // Bind textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->terrainColorMap.id);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->terrainTexture.id);
        glActiveTexture(GL_TEXTURE0);
        program->use();
        program->enableVertexAttribArrays();
        glUniform3fv(program->getUniform("unLightDirWorld"), 1, &rotLightDir[0]);
        glUniform1f(program->getUniform("unAlpha"), alpha);
        // Set up scattering uniforms
        setScatterUniforms(program, rotpos, aCmp);

        for (int i = 0; i < m_meshes.size();) {
            auto& m = m_meshes[i];

            if (m->m_shouldDelete) {
                m->recycleNormalMap(m_normalMapRecycler);

                // [15] If m_wvbo is 1, then chunk was marked for delete between
                // Drawing water and terrain. So we free m_wvbo to mark it
                // for delete on the next pass through m_waterMeshes
                if (m->m_wvbo) {
                    vg::GpuMemory::freeBuffer(m->m_wvbo);
                } else {
                    delete m;
                }

                m = m_meshes.back();
                m_meshes.pop_back();
            } else {
                /// Use bounding box to find closest point
                f64v3 closestPoint = m->getClosestPoint(rotpos);
                
                // Check horizon culling first, it's more likely to cull spherical patches
                if (!TerrainPatch::isOverHorizon(rotpos, closestPoint,
                    m_planetGenData->radius)) {
                    // Check frustum culling
                    // TODO(Ben): There could be a way to reduce the number of frustum checks
                    // via caching or checking a parent
                    f32v3 relSpherePos = orientationF32 * m->m_aabbCenter - f32v3(relativePos);
                    if (camera->sphereInFrustum(relSpherePos,
                        m->m_boundingSphereRadius)) {
                        m->draw(WVP, program, drawSkirts);
                    }
                }
                i++;
            }
        }
        program->disableVertexAttribArrays();
        program->unuse();
    }
}

TerrainPatchMeshManager::~TerrainPatchMeshManager() {
    for (auto& i : m_meshes) {
        delete i;
    }
    for (auto& i : m_farMeshes) {
        delete i;
    }
}

void TerrainPatchMeshManager::addMesh(TerrainPatchMesh* mesh, bool isSpherical) {
    if (isSpherical) {
        m_meshes.push_back(mesh);
        if (mesh->m_wvbo) {
            m_waterMeshes.push_back(mesh);
        }
    } else {
        m_farMeshes.push_back(mesh);
        if (mesh->m_wvbo) {
            m_farWaterMeshes.push_back(mesh);
        }
    }
    mesh->m_isRenderable = true;

}

bool meshComparator(TerrainPatchMesh* m1, TerrainPatchMesh* m2) {
    return (m1->distance2 < m2->distance2);
}

void TerrainPatchMeshManager::sortSpericalMeshes(const f64v3& relPos) {
    m_closestSphericalDistance2 = DOUBLE_SENTINEL;
    // Calculate squared distances
    for (auto& mesh : m_meshes) {
        f64v3 distVec = mesh->getClosestPoint(relPos) - relPos;
        mesh->distance2 = selfDot(distVec);
        // Useful for dynamic clipping plane
        if (mesh->distance2 < m_closestSphericalDistance2) m_closestSphericalDistance2 = mesh->distance2;
    }

    // Not sorting water since it would be of minimal benifit
    std::sort(m_meshes.begin(), m_meshes.end(), [](TerrainPatchMesh* m1, TerrainPatchMesh* m2) -> bool {
        return (m1->distance2 < m2->distance2);
    });
}

void TerrainPatchMeshManager::sortFarMeshes(const f64v3& relPos) {
    m_closestFarDistance2 = DOUBLE_SENTINEL;
    // Calculate squared distances
    for (auto& mesh : m_farMeshes) {
        f64v3 distVec = mesh->getClosestPoint(relPos) - relPos;
        mesh->distance2 = selfDot(distVec);
        // Useful for dynamic clipping plane
        if (mesh->distance2 < m_closestFarDistance2) m_closestFarDistance2 = mesh->distance2;
    }

    // Not sorting water since it would be of minimal benifit
    std::sort(m_farMeshes.begin(), m_farMeshes.end(), [](TerrainPatchMesh* m1, TerrainPatchMesh* m2) -> bool {
        return (m1->distance2 < m2->distance2);
    });
}

void TerrainPatchMeshManager::drawFarMeshes(const f64v3& relativePos,
                                            const Camera* camera,
                                            vg::GLProgram* program, vg::GLProgram* waterProgram,
                                            const f32v3& lightDir,
                                            f32 alpha, f32 radius,
                                            const AtmosphereComponent* aCmp,
                                            bool drawSkirts) {
    // TODO(Ben): This will lose precision over time
    static f32 dt = 0.0f;
    dt += 0.0001f;

    if (m_farWaterMeshes.size()) {
        // Bind textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->liquidColorMap.id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->liquidTexture.id);
        waterProgram->use();
        waterProgram->enableVertexAttribArrays();
        // Set uniforms
        glUniform1f(waterProgram->getUniform("unDt"), dt);
        glUniform1f(waterProgram->getUniform("unDepthScale"), m_planetGenData->liquidDepthScale);
        glUniform1f(waterProgram->getUniform("unFreezeTemp"), m_planetGenData->liquidFreezeTemp / 255.0f);
        glUniform1f(waterProgram->getUniform("unRadius"), radius);
        glUniform3fv(waterProgram->getUniform("unLightDirWorld"), 1, &lightDir[0]);
        glUniform1f(waterProgram->getUniform("unAlpha"), alpha);
        // Set up scattering uniforms
        setScatterUniforms(waterProgram, f64v3(0, relativePos.y + radius, 0), aCmp);

        for (int i = 0; i < m_farWaterMeshes.size();) {
            auto& m = m_farWaterMeshes[i];
            if (m->m_shouldDelete) {
                // Only delete here if m_wvbo is 0. See comment [15] in below block
                if (m->m_wvbo) {
                    vg::GpuMemory::freeBuffer(m->m_wvbo);
                } else {
                    delete m;
                }

                m = m_farWaterMeshes.back();
                m_farWaterMeshes.pop_back();

            } else {
                m->drawWaterAsFarTerrain(relativePos, camera->getViewProjectionMatrix(), waterProgram);
                i++;
            }
        }
        waterProgram->disableVertexAttribArrays();
        waterProgram->unuse();
    }

    if (m_farMeshes.size()) {
        // Bind textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->terrainColorMap.id);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->terrainTexture.id);
        glActiveTexture(GL_TEXTURE0);
        program->use();
        program->enableVertexAttribArrays();
        glUniform1f(program->getUniform("unRadius"), radius); // TODO(Ben): Use real radius
        glUniform3fv(program->getUniform("unLightDirWorld"), 1, &lightDir[0]);
        glUniform1f(program->getUniform("unAlpha"), alpha);

        // Set up scattering uniforms
        setScatterUniforms(program, f64v3(0, relativePos.y + radius, 0), aCmp);

        for (int i = 0; i < m_farMeshes.size();) {
            auto& m = m_farMeshes[i];
            if (m->m_shouldDelete) {
                m->recycleNormalMap(m_normalMapRecycler);

                // [15] If m_wvbo is 1, then chunk was marked for delete between
                // Drawing water and terrain. So we free m_wvbo to mark it
                // for delete on the next pass through m_farWaterMeshes
                if (m->m_wvbo) {
                    vg::GpuMemory::freeBuffer(m->m_wvbo);
                } else {
                    delete m;
                }

                m = m_farMeshes.back();
                m_farMeshes.pop_back();
            } else {
                // Check frustum culling
                // TODO(Ben): There could be a way to reduce the number of frustum checks
                // via caching or checking a parent
                // Check frustum culling first, it's more likely to cull far patches
                f32v3 relSpherePos = m->m_aabbCenter - f32v3(relativePos);
                if (camera->sphereInFrustum(relSpherePos, m->m_boundingSphereRadius)) {
                    /// Use bounding box to find closest point
                    f64v3 closestPoint = m->getClosestPoint(relativePos);
                    if (!FarTerrainPatch::isOverHorizon(relativePos, closestPoint,
                        m_planetGenData->radius)) {
                        m->drawAsFarTerrain(relativePos, camera->getViewProjectionMatrix(), program, drawSkirts);
                    }
                }
                i++;
            }
        }
        program->disableVertexAttribArrays();
        program->unuse();
    }
}

void TerrainPatchMeshManager::setScatterUniforms(vg::GLProgram* program, const f64v3& relPos, const AtmosphereComponent* aCmp) {
    // Set up scattering uniforms
    if (aCmp) {
        f32v3 relPosF(relPos);
        f32 camHeight = glm::length(relPosF);
        glUniform3fv(program->getUniform("unCameraPos"), 1, &relPosF[0]);
        glUniform3fv(program->getUniform("unInvWavelength"), 1, &aCmp->invWavelength4[0]);
        glUniform1f(program->getUniform("unCameraHeight2"), camHeight * camHeight);
        glUniform1f(program->getUniform("unInnerRadius"), aCmp->planetRadius);
        glUniform1f(program->getUniform("unOuterRadius"), aCmp->radius);
        glUniform1f(program->getUniform("unOuterRadius2"), aCmp->radius * aCmp->radius);
        glUniform1f(program->getUniform("unKrESun"), aCmp->kr * aCmp->esun);
        glUniform1f(program->getUniform("unKmESun"), aCmp->km * aCmp->esun);
        glUniform1f(program->getUniform("unKr4PI"), aCmp->kr * M_4_PI);
        glUniform1f(program->getUniform("unKm4PI"), aCmp->km * M_4_PI);
        f32 scale = 1.0f / (aCmp->radius - aCmp->planetRadius);
        glUniform1f(program->getUniform("unScale"), scale);
        glUniform1f(program->getUniform("unScaleDepth"), aCmp->scaleDepth);
        glUniform1f(program->getUniform("unScaleOverScaleDepth"), scale / aCmp->scaleDepth);
        glUniform1i(program->getUniform("unNumSamples"), 3);
        glUniform1f(program->getUniform("unNumSamplesF"), 3.0f);
        glUniform1f(program->getUniform("unG"), aCmp->g);
        glUniform1f(program->getUniform("unG2"), aCmp->g * aCmp->g);
    }
}

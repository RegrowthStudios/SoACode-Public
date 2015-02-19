#include "stdafx.h"
#include "TerrainPatchMeshManager.h"

#include "Errors.h"
#include "PlanetLoader.h"
#include "Camera.h"

#include <glm/gtx/quaternion.hpp>

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/TextureRecycler.hpp>

#include "TerrainPatchMesh.h"
#include "TerrainPatch.h"
#include "FarTerrainPatch.h"
#include "PlanetData.h"
#include "soaUtils.h"

void TerrainPatchMeshManager::drawSphericalMeshes(const f64v3& relativePos, const Camera* camera,
                                       const f64q& orientation, vg::GLProgram* program,
                                       vg::GLProgram* waterProgram) {
    
    static float dt = 0.0;
    dt += 0.001;

    const f64v3 rotpos = glm::inverse(orientation) * relativePos;
    // Convert f64q to f32q
    f32q orientationF32;
    orientationF32.x = (f32)orientation.x;
    orientationF32.y = (f32)orientation.y;
    orientationF32.z = (f32)orientation.z;
    orientationF32.w = (f32)orientation.w;
    // Convert to matrix
    f32m4 rotationMatrix = glm::toMat4(orientationF32);

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
                m->drawWater(relativePos, camera, rotationMatrix, waterProgram);
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
                        m->draw(relativePos, camera, rotationMatrix, program);
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

void TerrainPatchMeshManager::drawFarMeshes(const f64v3& relativePos, const Camera* camera, vg::GLProgram* program, vg::GLProgram* waterProgram) {
    static float dt = 0.0;
    dt += 0.001;

    glm::mat4 rot(1.0f); // no rotation

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
        glUniform1f(waterProgram->getUniform("unRadius"), 4200.0f); // TODO(Ben): Use real radius
       
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
                m->drawWater(relativePos, camera, rot, waterProgram);
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
        glUniform1f(program->getUniform("unRadius"), 4200.0f); // TODO(Ben): Use real radius

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
                if (1 || camera->sphereInFrustum(relSpherePos, m->m_boundingSphereRadius)) {
                    /// Use bounding box to find closest point
                    f64v3 closestPoint = m->getClosestPoint(relativePos);
                    if (!FarTerrainPatch::isOverHorizon(relativePos, closestPoint,
                        m_planetGenData->radius)) {
                        m->draw(relativePos, camera, rot, program);
                    }
                }
                i++;
            }
        }
        program->disableVertexAttribArrays();
        program->unuse();
    }
}

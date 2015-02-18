#include "stdafx.h"
#include "TerrainPatchMeshManager.h"

#include "Errors.h"
#include "PlanetLoader.h"
#include "Camera.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/TextureRecycler.hpp>

#include "TerrainPatchMesh.h"
#include "TerrainPatch.h"
#include "FarTerrainPatch.h"
#include "PlanetData.h"
#include "soaUtils.h"

void TerrainPatchMeshManager::drawSphericalMeshes(const f64v3& relativePos, const Camera* camera,
                                       const f32m4& rot,
                                       vg::GLProgram* program, vg::GLProgram* waterProgram) {
    
    static float dt = 0.0;
    dt += 0.001;

    f64v3 rotpos = f64v3(glm::inverse(rot) * f32v4(relativePos, 1.0f));
    f64v3 closestPoint;

    if (m_waterMeshes.size()) {

        waterProgram->use();
        waterProgram->enableVertexAttribArrays();

        // Bind textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->liquidColorMap.id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->liquidTexture.id);

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
                m->drawWater(relativePos, camera, rot, waterProgram);
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
                closestPoint = m->getClosestPoint(rotpos);
                
                // Check horizon culling first, it's more likely to cull spherical patches
                if (!TerrainPatch::isOverHorizon(rotpos, closestPoint,
                    m_planetGenData->radius)) {
                    // Check frustum culling
                    // TODO(Ben): There could be a way to reduce the number of frustum checks
                    // via caching or checking a parent
                    f32v3 relSpherePos = m->m_aabbCenter - f32v3(rotpos);
                    if (camera->sphereInFrustum(relSpherePos,
                        m->m_boundingSphereRadius)) {
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

    f64v3 closestPoint;

    if (m_farWaterMeshes.size()) {

        waterProgram->use();
        waterProgram->enableVertexAttribArrays();

        // Bind textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->liquidColorMap.id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_planetGenData->liquidTexture.id);

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
                if (camera->sphereInFrustum(relSpherePos, m->m_boundingSphereRadius)) {
                    /// Use bounding box to find closest point
                    closestPoint = m->getClosestPoint(relativePos);
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

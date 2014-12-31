#include "stdafx.h"
#include "SphericalTerrainMeshManager.h"

#include "Errors.h"
#include "PlanetLoader.h"
#include "Camera.h"

#include <Vorb/GLProgram.h>
#include <Vorb/TextureRecycler.hpp>

#include "SphericalTerrainPatch.h"

void SphericalTerrainMeshManager::draw(const f64v3& relativePos, const Camera* camera,
                                       const f32m4& rot,
                                       vg::GLProgram* program, vg::GLProgram* waterProgram) {
    
    static float dt = 0.0;
    dt += 0.001;

    if (m_waterMeshes.size()) {

        if (!waterProgram->getIsLinked()) {
            pError("HELLO");
        }
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
            if (m_waterMeshes[i]->m_shouldDelete) {
                // Don't delete here, it will happen in m_meshes
                m_waterMeshes[i] = m_waterMeshes.back();
                m_waterMeshes.pop_back();
            } else {
                m_waterMeshes[i]->drawWater(relativePos, camera, rot, waterProgram);
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
            if (m_meshes[i]->m_shouldDelete) {
                m_meshes[i]->recycleNormalMap(m_normalMapRecycler);
                delete m_meshes[i];
                m_meshes[i] = m_meshes.back();
                m_meshes.pop_back();
            } else {
                m_meshes[i]->draw(relativePos, camera, rot, program);
                i++;
            }
        }

        program->disableVertexAttribArrays();
        program->unuse();
    }
}

void SphericalTerrainMeshManager::addMesh(SphericalTerrainMesh* mesh) {
    m_meshes.push_back(mesh);
    if (mesh->m_wvbo) {
        m_waterMeshes.push_back(mesh);
    }
    mesh->m_isRenderable = true;
}
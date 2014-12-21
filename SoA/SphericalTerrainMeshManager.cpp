#include "stdafx.h"
#include "SphericalTerrainMeshManager.h"

#include "SphericalTerrainPatch.h"

void SphericalTerrainMeshManager::draw(const f64v3& cameraPos, const f32m4& V, const f32m4& VP, vg::GLProgram* program) {
    for (int i = 0; i < m_meshes.size(); i++) {
        if (m_meshes[i]->shouldDelete) {
            delete m_meshes[i];
            m_meshes[i] = m_meshes.back();
            m_meshes.pop_back();
            i--;
        } else {
            m_meshes[i]->draw(cameraPos, V, VP, program);
        }
    }
}

void SphericalTerrainMeshManager::addMesh(SphericalTerrainMesh* mesh) {
    m_meshes.push_back(mesh);
    mesh->isRenderable = true;
}
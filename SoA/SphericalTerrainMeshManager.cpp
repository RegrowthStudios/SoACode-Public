#include "stdafx.h"
#include "SphericalTerrainMeshManager.h"

#include "SphericalTerrainPatch.h"

void SphericalTerrainMeshManager::draw(const f64v3& cameraPos, const f32m4& VP, vg::GLProgram* program) {
    for (int i = 0; i < m_meshes.size(); i++) {
        m_meshes[i]->draw(cameraPos, VP, program);
    }
}

void SphericalTerrainMeshManager::addMesh(SphericalTerrainMesh* mesh) {
    m_meshes.push_back(mesh);
    mesh->isRenderable = true;
}
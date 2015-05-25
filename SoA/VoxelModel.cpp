#include "stdafx.h"
#include "VoxelModel.h"

#include "VoxelMatrix.h"
#include "VoxelModelLoader.h"
#include "ModelMesher.h"

VoxelModel::VoxelModel():
m_mesh()
{ /* Empty */}

VoxelModel::~VoxelModel() {
    for(int i = m_matrices.size() - 1; i >= 0; i--) {
        delete[] m_matrices.back();
        m_matrices.pop_back();
    }
}

void VoxelModel::loadFromFile(const nString& path) {
    addMatrices(VoxelModelLoader::loadModel(path));
    m_mesh = ModelMesher::createMesh(this);
}

void VoxelModel::addMatrix(VoxelMatrix* matrix) {
    m_matrices.push_back(matrix);
}

void VoxelModel::addMatrices(std::vector<VoxelMatrix*> matrices) {
    for(VoxelMatrix* matrix : matrices) {
        m_matrices.push_back(matrix);
    }
}
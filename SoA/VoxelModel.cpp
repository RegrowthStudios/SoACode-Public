#include "stdafx.h"
#include "VoxelModel.h"

#include "VoxelMatrix.h"
#include "VoxelModelLoader.h"
#include "ModelMesher.h"

VoxelModel::VoxelModel():
m_mesh() {
    // Empty
}

VoxelModel::~VoxelModel() {
    m_matrix.dispose();
}

void VoxelModel::loadFromFile(const nString& path) {
    setMatrix(VoxelModelLoader::loadModel(path));
    m_mesh = ModelMesher::createMesh(this);
}

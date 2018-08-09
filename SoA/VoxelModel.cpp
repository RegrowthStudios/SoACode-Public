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

bool VoxelModel::loadFromFile(const nString& path) {
    VoxelMatrix matrix;
    if (!VoxelModelLoader::loadModel(path, matrix)) {
        return false;
    }
    setMatrix(matrix);
    return true;
}

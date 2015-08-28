#pragma once
#ifndef VoxelModel_h__
#define VoxelModel_h__

#include <vector>

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>

#include "VoxelModelMesh.h"
#include "VoxelMatrix.h"

class VoxelModelVertex;

class VoxelModel {
public:
    VoxelModel();
    ~VoxelModel();
    
    void loadFromFile(const nString& path);
    
    void setMatrix(const VoxelMatrix& matrix) { m_matrix = matrix; }
    void setMesh(const VoxelModelMesh& mesh) { m_mesh = mesh; }
    
    VoxelMatrix& getMatrix() { return m_matrix; }
    const VoxelMatrix& getMatrix() const { return m_matrix; }
    const VoxelModelMesh& getMesh() const { return m_mesh; }

private:
    VoxelMatrix m_matrix;
    VoxelModelMesh m_mesh;
};

#endif //VoxelModel_h__
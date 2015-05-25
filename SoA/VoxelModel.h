#pragma once
#ifndef VoxelModel_h__
#define VoxelModel_h__

#include <vector>

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>

#include <glm/gtc/quaternion.hpp>

#include "VoxelModelMesh.h"

class VoxelMatrix;
class VoxelModelVertex;

class VoxelModel {
public:
    VoxelModel();
    ~VoxelModel();
    
    void loadFromFile(const nString& path);
    void addMatrix(VoxelMatrix* matrix);
    void addMatrices(std::vector<VoxelMatrix*> matrices);
    const std::vector<VoxelMatrix*>& getMatrices() const { return m_matrices; }

    void setMesh(const VoxelModelMesh& mesh) { m_mesh = mesh; }
    const VoxelModelMesh& getMesh() const { return m_mesh; }

private:
    std::vector<VoxelMatrix*> m_matrices;
    VoxelModelMesh m_mesh;
};

#endif //VoxelModel_h__
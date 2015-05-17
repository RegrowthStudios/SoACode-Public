#pragma once
#ifndef VoxelModel_h__
#define VoxelModel_h__

#include <vector>

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>

#include <glm/gtc/quaternion.hpp>

class VoxelMatrix;
class VoxelModelVertex;

class VoxelModel {
public:
    VoxelModel();
    ~VoxelModel();
    
    void loadFromFile(const nString& path);
    void addMatrix(VoxelMatrix* matrix);
    void addMatrices(std::vector<VoxelMatrix*> matrices);

    void generateMesh();
    void draw(f32m4 projectionMatrix);

    void setShaderProgram(vg::GLProgram program) { m_program = program; }

private:
    std::vector<VoxelMatrix*> m_matrices;
    VGVertexBuffer m_verts;
    VGVertexBuffer m_inds;
    ui32 m_indCount;
    vg::GLProgram m_program;

    f32v3 m_translation;
    glm::quat m_rotation;
    f32v3 m_scale;

    void genMatrixMesh(const VoxelMatrix* matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices);
};

#endif //VoxelModel_h__
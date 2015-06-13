#pragma once
#ifndef ModelMesher_h__
#define ModelMesher_h__

class VoxelMatrix;
class VoxelModelVertex;
class VoxelModelMesh;

class ModelMesher {
public:
    static VoxelModelMesh createMesh(const VoxelModel* model);

private:
    ModelMesher();

    static void genMatrixMesh(const VoxelMatrix* matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices);
};

#endif //ModelMesher_h__
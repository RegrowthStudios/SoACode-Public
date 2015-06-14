#pragma once
#ifndef ModelMesher_h__
#define ModelMesher_h__

class VoxelMatrix;
class VoxelModel;
class VoxelModelMesh;
class VoxelModelVertex;

class ModelMesher {
public:
    static VoxelModelMesh createMesh(const VoxelModel* model);
    static VoxelModelMesh createMarchingCubesMesh(const VoxelModel* model);

private:
    static f32 getMarchingPotential(const VoxelMatrix& matrix, int x, int y, int z);
    static void genMatrixMesh(const VoxelMatrix& matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices);
};

#endif //ModelMesher_h__
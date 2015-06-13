#pragma once
#ifndef VoxelModelRenderer_h__
#define VoxelModelRenderer_h__

class VoxelMatrix;
class VoxelModel;

class VoxelModelRenderer {
public:
    VoxelModelRenderer();
    ~VoxelModelRenderer();

    static void draw(VoxelModel* model, f32m4 projectionMatrix, f32v3 translation = f32v3(0.0f, 0.0f, 0.0f), f32v3 eulerRotation = f32v3(0.0f, 0.0f, 0.0f), f32v3 scale = f32v3(1.0f, 1.0f, 1.0f));
};

#endif //VoxelModelRenderer_h__
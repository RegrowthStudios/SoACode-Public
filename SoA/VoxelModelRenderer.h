#pragma once
#ifndef VoxelModelRenderer_h__
#define VoxelModelRenderer_h__

#include "Vorb/graphics/GLProgram.h"

class VoxelMatrix;
class VoxelModel;

class VoxelModelRenderer {
public:
    void initGL();
    void dispose();
    void draw(VoxelModel* model, f32m4 mVP, const f64v3& relativePos, const f64q& orientation);
private:
    vg::GLProgram m_program;
};

#endif //VoxelModelRenderer_h__
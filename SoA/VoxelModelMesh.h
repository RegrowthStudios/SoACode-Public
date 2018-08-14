#pragma once
#ifndef VoxelModelMesh_h__
#define VoxelModelMesh_h__

#include <Vorb/types.h>
#include <Vorb/graphics/gtypes.h>

class VoxelModelVertex {
public:
    VoxelModelVertex() {}
    VoxelModelVertex(f32v3 pos, color3 color, f32v3 normal):
        pos(pos),
        normal(normal),
        color(color) {}
    f32v3 pos;
    f32v3 normal;
    color3 color;
    ui8 padding;
};

class VoxelModelMesh {
    friend class ModelMesher;
public:
    void bind() const;
    static void unbind();

    void dispose();

    VGVertexBuffer getVBO() const { return m_vbo; }
    VGIndexBuffer getIBO() const { return m_ibo; }
    VGVertexArray getVAO() const { return m_vao; }
    ui32 getIndexCount() const { return m_indCount; }
    ui32 getTriCount() const { return m_triCount; }
private:
    VGVertexBuffer m_vbo = 0;
    VGIndexBuffer m_ibo = 0;
    VGVertexArray m_vao = 0;
    ui32 m_indCount = 0;
    ui32 m_triCount = 0;
};

#endif // VoxelModelMesh_h__
#pragma once
#ifndef VoxelModelMesh_h__
#define VoxelModelMesh_h__

#include <Vorb\types.h>
#include <Vorb\graphics\gtypes.h>

class VoxelModelVertex {
public:
    VoxelModelVertex(f32v3 pos, color3 color, f32v3 normal):
        pos(pos),
        color(color),
        normal(normal) {}
    f32v3 pos;
    color3 color;
    f32v3 normal;
};

class VoxelModelMesh {
public:
    VoxelModelMesh():
        m_vbo(NULL),
        m_ibo(NULL),
        m_indCount(0)  
    { /* Empty */ }

    VoxelModelMesh(VGVertexBuffer vbo, VGIndexBuffer ibo, ui32 indCount):
        m_vbo(vbo),
        m_ibo(ibo),
        m_indCount(indCount)            
    { /* Empty */ }

    void bind() const;

    VGVertexBuffer getVBO() const { return m_vbo; }
    VGIndexBuffer getIBO() const { return m_ibo; }
    ui32 getIndexCount() const { return m_indCount; }
    
private:
    VGVertexBuffer m_vbo;
    VGIndexBuffer m_ibo;
    ui32 m_indCount;
};

#endif // VoxelModelMesh_h__
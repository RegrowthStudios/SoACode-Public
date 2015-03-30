#pragma once

#ifndef GasGiantRenderer_h__
#define GasGiantRenderer_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/GLProgram.h>

DECL_VG(class GLProgramManager);
DECL_VIO(class IOManager);

class GasGiantVertex {
public:
    GasGiantVertex():
        position(0, 0, 0),
        normal(0, 0, 0),
        uv(0, 0)
    {}

    GasGiantVertex(f32v3 position, f32v3 normal, f32v2 uv):
        position(position),
        normal(normal) 
    {}

    f32v3 position;
    f32v3 normal;
    f32v2 uv;
};

class GasGiantMesh {
public:
    GLuint numIndices;
    GLuint numVertices;
    VGVertexBuffer vbo;
    VGIndexBuffer ibo;
    VGTexture colorBandLookup;
    VGUniform unWVP;
    VGUniform unColorBandLookup;

    void bind();
};

class GasGiantRenderer {
public:
    GasGiantRenderer();
    ~GasGiantRenderer();

    void drawGasGiant(f32m4& mvp);

    void setColorBandLookupTexture(VGTexture texture) { m_mesh->colorBandLookup = texture; }

    void reloadShader();

private:
    void initMesh();
    vg::ShaderSource createShaderCode(std::vector<const cString>& filesToDelete, const vg::ShaderType& stage, const vio::IOManager& iom, const cString path, const cString defines = nullptr);

    vg::GLProgram* m_shaderProgram;
    GasGiantMesh* m_mesh;

    
};

#endif
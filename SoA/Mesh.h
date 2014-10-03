// 
//  Mesh.h
//  Vorb Engine
//
//  Created by Ben Arnold on 30/9/2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides a basic mesh implementation for drawing
//  batches of primitives in an efficient manner.
//
#pragma once

#ifndef MESH_H_
#define MESH_H_

#include <vector>
#include "SamplerState.h"
#include "DepthState.h"
#include "RasterizerState.h"

namespace vorb {
namespace core {

enum class PrimitiveType {
    TRIANGLES = GL_TRIANGLES,
    LINES = GL_LINES,
    POINTS = GL_POINTS
};

enum class MeshUsage {
    STATIC = GL_STATIC_DRAW,
    DYNAMIC = GL_DYNAMIC_DRAW,
    STREAM = GL_STREAM_DRAW
};

struct MeshVertex {
    f32v3 position;
    f32v2 uv;
    ColorRGBA8 color;
};

// TODO(Ben): This belongs in Vorb.h or VorbErrors.h
enum class VorbError { 
    NONE,
    INVALID_PARAMETER,
    INVALID_STATE
};

class Mesh
{
public:

    Mesh();
    ~Mesh();

    // Frees all resources
    void destroy();

    // Initializes the mesh
    // @param primitiveType: type of primitive for this mesh
    // @param isIndexed: true when using glDrawElements, false for
    // glDrawArrays
    void init(PrimitiveType primitiveType, bool isIndexed);

    // Reserves a specific number of vertices and indices
    // @param numVertices: the number of vertices to reserve
    // @param numIndices: the number of indices to reserve
    void reserve(int numVertices, int numIndices = 0);

    // Draws the mesh
    void draw();

    // Adds vertices to the mesh
    // @param _vertices: vector of vertices to add
    void addVertices(const std::vector<MeshVertex>& vertices);

    // Adds indexed vertices to the mesh
    // @param _vertices: vector of vertices to add
    // @param _indices: vector of indices to add
    void addVertices(const std::vector<MeshVertex>& vertices,
                        const std::vector<ui32>& indices);

    // Uploads all data to the GPU and clears the local buffers
    void uploadAndClearLocal(MeshUsage usage = MeshUsage::STATIC);
    // Uploads all data to the GPU and keeps the local buffers
    void uploadAndKeepLocal(MeshUsage usage = MeshUsage::STATIC);

    // Setters
    void setModelMatrix(const f32m4& modelMatrix) { _modelMatrix = modelMatrix; }

    // Getters
    f32m4 getModelMatrix() const { return _modelMatrix; }
    int getNumVertices() const { return _vertices.size(); }
    int getNumPrimitives() const;

    // Default shader source
    static const cString defaultVertexShaderSource;
    static const cString defaultFragmentShaderSource;
    static const std::vector<std::pair<nString, ui32> > defaultShaderAttributes;

private:
    void upload(MeshUsage usage);
    void createVertexArray();

    f32m4 _modelMatrix;

    ui32 _vbo;
    ui32 _vao;
    ui32 _ibo;

    bool _isIndexed;
    bool _isUploaded;

    PrimitiveType _primitiveType;

    int _numVertices;
    int _numIndices;

    std::vector <MeshVertex> _vertices;
    std::vector <ui32> _indices;
};

}
}
#endif //MESH_H_
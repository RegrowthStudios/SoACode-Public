// 
//  Mesh.h
//  Vorb
//
//  Created by Ben Arnold on 30/9/2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides a basic mesh implementation for drawing
//  batches of primitives in an efficient manner.
//

#pragma once
#include <vector>
#include "SamplerState.h"
#include "DepthState.h"
#include "RasterizerState.h"

namespace vorb {

enum class PrimitiveType { TRIANGLES = GL_TRIANGLES, LINES = GL_LINES, POINTS = GL_POINTS };

struct MeshVertex {
    f32v3 position;
    f32v2 uv;
    ColorRGBA8 color;
};

//TODO: This belongs in Vorb.h or VorbErrors.h
enum class VorbError { NONE, INVALID_PARAMETER, INVALID_STATE };

class Mesh
{
public:
    
    Mesh();
    ~Mesh();

    //Initializes the mesh
    //\Param primitiveType: type of primitive for this mesh
    void init(PrimitiveType primitiveType, bool isIndexed);

    //Reserves a specific number of vertices
    //\Param numVertices: the number of vertices to reserve
    void reserve(int numVertices);

    //Draws the mesh
    //\Param viewProjectionMatrix: the combined view and projection matrix for the scene
    //\Param ss: sampler state, that holds texture info
    //\param ds: depth state, which holds depth info
    //\Param rs: rasterizer state, which holds culling info
    void draw(const f32m4& viewProjectionMatrix, 
              const SamplerState* ss = &SamplerState::LINEAR_WRAP,
              const DepthState* ds = &DepthState::NONE,
              const RasterizerState* rs = &RasterizerState::CULL_NONE);

    //Adds vertices to the mesh
    //\Param _vertices: vector of vertices to add
    void addVertices(const std::vector<MeshVertex>& _vertices);
 
    //Adds indexed vertices to the mesh
    //\Param _vertices: vector of vertices to add
    //\Param _indices: vector of indices to add
    void addVertices(const std::vector<MeshVertex>& _vertices, const std::vector<ui32>& _indices);

    //setters
    void setModelMatrix(const f32m4& modelMatrix) { _modelMatrix = modelMatrix; }

    //getters
    f32m4 getModelMatrix() const { return _modelMatrix; }
    int getNumVertices() const { return _vertices.size(); }
    int getNumPrimitives() const;

private:
    void createVertexArray();

    f32m4 _modelMatrix;

    GLuint _vbo;
    GLuint _vao;

    PrimitiveType _primitiveType;

    std::vector <MeshVertex> _vertices;
    std::vector <ui32> _indices;

    bool _isIndexed;
};

}
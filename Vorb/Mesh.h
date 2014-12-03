// 
//  Mesh.h
//  Vorb Engine
//
//  Created by Ben Arnold on 30 Sep 2014
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

#include "DepthState.h"
#include "GLEnums.h"
#include "GLProgram.h"
#include "gtypes.h"
#include "RasterizerState.h"
#include "SamplerState.h"

namespace vorb {
    namespace core {
        struct MeshVertex {
        public:
            f32v3 position;
            f32v2 uv;
            color4 color;
        };

        // TODO(Ben): This belongs in Vorb.h or VorbErrors.h
        enum class VorbError {
            NONE,
            INVALID_PARAMETER,
            INVALID_STATE
        };

        class Mesh {
        public:
            Mesh();
            ~Mesh();

            /// Frees all resources
            void destroy();

            /// Initializes the mesh
            /// @param primitiveType: type of primitive for this mesh
            /// @param isIndexed: true when using glDrawElements, false for
            /// glDrawArrays
            void init(vg::PrimitiveType primitiveType, bool isIndexed);

            /// Reserves a specific number of vertices and indices
            /// @param numVertices: the number of vertices to reserve
            /// @param numIndices: the number of indices to reserve
            void reserve(i32 numVertices, i32 numIndices = 0);

            /// Draws the mesh
            void draw();

            /// Adds vertices to the mesh
            /// @param _vertices: vector of vertices to add
            void addVertices(const std::vector<MeshVertex>& vertices);

            /// Adds indexed vertices to the mesh
            /// @param _vertices: vector of vertices to add
            /// @param _indices: vector of indices to add
            void addVertices(const std::vector<MeshVertex>& vertices, const std::vector<ui32>& indices);

            /// Uploads all data to the GPU and clears the local buffers
            void uploadAndClearLocal(vg::BufferUsageHint usage = vg::BufferUsageHint::STATIC_DRAW);
            /// Uploads all data to the GPU and keeps the local buffers
            void uploadAndKeepLocal(vg::BufferUsageHint usage = vg::BufferUsageHint::STATIC_DRAW);

            /************************************************************************/
            /* Setters                                                              */
            /************************************************************************/
            void setModelMatrix(const f32m4& modelMatrix) {
                _modelMatrix = modelMatrix;
            }

            /************************************************************************/
            /* Getters                                                              */
            /************************************************************************/
            f32m4 getModelMatrix() const {
                return _modelMatrix;
            }
            i32 getNumVertices() const {
                return _vertices.size();
            }
            int getNumPrimitives() const;

            static const cString defaultVertexShaderSource; ///< Default vertex shader code
            static const cString defaultFragmentShaderSource; ///< Default fragment shader code
            static const std::vector<vg::GLProgram::AttributeBinding> defaultShaderAttributes; ///< Default attribute locations

        private:
            void upload(vg::BufferUsageHint usage);
            void createVertexArray();

            f32m4 _modelMatrix;

            VGVertexBuffer _vbo;
            VGVertexArray _vao;
            VGIndexBuffer _ibo;

            bool _isIndexed;
            bool _isUploaded;

            vg::PrimitiveType _primitiveType;

            i32 _numVertices;
            i32 _numIndices;

            std::vector<MeshVertex> _vertices;
            std::vector<ui32> _indices;
        };
    }
}
namespace vcore = vorb::core;

#endif //MESH_H_

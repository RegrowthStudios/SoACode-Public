///
/// VoxelMesh.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 24 Dec 2014
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Graphics data for a voxel mesh
///

#pragma once

#ifndef VoxelMesh_h__
#define VoxelMesh_h__

#include <Vorb/gtypes.h>

/// Mesh data in a specific direction
struct VoxelFaceDirectionMesh {
public:
    VGVertexBuffer verts = 0; ///< Vertex data
    VGVertexArray vertexDeclarationSolid = 0; ///< Binding information for solid blocks
    VGVertexArray vertexDeclarationTransparent = 0; ///< Binding information for transparent blocks

    ui32 numSolid = 0; ///< Number of solid faces
    ui32 numTransparent = 0; ///< Number of transparent faces
};

/// Mesh data for solid and transparent blocks
struct VoxelMesh {
public:
    VoxelFaceDirectionMesh meshes[6]; ///< Voxel mesh data for all 6 faces
    VGIndexBuffer indices = 0; ///< Reference to quad index buffer
};

#endif // VoxelMesh_h__
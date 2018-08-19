///
/// DualContouringMesher.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 15 Jun 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// 
///

#pragma once

#ifndef DualContouringMesher_h__
#define DualContouringMesher_h__

#include "Octree.h"
#include "VoxelModelMesh.h"

class VoxelMatrix;

// TODO(Ben): I don't understand this
// Source: http://ngildea.blogspot.com/2014/11/implementing-dual-contouring.html
class DualContouringMesher {
public:
    static void genMatrixMesh(const VoxelMatrix& matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices);
private:

};

#endif // DualContouringMesher_h__
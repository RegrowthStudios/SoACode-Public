///
/// DualContouringMesher.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 15 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
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

class DualContouringMesher {
public:
    static void genMatrixMesh(const VoxelMatrix& matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices);
private:

};

#endif // DualContouringMesher_h__
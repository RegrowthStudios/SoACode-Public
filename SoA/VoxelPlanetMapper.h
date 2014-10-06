// 
//  VoxelPlanetMapper.h
//  Vorb Engine
//
//  Created by Ben Arnold on 6 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides a way to map voxels to a sphere.
//

#pragma once
#include "IVoxelMapper.h"

namespace vorb {
namespace voxel {


//relative rotations for the face transitions
//0 = top, 1 = left, 2 = right, 3 = front, 4 = back, 5 = bottom
const int FaceTransitions[6][6] = { 
    { 0, 1, -1, 0, 2, 0 }, //top
    { -1, 0, 0, 0, 0, 1 }, //left
    { 1, 0, 0, 0, 0, -1 }, //right
    { 0, 0, 0, 0, 0, 0 }, //front
    { 2, 0, 0, 0, 0, 2 }, //back
    { 0, -1, 1, 0, 2, 0 } }; //bottom

const int FaceNeighbors[6][4] = {
    { 2, 4, 1, 3 }, //top
    { 3, 0, 4, 5 }, //left
    { 4, 0, 3, 5 }, //right
    { 2, 0, 1, 5 }, //front
    { 1, 0, 2, 5 }, //back
    { 2, 3, 1, 4 } }; //bottom

// 6 faces, 4 rotations, i j r
//could just do rotation % 2 but repeating works fine
const int FaceCoords[6][4][3] = {
    { { 2, 0, 1 }, { 0, 2, 1 }, { 2, 0, 1 }, { 0, 2, 1 } },  //top
    { { 1, 2, 0 }, { 2, 1, 0 }, { 1, 2, 0 }, { 2, 1, 0 } }, //left
    { { 1, 2, 0 }, { 2, 1, 0 }, { 1, 2, 0 }, { 2, 1, 0 } }, //right
    { { 1, 0, 2 }, { 0, 1, 2 }, { 1, 0, 2 }, { 0, 1, 2 } }, //front
    { { 1, 0, 2 }, { 0, 1, 2 }, { 1, 0, 2 }, { 0, 1, 2 } }, //back
    { { 2, 0, 1 }, { 0, 2, 1 }, { 2, 0, 1 }, { 0, 2, 1 } } }; //bottom

// 6 faces, 4 rotations, ioff joff
//determined by taking the base case for i and j direction, and then rotating it 3 times, recording the new i j directions
const int FaceSigns[6][4][2] = {
    { { 1, 1 }, { 1, -1 }, { -1, -1 }, { -1, 1 } }, //top
    { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } }, //left
    { { -1, -1 }, { -1, 1 }, { 1, 1 }, { 1, -1 } }, //right
    { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } }, //front
    { { -1, -1 }, { -1, 1 }, { 1, 1 }, { 1, -1 } }, //back
    { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } } }; //bottom

const int FaceRadialSign[6] = { 1, -1, 1, 1, -1, -1 };


class VoxelPlanetMapData : public VoxelMapData
{
public:
    VoxelPlanetMapData(int Face, int Ipos, int Jpos, int Rot){
        face = Face;
        ipos = Ipos;
        jpos = Jpos;
        rotation = Rot;
    }
    // Used to get the directory path for chunks, based on which planet face
    nString getFilePath() {
        return "f" + std::to_string(face) + "/";
    }
    int face;
    int rotation;
};

class VoxelPlanetMapper : public IVoxelMapper
{
public:
    VoxelPlanetMapper();
    ~VoxelPlanetMapper();

    i32v3 getWorldCoords(VoxelMapData* voxelGridData);

private:
    

};


}
}
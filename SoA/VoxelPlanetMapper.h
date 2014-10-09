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
    void getIterationConstants(int& jStart, int& jMult, int& jEnd, int& jInc, int& kStart, int& kMult, int& kEnd, int& kInc) {
        switch (rotation){ //we use rotation value to un-rotate the chunk data
            case 0: //no rotation
                jStart = 0;
                kStart = 0;
                jEnd = kEnd = CHUNK_WIDTH;
                jInc = kInc = 1;
                jMult = CHUNK_WIDTH;
                kMult = 1;
                break;
            case 1: //up is right
                jMult = 1;
                jStart = CHUNK_WIDTH - 1;
                jEnd = -1;
                jInc = -1;
                kStart = 0;
                kEnd = CHUNK_WIDTH;
                kInc = 1;
                kMult = CHUNK_WIDTH;
                break;
            case 2: //up is down
                jMult = CHUNK_WIDTH;
                jStart = CHUNK_WIDTH - 1;
                kStart = CHUNK_WIDTH - 1;
                jEnd = kEnd = -1;
                jInc = kInc = -1;
                kMult = 1;
                break;
            case 3: //up is left
                jMult = 1;
                jStart = 0;
                jEnd = CHUNK_WIDTH;
                jInc = 1;
                kMult = CHUNK_WIDTH;
                kStart = CHUNK_WIDTH - 1;
                kEnd = -1;
                kInc = -1;
                break;
        }
    }

    void getChunkGridPos(int& iPos, int& jPos) {
        int idir = FaceSigns[face][rotation][0];
        int jdir = FaceSigns[face][rotation][1];
       
        iPos = ipos*idir;
        jPos = jpos*jdir;

        if (rotation % 2) { //when rotation%2 i and j must switch
            int tmp = iPos;
            iPos = jPos;
            jPos = tmp;
        }
    }

    void getVoxelGridPos(int& iPos, int& jPos) {
        //used for tree coords
        jPos = jpos * CHUNK_WIDTH * FaceSigns[face][rotation][0];
        iPos = ipos * CHUNK_WIDTH * FaceSigns[face][rotation][1];
        //swap em if rot%2
        if (rotation % 2){
            int tmp = iPos;
            iPos = jPos;
            jPos = tmp;
        }
    }

    void getGenerationIterationConstants(int& ipos, int& jpos, int& rpos, int& idir, int& jdir, int& rdir) {
        ipos = FaceCoords[face][rotation][0];
        jpos = FaceCoords[face][rotation][1];
        rpos = FaceCoords[face][rotation][2];
        idir = FaceSigns[face][rotation][0];
        jdir = FaceSigns[face][rotation][1];
        rdir = FaceRadialSign[face];

    }

    VoxelMapData* getNewNeighborData(const i32v2& ijOffset) {
        return new VoxelPlanetMapData(face, 
                                                       ipos + ijOffset.x,
                                                       jpos + ijOffset.y,
                                                       rotation);
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

    VoxelMapData* getNewVoxelMapData() {
        return new VoxelPlanetMapData(0, 0, 0, 0);
    }

    VoxelMapData* getNewVoxelMapData(VoxelMapData* copy) {
        VoxelPlanetMapData* newData = new VoxelPlanetMapData(0, 0, 0, 0);
        *newData = *static_cast<VoxelPlanetMapData*>(copy);
        return newData;
    }

private:
    

};


}
}
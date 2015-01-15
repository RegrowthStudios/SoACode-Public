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

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// TODO: Yeah... I dunno
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
    VoxelPlanetMapData() {};
    VoxelPlanetMapData(int Face, int Ipos, int Jpos, int Rot){
        face = Face;
        ipos = Ipos;
        jpos = Jpos;
        rotation = Rot;
    }
    void set(int Face, int Ipos, int Jpos, int Rot) {
        face = Face;
        ipos = Ipos;
        jpos = Jpos;
        rotation = Rot;
    }
    void getIterationConstants(OUT int& jStart, OUT int& jMult, OUT int& jEnd, OUT int& jInc, OUT int& kStart, OUT int& kMult, OUT int& kEnd, OUT int& kInc) {
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

    void getChunkGridPos(OUT int& iPos, OUT int& jPos) {
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

    f32v3 getWorldNormal(float radius) {
        float i = FaceCoords[face][rotation][0];
        float j = FaceCoords[face][rotation][1];
        float r = FaceCoords[face][rotation][2];
        float idir = FaceSigns[face][rotation][0];
        float jdir = FaceSigns[face][rotation][1];
        float rdir = FaceRadialSign[face];
        f32v3 position;
        position[i] = ipos * idir * CHUNK_WIDTH;
        position[j] = jpos * jdir * CHUNK_WIDTH;
        position[r] = radius * rdir;
        return glm::normalize(position);
    }
    f64v3 getWorldNormal(f64 radius) {
        f64 i = FaceCoords[face][rotation][0];
        f64 j = FaceCoords[face][rotation][1];
        f64 r = FaceCoords[face][rotation][2];
        f64 idir = FaceSigns[face][rotation][0];
        f64 jdir = FaceSigns[face][rotation][1];
        f64 rdir = FaceRadialSign[face];
        f64v3 position;
        position[i] = ipos * idir * CHUNK_WIDTH;
        position[j] = jpos * jdir * CHUNK_WIDTH;
        position[r] = radius * rdir;
        return glm::normalize(position);
    }

    void getVoxelGridPos(OUT int& iPos, OUT int& jPos) {
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

    void getGenerationIterationConstants(OUT int& ipos, OUT int& jpos, OUT int& rpos, OUT int& idir, OUT int& jdir, OUT int& rdir) {
        ipos = FaceCoords[face][rotation][0];
        jpos = FaceCoords[face][rotation][1];
        rpos = FaceCoords[face][rotation][2];
        idir = FaceSigns[face][rotation][0];
        jdir = FaceSigns[face][rotation][1];
        rdir = FaceRadialSign[face];
    }

    // TODO(Ben): This is jank, copied from old code
    f64q calculateVoxelToSpaceQuat(const f64v3& facePosition, f64 worldRadius) {

        int ipos, rpos, jpos;
        double incI, incJ, magnitude;
        glm::dvec3 v1, v2, v3;
        glm::vec3 tangent, biTangent;
        //Imagine the cube unfolded. Each face is like the following image for calculating tangent and bitangent
        // _____________
        // |           |
        // |        j  |
        // |       --->|
        // |     |i    |
        // |_____V_____|
        ipos = vvox::FaceCoords[face][rotation][0];
        jpos = vvox::FaceCoords[face][rotation][1];
        rpos = vvox::FaceCoords[face][rotation][2];
        incI = vvox::FaceSigns[face][rotation][0];
        incJ = vvox::FaceSigns[face][rotation][1];
        v1[ipos] = incI * facePosition.z;
        v1[jpos] = incJ * facePosition.x;
        v1[rpos] = vvox::FaceRadialSign[face] * worldRadius;
        f64v3 worldPosition = (facePosition.y + worldRadius)*glm::normalize(v1);

        incI *= 100;
        incJ *= 100;
        v1 = worldPosition;
        //need v2 and v3 for computing tangent and bitangent
        v2[ipos] = (double)worldPosition[ipos];
        v2[jpos] = (double)worldPosition[jpos] + incJ;
        v2[rpos] = worldPosition[rpos];
        v3[ipos] = (double)worldPosition[ipos] + incI;
        v3[jpos] = (double)worldPosition[jpos];
        v3[rpos] = worldPosition[rpos];
        //normalize them all
        v1 = glm::normalize(v1);
        v2 = glm::normalize(v2);
        v3 = glm::normalize(v3);
        tangent = glm::vec3(glm::normalize(v2 - v1));
        biTangent = glm::vec3(glm::normalize(v3 - v1));
        f64m4 worldRotationMatrix;
        worldRotationMatrix[0] = f64v4(tangent, 0);
        worldRotationMatrix[1] = f64v4(v1, 0);
        worldRotationMatrix[2] = f64v4(biTangent, 0);
        worldRotationMatrix[3] = f64v4(0, 0, 0, 1);
      
        return glm::quat_cast(worldRotationMatrix);
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
    VoxelPlanetMapper(int gridWidth);
    ~VoxelPlanetMapper();

    i32v3 getWorldCoords(VoxelMapData* voxelGridData);

    VoxelMapData* getNewVoxelMapData() {
        return new VoxelPlanetMapData(0, 0, 0, 0);
    }

    VoxelMapData* getNewVoxelMapData(const VoxelMapData* copy) {
        VoxelPlanetMapData* newData = new VoxelPlanetMapData(0, 0, 0, 0);
        *newData = *static_cast<const VoxelPlanetMapData*>(copy);
        return newData;
    }

    VoxelMapData* getNewRelativeData(const VoxelMapData* relative, const i32v2& ijOffset) {
        const VoxelPlanetMapData* mapData = static_cast<const VoxelPlanetMapData*>(relative);

       

        VoxelMapData* newMapData = new VoxelPlanetMapData(mapData->face,
                                                          mapData->ipos,
                                                          mapData->jpos,
                                                          mapData->rotation);
        offsetPosition(newMapData, ijOffset);

        return newMapData;
    }

    void offsetPosition(VoxelMapData* vmapData, const i32v2& ijOffset) {
        VoxelPlanetMapData* mapData = static_cast<VoxelPlanetMapData*>(vmapData);

        mapData->ipos += ijOffset.x;
        mapData->jpos += ijOffset.y;
        int newFace = mapData->face;

        if (mapData->ipos < -_halfGridWidth) {
            mapData->ipos += _gridWidth;

            newFace = FaceNeighbors[mapData->face][(1 + mapData->rotation) % 4];
            mapData->rotation += FaceTransitions[mapData->face][newFace];
            if (mapData->rotation < 0){ mapData->rotation += 4; } else{ mapData->rotation %= 4; }
        } else if (mapData->ipos > _halfGridWidth) {
            mapData->ipos -= _gridWidth;

            newFace = FaceNeighbors[mapData->face][(3 + mapData->rotation) % 4];
            mapData->rotation += FaceTransitions[mapData->face][newFace];
            if (mapData->rotation < 0){ mapData->rotation += 4; } else{ mapData->rotation %= 4; }
        }

        if (mapData->jpos < -_halfGridWidth) {
            mapData->jpos += _gridWidth;

            newFace = FaceNeighbors[mapData->face][(2 + mapData->rotation) % 4];
            mapData->rotation += FaceTransitions[mapData->face][newFace];
            if (mapData->rotation < 0){ mapData->rotation += 4; } else{ mapData->rotation %= 4; }
        } else if (mapData->jpos > _halfGridWidth) {
            mapData->jpos -= _gridWidth;

            newFace = FaceNeighbors[mapData->face][mapData->rotation];
            mapData->rotation += FaceTransitions[mapData->face][newFace];
            if (mapData->rotation < 0){ mapData->rotation += 4; } else{ mapData->rotation %= 4; }
        }

        mapData->face = newFace;
    }

private:
    int _gridWidth;
    int _halfGridWidth;

};


}
}
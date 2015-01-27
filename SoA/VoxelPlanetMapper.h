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



class VoxelPlanetMapData : public VoxelMapData
{
public:
    VoxelPlanetMapData() {};
    VoxelPlanetMapData(int Face, const i32v2& pos, int Rot){
        face = Face;
        chunkPos = pos;
        rotation = Rot;
    }
    void set(int Face, const i32v2& pos, int Rot) {
        face = Face;
        chunkPos = pos;
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


    void getGenerationIterationConstants(OUT int& ipos, OUT int& jpos, OUT int& rpos, OUT int& idir, OUT int& jdir, OUT int& rdir) {
        /*     ipos = FaceCoords[face][rotation][0];
             jpos = FaceCoords[face][rotation][1];
             rpos = FaceCoords[face][rotation][2];
             idir = FaceSigns[face][rotation][0];
             jdir = FaceSigns[face][rotation][1];
             rdir = FaceRadialSign[face];*/
    }

    f64q calculateVoxelToSpaceQuat(const f64v3& facePosition, f64 worldRadius) {
        #define OFFSET 1000
        f64v3 v1 = getWorldNormal(worldRadius);
        // Temporarily offset to right
        chunkPos.x += OFFSET;
        f64v3 v2 = getWorldNormal(worldRadius);
        chunkPos.x -= OFFSET;
        // Temporarily offset to back
        chunkPos.y += OFFSET;
        f64v3 v3 = getWorldNormal(worldRadius);
        chunkPos.y -= OFFSET;
      
        //normalize them all
        v1 = glm::normalize(v1);
        v2 = glm::normalize(v2);
        v3 = glm::normalize(v3);
        f64v3 tangent = glm::normalize(v2 - v1);
        f64v3 biTangent = glm::normalize(v3 - v1);
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
                                                          mapData->chunkPos,
                                                          mapData->rotation);
        offsetPosition(newMapData, ijOffset);

        return newMapData;
    }

    void offsetPosition(VoxelMapData* vmapData, const i32v2& ijOffset) {
        VoxelPlanetMapData* mapData = static_cast<VoxelPlanetMapData*>(vmapData);

        mapData->chunkPos += ijOffset;
        int newFace = mapData->face;
        
        if (mapData->chunkPos.y < -_halfGridWidth) {
            mapData->chunkPos.y += _gridWidth;

            newFace = FACE_NEIGHBORS[mapData->face][(1 + mapData->rotation) % 4];
            mapData->rotation += FACE_TRANSITIONS[mapData->face][newFace];
            if (mapData->rotation < 0){ mapData->rotation += 4; } else{ mapData->rotation %= 4; }
        } else if (mapData->chunkPos.y > _halfGridWidth) {
            mapData->chunkPos.y -= _gridWidth;

            newFace = FACE_NEIGHBORS[mapData->face][(3 + mapData->rotation) % 4];
            mapData->rotation += FACE_TRANSITIONS[mapData->face][newFace];
            if (mapData->rotation < 0){ mapData->rotation += 4; } else{ mapData->rotation %= 4; }
        }

        if (mapData->chunkPos.x < -_halfGridWidth) {
            mapData->chunkPos.x += _gridWidth;

            newFace = FACE_NEIGHBORS[mapData->face][(2 + mapData->rotation) % 4];
            mapData->rotation += FACE_TRANSITIONS[mapData->face][newFace];
            if (mapData->rotation < 0){ mapData->rotation += 4; } else{ mapData->rotation %= 4; }
        } else if (mapData->chunkPos.x > _halfGridWidth) {
            mapData->chunkPos.x -= _gridWidth;

            newFace = FACE_NEIGHBORS[mapData->face][mapData->rotation];
            mapData->rotation += FACE_TRANSITIONS[mapData->face][newFace];
            if (mapData->rotation < 0){ mapData->rotation += 4; } else{ mapData->rotation %= 4; }
        }

        mapData->face = newFace;
    }

private:
    int _gridWidth;
    int _halfGridWidth;

};

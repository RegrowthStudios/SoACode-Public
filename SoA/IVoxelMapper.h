// 
//  IVoxelMapper.h
//  Vorb Engine
//
//  Created by Ben Arnold on 6 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides an interface for VoxelMappers
//
// 
#pragma once
#include "Constants.h"

// TODO: Should this really be here?
namespace vorb {
    namespace voxel {

        class VoxelMapData {
        public:
            // Used to get the directory path for chunks. By default there is no distinction so it returns ""
            virtual nString getFilePath() {
                return "";
            }
            virtual void getIterationConstants(int& jStart, int& jMult, int& jEnd, int& jInc, int& kStart, int& kMult, int& kEnd, int& kInc) {
                jStart = 0;
                kStart = 0;
                jEnd = kEnd = CHUNK_WIDTH;
                jInc = kInc = 1;
                jMult = CHUNK_WIDTH;
                kMult = 1;
            }
            virtual void getChunkGridPos(int& iPos, int& jPos) {
                iPos = ipos;
                jPos = jpos;
            }
            virtual void getVoxelGridPos(int& iPos, int& jPos) {
                iPos = ipos * CHUNK_WIDTH;
                jPos = jpos * CHUNK_WIDTH;
            }

            virtual void getGenerationIterationConstants(int& ipos, int& jpos, int& rpos, int& idir, int& jdir, int& rdir) {
                ipos = 2;
                jpos = 0;
                rpos = 1;
                idir = 1;
                jdir = 1;
                rdir = 1;
            }

            int ipos; //front-back axis
            int jpos; //left-right axis
        };

        class IVoxelMapper {
        public:
            virtual VoxelMapData* getNewVoxelMapData() {
                return new VoxelMapData();
            }
            virtual VoxelMapData* getNewVoxelMapData(const VoxelMapData* copy) {
                VoxelMapData* newData = new VoxelMapData;
                *newData = *copy;
                return newData;
            }

            // Generates a new voxelMapData with ijOffset relative to the relative
            virtual VoxelMapData* getNewRelativeData(const VoxelMapData* relative, const i32v2& ijOffset) {
                VoxelMapData* newData = new VoxelMapData();
                newData->ipos = relative->ipos + ijOffset.x;
                newData->jpos = relative->jpos + ijOffset.y;
                return newData;
            }

            virtual void offsetPosition(VoxelMapData* mapData, const i32v2& ijOffset) {
                mapData->ipos += ijOffset.x;
                mapData->jpos += ijOffset.y;
            }

        protected:

        };

    }
}
namespace vvox = vorb::voxel;
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
#pragma once

namespace vorb{
namespace voxel{

class VoxelMapData {
public:
    // Used to get the directory path for chunks. By default there is no distinction so it returns ""
    virtual nString getFilePath() {
        return "";
    }
protected:
    int ipos; //front-back axis
    int jpos; //left-right axis
};

class IVoxelMapper {
public:
        
protected:

};

}
}
#include "stdafx.h"
#include "VoxelPlanetMapper.h"

namespace vorb {
namespace voxel {

VoxelPlanetMapper::VoxelPlanetMapper(int gridWidth) :
    _gridWidth(gridWidth),
    _halfGridWidth(_gridWidth/2)
{
}


VoxelPlanetMapper::~VoxelPlanetMapper()
{
}

i32v3 VoxelPlanetMapper::getWorldCoords(VoxelMapData* voxelGridData) {
    VoxelPlanetMapData* vpmd = static_cast<VoxelPlanetMapData*>(voxelGridData);
    return i32v3(0);
}

}
}
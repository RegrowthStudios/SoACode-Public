///
/// VoxelPositionComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component that houses position on a voxel planet
///

#pragma once

#ifndef VoxelPositionComponent_h__
#define VoxelPositionComponent_h__

#include "VoxelPlanetMapper.h"

class VoxelPositionComponent {
public:
    f64v3 position = f64v3(0.0);
    f64q orientation;
    vvox::VoxelPlanetMapData mapData;
};

#endif // VoxelPositionComponent_h__
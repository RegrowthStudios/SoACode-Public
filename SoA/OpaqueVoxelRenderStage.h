/// 
///  OpaqueVoxelRenderStage.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements the render stage for opaque voxels.
///  Opaque voxels have no transparency.
///

#pragma once

#ifndef OpaqueVoxelRenderStage_h__
#define OpaqueVoxelRenderStage_h__

#include <Vorb/IRenderStage.h>

class GameRenderParams;
class Camera;
class MeshManager;

class OpaqueVoxelRenderStage : public vg::IRenderStage
{
public:
    /// Constructor which injects dependencies
    /// @param gameRenderParams: Shared parameters for rendering voxels
    OpaqueVoxelRenderStage(const GameRenderParams* gameRenderParams);

    /// Draws the render stage
    virtual void draw() override;
private:
    const GameRenderParams* _gameRenderParams; ///< Handle to some shared parameters
};

#endif // OpaqueVoxelRenderStage_h__
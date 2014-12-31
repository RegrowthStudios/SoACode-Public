/// 
///  TransparentVoxelRenderStage.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file provides the implementation of the transparent voxel
///  render stage. Transparent voxels have partial transparency, and
///  will be sorted and blended.
///

#pragma once

#ifndef TransparentVoxelRenderStage_h__
#define TransparentVoxelRenderStage_h__

#include <Vorb/IRenderStage.h>

class GameRenderParams;
class Camera;
class MeshManager;

class TransparentVoxelRenderStage : public vg::IRenderStage {
public:
    /// Conclassor which injects dependencies
    /// @param camera: The camera handle
    /// @param gameRenderParams: Shared parameters for rendering voxels
    /// @param meshManager: Handle to the class that holds meshes
    TransparentVoxelRenderStage(const GameRenderParams* gameRenderParams);

    /// Draws the render stage
    virtual void draw() override;
private:
    const GameRenderParams* _gameRenderParams; ///< Handle to some shared parameters
};

#endif // TransparentVoxelRenderStage_h__


/// 
///  LiquidVoxelRenderStage.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements the render stage for
///  liquid voxel rendering.
///

#pragma once

#ifndef LiquidVoxelRenderStage_h__
#define LiquidVoxelRenderStage_h__

#include "IRenderStage.h"

class MeshManager;
class GameRenderParams;

class LiquidVoxelRenderStage : public vg::IRenderStage
{
public:
    /// Constructor which injects dependencies
    /// @param camera: The camera handle
    /// @param gameRenderParams: Shared parameters for rendering voxels
    /// @param meshManager: Handle to the class that holds meshes
    LiquidVoxelRenderStage(const Camera* camera,
                           const GameRenderParams* gameRenderParams, 
                           const MeshManager* meshManager);
    /// Draws the render stage
    virtual void draw() override;
private:
    const MeshManager* _meshManager; ///< Holds the meshes to render
    const GameRenderParams* _gameRenderParams; ///< Some shared rendering parameters
};

#endif // LiquidVoxelRenderStage_h__
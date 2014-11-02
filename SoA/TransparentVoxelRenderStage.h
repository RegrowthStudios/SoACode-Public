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

#include "IRenderStage.h"

class GameRenderParams;
class Camera;
class MeshManager;

class TransparentVoxelRenderStage : public vg::IRenderStage {
public:
    /// Constructor which injects dependencies
    /// @param camera: The camera handle
    /// @param gameRenderParams: Shared parameters for rendering voxels
    /// @param meshManager: Handle to the class that holds meshes
    TransparentVoxelRenderStage(Camera* camera, GameRenderParams* gameRenderParams, MeshManager* meshManager);
    ~TransparentVoxelRenderStage();

    /// Draws the render stage
    virtual void draw() override;
private:
    GameRenderParams* _gameRenderParams; ///< Handle to some shared parameters
    MeshManager* _meshManager; ///< Stores the meshes we need to render
};

#endif // TransparentVoxelRenderStage_h__


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

#include "IRenderStage.h"

class GameRenderParams;
class Camera;
class MeshManager;

class OpaqueVoxelRenderStage : public vg::IRenderStage
{
public:
    /// Constructor which injects dependencies
    /// @param camera: The camera handle
    /// @param gameRenderParams: Shared parameters for rendering voxels
    /// @param meshManager: Handle to the class that holds meshes
    OpaqueVoxelRenderStage(Camera* camera, GameRenderParams* gameRenderParams, MeshManager* meshManager);

    /// Draws the render stage
    virtual void draw() override;
private:
    GameRenderParams* _gameRenderParams; ///< Handle to some shared parameters
    MeshManager* _meshManager; ///< Stores the meshes we need to render
};

#endif // OpaqueVoxelRenderStage_h__
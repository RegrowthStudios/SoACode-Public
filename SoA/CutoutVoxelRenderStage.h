/// 
///  CutoutVoxelRenderStage.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements the render stage for cutout voxels.
///  cutout voxels have pixels that are either fully opaque, or
///  fully transparent, and it's shader uses glDiscard to discard
///  transparent fragments.
///

#pragma once

#ifndef CutoutVoxelRenderStage_h__
#define CutoutVoxelRenderStage_h__

#include "IRenderStage.h"

class GameRenderParams;
class Camera;
class MeshManager;

class CutoutVoxelRenderStage : public vg::IRenderStage {
public:
    /// Constructor which injects dependencies
    /// @param camera: The camera handle
    /// @param gameRenderParams: Shared parameters for rendering voxels
    /// @param meshManager: Handle to the class that holds meshes
    CutoutVoxelRenderStage(const Camera* camera,
                           const GameRenderParams* gameRenderParams,
                           const MeshManager* meshManager);

    /// Draws the render stage
    virtual void draw() override;
private:
    const GameRenderParams* _gameRenderParams; ///< Handle to some shared parameters
    const MeshManager* _meshManager; ///< Stores the meshes we need to render
};

#endif // CutoutVoxelRenderStage_h__


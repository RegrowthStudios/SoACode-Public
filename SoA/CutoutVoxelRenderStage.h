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

#include <Vorb/graphics/IRenderStage.h>

class GameRenderParams;
class Camera;
class MeshManager;

class CutoutVoxelRenderStage : public vg::IRenderStage {
public:
    /// Constructor which injects dependencies
    /// @param gameRenderParams: Shared parameters for rendering voxels
    /// @param meshManager: Handle to the class that holds meshes
    CutoutVoxelRenderStage(const GameRenderParams* gameRenderParams);

    /// Draws the render stage
    virtual void render() override;
private:
    const GameRenderParams* _gameRenderParams; ///< Handle to some shared parameters
};

#endif // CutoutVoxelRenderStage_h__
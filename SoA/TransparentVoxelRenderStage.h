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

#include <Vorb/graphics/IRenderStage.h>

class GameRenderParams;
class Camera;
class MeshManager;

class TransparentVoxelRenderStage : public vg::IRenderStage {
public:
    TransparentVoxelRenderStage();

    void init(const GameRenderParams* gameRenderParams);

    /// Draws the render stage
    virtual void render() override;
private:
    const GameRenderParams* m_gameRenderParams = nullptr; ///< Handle to some shared parameters
};

#endif // TransparentVoxelRenderStage_h__


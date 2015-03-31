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

#include <Vorb/graphics/IRenderStage.h>

class MeshManager;
class GameRenderParams;

class LiquidVoxelRenderStage : public vg::IRenderStage
{
public:
    /// Constructor which injects dependencies
    /// @param gameRenderParams: Shared parameters for rendering voxels
    /// @param meshManager: Handle to the class that holds meshes
    LiquidVoxelRenderStage(const GameRenderParams* gameRenderParams);
    /// Draws the render stage
    virtual void render() override;
private:
    void drawChunk(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP);
    
    const GameRenderParams* m_gameRenderParams; ///< Some shared rendering parameters
};

#endif // LiquidVoxelRenderStage_h__
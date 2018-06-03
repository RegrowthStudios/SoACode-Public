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

#include <Vorb/graphics/GLProgram.h>

class MeshManager;
class GameRenderParams;
class ChunkRenderer;

class LiquidVoxelRenderStage : public IRenderStage {
public:
    void hook(ChunkRenderer* renderer, const GameRenderParams* gameRenderParams);
    /// Draws the render stage
    virtual void render(const Camera* camera) override;
private:
    ChunkRenderer* m_renderer;
    const GameRenderParams* m_gameRenderParams = nullptr; ///< Some shared rendering parameters
};

#endif // LiquidVoxelRenderStage_h__

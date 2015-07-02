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

#include <Vorb/graphics/GLProgram.h>

class Camera;
class ChunkRenderer;
class GameRenderParams;
class MeshManager;

class CutoutVoxelRenderStage : public IRenderStage {
public:
    void hook(ChunkRenderer* renderer, const GameRenderParams* gameRenderParams);

    /// Draws the render stage
    virtual void render(const Camera* camera) override;
private:
    ChunkRenderer* m_renderer;
    const GameRenderParams* m_gameRenderParams; ///< Handle to some shared parameters
};

#endif // CutoutVoxelRenderStage_h__
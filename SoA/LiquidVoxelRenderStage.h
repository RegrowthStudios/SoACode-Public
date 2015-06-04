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

class LiquidVoxelRenderStage : public IRenderStage {
public:
    LiquidVoxelRenderStage();

    void init(const GameRenderParams* gameRenderParams);
    /// Draws the render stage
    virtual void render() override;
private:
    
    const GameRenderParams* m_gameRenderParams = nullptr; ///< Some shared rendering parameters
};

#endif // LiquidVoxelRenderStage_h__

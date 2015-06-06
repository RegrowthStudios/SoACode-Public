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

#include <Vorb/graphics/GLProgram.h>

class GameRenderParams;
class Camera;
class MeshManager;

class OpaqueVoxelRenderStage : public IRenderStage
{
public:
    void hook(const GameRenderParams* gameRenderParams);

    /// Draws the render stage
    virtual void render(const Camera* camera) override;
private:
    vg::GLProgram m_program;
    const GameRenderParams* m_gameRenderParams; ///< Handle to some shared parameters
};

#endif // OpaqueVoxelRenderStage_h__
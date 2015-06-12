///
/// ChunkGridRenderStage.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 14 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// This file provides the chunk grid render stage,
/// which is a debug rendering stage for chunk state
///

#pragma once

#ifndef ChunkGridRenderStage_h__
#define ChunkGridRenderStage_h__

#include "IRenderStage.h"

#include <Vorb/graphics/GLProgram.h>

class GameRenderParams;
class ChunkMemoryManager;

class ChunkGridRenderStage : public IRenderStage {
public:
    void hook(const GameRenderParams* gameRenderParams);

    // Draws the render stage
    void setChunks(const ChunkMemoryManager* cmm) { m_chunkMemoryManager = cmm; }
    virtual void render(const Camera* camera) override;
private:
    vg::GLProgram m_program;
    const GameRenderParams* m_gameRenderParams = nullptr; ///< Handle to some shared parameters
    const ChunkMemoryManager* m_chunkMemoryManager = nullptr;
};

#endif // ChunkGridRenderStage_h__

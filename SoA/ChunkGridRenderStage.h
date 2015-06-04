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

#include <Vorb/graphics/IRenderStage.h>

class GameRenderParams;
class ChunkMemoryManager;

class ChunkGridRenderStage : public vg::IRenderStage {
public:
    ChunkGridRenderStage();
    ~ChunkGridRenderStage();

    void init(const GameRenderParams* gameRenderParams);

    // Draws the render stage
    void setChunks(const ChunkMemoryManager* cmm) { m_chunkMemoryManager = cmm; }
    virtual void render() override;
private:
    const GameRenderParams* m_gameRenderParams = nullptr; ///< Handle to some shared parameters
    const ChunkMemoryManager* m_chunkMemoryManager = nullptr;
};

#endif // ChunkGridRenderStage_h__

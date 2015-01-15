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
class ChunkSlot;

class ChunkGridRenderStage : public vg::IRenderStage {
public:
    /// Constructor which injects dependencies
    /// @param gameRenderParams: Shared parameters for rendering voxels
    /// @param chunkSlots: The chunk slots that we need to render boxes for
    ChunkGridRenderStage(const GameRenderParams* gameRenderParams);
    ~ChunkGridRenderStage();

    // Draws the render stage
    void setChunkSlots(const std::vector<ChunkSlot>* chunkSlots) { _chunkSlots = chunkSlots; }
    virtual void draw() override;
private:
    const GameRenderParams* _gameRenderParams; ///< Handle to some shared parameters
    const std::vector<ChunkSlot>* _chunkSlots = nullptr;
};

#endif // ChunkGridRenderStage_h__
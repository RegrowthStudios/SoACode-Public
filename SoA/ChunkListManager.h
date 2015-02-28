///
/// ChunkListManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 26 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Manages chunk state lists
///

#pragma once

#ifndef ChunkListManager_h__
#define ChunkListManager_h__

#include "Chunk.h"

class ChunkListManager {
public:
    /// Adds a chunk to the setupList
    /// @param chunk: the chunk to add
    void addToSetupList(Chunk* chunk);
    /// Adds a chunk to the loadList
    /// @param chunk: the chunk to add
    void addToLoadList(Chunk* chunk);
    /// Adds a chunk to the meshList
    /// @param chunk: the chunk to add
    void addToMeshList(Chunk* chunk);
    /// Adds a chunk to the generateList
    /// @param chunk: the chunk to add
    void addToGenerateList(Chunk* chunk);
    /// Adds a chunk to the freeWait list
    void addToFreeWaitList(Chunk* chunk);

    void sortLists();

    /// Stack of chunks needing setup
    std::vector<Chunk*> setupList;
    /// Stack of chunks that need to be meshed on the threadPool
    std::vector<Chunk*> meshList;
    /// Stack of chunks that need to be sent to the IO thread
    std::vector<Chunk*> loadList;
    /// Stack of chunks needing generation
    std::vector<Chunk*> generateList;
    /// Chunks waiting to be freed
    std::vector<Chunk*> freeWaitingChunks;
};

#endif // ChunkListManager_h__

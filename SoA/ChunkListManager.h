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

class NChunk;

class ChunkListManager {
public:
    void addToSetupList(NChunk* chunk);
    void addToLoadList(NChunk* chunk);
    void addToMeshList(NChunk* chunk);
    void addToGenerateList(NChunk* chunk);
    void addToFreeWaitList(NChunk* chunk);
    /// Sorts all lists in descending order of distance
    void sortLists();

    /// Stack of chunks needing setup
    std::vector<NChunk*> setupList;
    /// Stack of chunks that need to be meshed on the threadPool
    std::vector<NChunk*> meshList;
    /// Stack of chunks that need to be sent to the IO thread
    std::vector<NChunk*> loadList;
    /// Stack of chunks needing generation
    std::vector<NChunk*> generateList;
    /// Chunks waiting to be freed
    std::vector<NChunk*> freeWaitingChunks;
};

#endif // ChunkListManager_h__

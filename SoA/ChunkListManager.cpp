#include "stdafx.h"
#include "ChunkListManager.h"

#include "NChunk.h"

void ChunkListManager::addToSetupList(NChunk* chunk) {
    chunk->addToChunkList(&setupList);
}

void ChunkListManager::addToLoadList(NChunk* chunk) {
    chunk->_state = ChunkStates::LOAD;
    chunk->addToChunkList(&loadList);
}

void ChunkListManager::addToMeshList(NChunk* chunk) {
    if (!chunk->queuedForMesh) {
        chunk->addToChunkList(&meshList);
        chunk->queuedForMesh = true;
    }
}

void ChunkListManager::addToGenerateList(NChunk* chunk) {
    chunk->_state = ChunkStates::GENERATE;
    chunk->addToChunkList(&generateList);
}

void ChunkListManager::addToFreeWaitList(NChunk* chunk) {
    chunk->freeWaiting = true;
    freeWaitingChunks.push_back(chunk);
}

bool sortChunksAscending(const NChunk* a, const NChunk* b) {
    return a->distance2 < b->distance2;
}

bool sortChunksDescending(const NChunk* a, const NChunk* b) {
    return a->distance2 > b->distance2;
}

void ChunkListManager::sortLists() {
    std::sort(setupList.begin(), setupList.end(), sortChunksDescending);
    std::sort(meshList.begin(), meshList.end(), sortChunksDescending);
    std::sort(loadList.begin(), loadList.end(), sortChunksDescending);
}

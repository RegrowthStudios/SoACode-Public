#include "stdafx.h"
#include "ChunkListManager.h"

void ChunkListManager::addToSetupList(Chunk* chunk) {
    chunk->addToChunkList(&setupList);
}

void ChunkListManager::addToLoadList(Chunk* chunk) {
    chunk->_state = ChunkStates::LOAD;
    chunk->addToChunkList(&loadList);
}

void ChunkListManager::addToMeshList(Chunk* chunk) {
    if (!chunk->queuedForMesh) {
        chunk->addToChunkList(&meshList);
        chunk->queuedForMesh = true;
    }
}

void ChunkListManager::addToGenerateList(Chunk* chunk) {
    chunk->_state = ChunkStates::GENERATE;
    chunk->addToChunkList(&generateList);
}

void ChunkListManager::addToFreeWaitList(Chunk* chunk) {
    chunk->freeWaiting = true;
    freeWaitingChunks.push_back(chunk);
}

bool sortChunksAscending(const Chunk* a, const Chunk* b) {
    return a->distance2 < b->distance2;
}

bool sortChunksDescending(const Chunk* a, const Chunk* b) {
    return a->distance2 > b->distance2;
}

void ChunkListManager::sortLists() {
    std::sort(setupList.begin(), setupList.end(), sortChunksDescending);
    std::sort(meshList.begin(), meshList.end(), sortChunksDescending);
    std::sort(loadList.begin(), loadList.end(), sortChunksDescending);
}

#include "stdafx.h"
#include "ChunkListManager.h"

void ChunkListManager::addToSetupList(Chunk* chunk) {

}

void ChunkListManager::addToLoadList(Chunk* chunk) {

}

void ChunkListManager::addToMeshList(Chunk* chunk) {

}

void ChunkListManager::addToGenerateList(Chunk* chunk) {

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

#include "stdafx.h"
#include "FloraTask.h"

#include <algorithm>

#include <Vorb/ThreadPool.h>

#include "Chunk.h"

bool searchVector(const std::vector <ui16>& allChunkOffsets, ui16 offset) {
    return (std::find(allChunkOffsets.begin(), allChunkOffsets.end(), offset) != allChunkOffsets.end());
}

// We use this to make 00111 == 0
#define SUBTRACT_OFFSET 7

inline i32 getXOffset(ui16 offset) {
#define X_SHIFT 10 
    return (i32)(offset >> X_SHIFT) - SUBTRACT_OFFSET;
}

inline i32 getYOffset(ui16 offset) {
#define Y_SHIFT 5 
    return (i32)((offset >> Y_SHIFT) & 0x1F) - SUBTRACT_OFFSET;
}

inline i32 getZOffset(ui16 offset) {
    return (i32)(offset & 0x1F) - SUBTRACT_OFFSET;
}

i32v3 FloraTask::getChunkOffset(ui16 offset) {
    return i32v3(getXOffset(offset), getYOffset(offset), getZOffset(offset));
}

void FloraTask::execute(WorkerData* workerData) {

    //generatedTreeNodes = new GeneratedTreeNodes();
    //generatedTreeNodes->startChunkGridPos = chunk->getChunkPosition().pos;

    //// Lazily initialize flora generator
    //if (workerData->floraGenerator == nullptr) {
    //    workerData->floraGenerator = new FloraGenerator();
    //}

    //isSuccessful = false;
    //if (workerData->floraGenerator->generateFlora(chunk, 
    //    generatedTreeNodes->wnodes, 
    //    generatedTreeNodes->lnodes)) {
    //    isSuccessful = true;
    //    // Sort by chunk to minimize locking in the main thread
    //    if (generatedTreeNodes->lnodes.size() || generatedTreeNodes->wnodes.size()) {
    //        sortNodes(generatedTreeNodes);
    //    }
    //    // Get all chunk offsets so its easy to check if they are active in the main thread
    //    std::vector <ui16> allChunkOffsets;
    //    if (generatedTreeNodes->wnodes.size()) {
    //        ui16 lastPos = generatedTreeNodes->wnodes[0].chunkOffset;
    //        allChunkOffsets.push_back(lastPos);
    //        for (size_t i = 1; i < generatedTreeNodes->wnodes.size(); i++) {
    //            if (generatedTreeNodes->wnodes[i].chunkOffset != lastPos) {
    //                lastPos = generatedTreeNodes->wnodes[i].chunkOffset;
    //                allChunkOffsets.push_back(lastPos);
    //            }
    //        }
    //    }
    //    if (generatedTreeNodes->lnodes.size()) {
    //        ui16 lastPos = generatedTreeNodes->lnodes[0].chunkOffset;
    //        if (!searchVector(allChunkOffsets, lastPos)) {
    //            allChunkOffsets.push_back(lastPos);
    //        }
    //        for (size_t i = 0; i < generatedTreeNodes->lnodes.size(); i++) {
    //            if (generatedTreeNodes->lnodes[i].chunkOffset != lastPos) {
    //                lastPos = generatedTreeNodes->lnodes[i].chunkOffset;
    //                if (!searchVector(allChunkOffsets, lastPos)) {
    //                    allChunkOffsets.push_back(lastPos);
    //                }
    //            }
    //        }
    //    }
    //    // Construct chunk positions using all chunk offsets
    //    const i32v3& startPos = generatedTreeNodes->startChunkGridPos;
    //    for (auto& it : allChunkOffsets) {
    //        generatedTreeNodes->allChunkPositions.emplace_back(startPos.x + getXOffset(it),
    //                                                           startPos.y + getYOffset(it),
    //                                                           startPos.z + getZOffset(it));
    //    }
    //}
}

//bool sortByChunk(TreeNode& a, TreeNode& b) {
//    return (a.chunkOffset < b.chunkOffset);
//}

void FloraTask::sortNodes(GeneratedTreeNodes* generatedTreeNodes) {
  //  std::sort(generatedTreeNodes->lnodes.begin(), generatedTreeNodes->lnodes.end(), sortByChunk);
  //  std::sort(generatedTreeNodes->wnodes.begin(), generatedTreeNodes->wnodes.end(), sortByChunk);
}
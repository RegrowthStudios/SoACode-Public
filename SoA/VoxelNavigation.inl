///
/// VoxelNavigator.inl
/// Vorb
///
/// Created by Benjamin Arnold on 26 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// This file implements the Voxel Navigation functions with
/// thread safety.
///

#pragma once

#ifndef VoxelNavigation_inl__
#define VoxelNavigation_inl__

// TODO(Ben): Generalize Chunk and voxel stuff
#include "Chunk.h"
#include "VoxelUtils.h"

namespace vorb {
    namespace voxel {

        /// Locks a chunk and unlocks the currently locked chunk. Keeps track
        /// of which chunk is locked.
        /// @param chunkToLock: Chunk that needs to be locked
        /// @param lockedChunk: Currently locked chunk, or nullptr if no chunk
        /// is locked. Will be set to chunkToLock if chunkToLock is locked.
        inline void lockChunk(Chunk* chunkToLock, Chunk*& lockedChunk) {
            if (chunkToLock == lockedChunk) return;
            if (lockedChunk) lockedChunk->unlock();
            lockedChunk = chunkToLock;
            lockedChunk->lock();
        }

        inline int getLeftBlockData(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int x, int& nextBlockIndex, Chunk*& owner) {
            if (x > 0) {
                owner = chunk;
                nextBlockIndex = blockIndex - 1;
                return chunk->getBlockData(nextBlockIndex);
            } else if (chunk->left && chunk->left->isAccessible) {
                owner = chunk->left;
                nextBlockIndex = blockIndex + CHUNK_WIDTH - 1;
                lockChunk(owner, lockedChunk);
                return owner->getBlockData(nextBlockIndex);
            }
            return -1;
        }

        inline int getRightBlockData(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int x, int& nextBlockIndex, Chunk*& owner) {
            if (x < CHUNK_WIDTH - 1) {
                owner = chunk;
                nextBlockIndex = blockIndex + 1;
                return chunk->getBlockData(nextBlockIndex);
            } else if (chunk->right && chunk->right->isAccessible) {
                owner = chunk->right;
                nextBlockIndex = blockIndex - CHUNK_WIDTH + 1;
                lockChunk(owner, lockedChunk);
                return owner->getBlockData(nextBlockIndex);
            }
            return -1;
        }

        inline int getFrontBlockData(Chunk* chunk, Chunk*& lockedChunk, int blockIndex) {
            if (getZFromBlockIndex(blockIndex) < CHUNK_WIDTH - 1) {
                return chunk->getBlockData(blockIndex + CHUNK_WIDTH);
            } else if (chunk->front && chunk->front->isAccessible) {
                lockChunk(chunk->front, lockedChunk);
                return chunk->front->getBlockData(blockIndex - CHUNK_LAYER + CHUNK_WIDTH);
            }
            return -1;
        }

        inline int getFrontBlockData(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int z, int& nextBlockIndex, Chunk*& owner) {
            if (z < CHUNK_WIDTH - 1) {
                owner = chunk;
                nextBlockIndex = blockIndex + CHUNK_WIDTH;
                return chunk->getBlockData(nextBlockIndex);
            } else if (chunk->front && chunk->front->isAccessible) {
                owner = chunk->front;
                nextBlockIndex = blockIndex - CHUNK_LAYER + CHUNK_WIDTH;
                lockChunk(owner, lockedChunk);
                return owner->getBlockData(nextBlockIndex);
            }
            return -1;
        }

        inline int getBackBlockData(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int z, int& nextBlockIndex, Chunk*& owner) {
            if (z > 0) {
                owner = chunk;
                nextBlockIndex = blockIndex - CHUNK_WIDTH;
                return chunk->getBlockData(nextBlockIndex);
            } else if (chunk->back && chunk->back->isAccessible) {
                owner = chunk->back;
                nextBlockIndex = blockIndex + CHUNK_LAYER - CHUNK_WIDTH;
                lockChunk(owner, lockedChunk);
                return owner->getBlockData(nextBlockIndex);
            }
            return -1;
        }

        inline int getBottomBlockData(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int y, int& nextBlockIndex, Chunk*& owner) {
            if (y > 0) {
                owner = chunk;
                nextBlockIndex = blockIndex - CHUNK_LAYER;
                return chunk->getBlockData(nextBlockIndex);
            } else if (chunk->bottom && chunk->bottom->isAccessible) {
                owner = chunk->bottom;
                nextBlockIndex = blockIndex + CHUNK_SIZE - CHUNK_LAYER;
                lockChunk(owner, lockedChunk);
                return owner->getBlockData(nextBlockIndex);
            }
            return -1;
        }

        inline int getBottomBlockData(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int y) {
            if (y > 0) {
                return chunk->getBlockData(blockIndex - CHUNK_LAYER);
            } else if (chunk->bottom && chunk->bottom->isAccessible) {
                lockChunk(chunk->bottom, lockedChunk);
                return chunk->bottom->getBlockData(blockIndex + CHUNK_SIZE - CHUNK_LAYER);
            }
            return -1;
        }

        inline int getTopBlockData(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int y, int& nextBlockIndex, Chunk*& owner) {
            if (y < CHUNK_WIDTH - 1) {
                owner = chunk;
                nextBlockIndex = blockIndex + CHUNK_LAYER;
                return chunk->getBlockData(nextBlockIndex);
            } else if (chunk->top && chunk->top->isAccessible) {
                owner = chunk->top;
                nextBlockIndex = blockIndex - CHUNK_SIZE + CHUNK_LAYER;
                lockChunk(owner, lockedChunk);
                return owner->getBlockData(nextBlockIndex);
            }
            return -1;
        }
    }
}

namespace vvoxel = vorb::voxel;
#endif // VoxelNavigation_inl__
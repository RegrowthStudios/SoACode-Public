#include "stdafx.h"
#include "Chunk.h"

#include "VoxelSpaceConversions.h"


void Chunk::init(const ChunkPosition3D& pos) {
    memset(neighbors, 0, sizeof(neighbors));  
    numNeighbors = 0u;
    distance2 = FLT_MAX;
    m_chunkPosition = pos;
    m_voxelPosition = VoxelSpaceConversions::chunkToVoxel(m_chunkPosition);
    m_genQueryData.current = nullptr;
    remeshFlags = 1;
    gridData = nullptr;
    // Maybe do this after its done being used in grid?
    std::vector<ChunkQuery*>().swap(m_genQueryData.pending);
    numBlocks = 0;
    hasCreatedMesh = false;
    accessor = nullptr;
    m_id.x = pos.pos.x;
    m_id.y = pos.pos.y;
    m_id.z = pos.pos.z;
}

void Chunk::initAndFillEmpty(const ChunkPosition3D& pos, vvox::VoxelStorageState /*= vvox::VoxelStorageState::INTERVAL_TREE*/) {
    init(pos);
    IntervalTree<ui16>::LNode blockNode;
    IntervalTree<ui16>::LNode tertiaryNode;
    blockNode.set(0, CHUNK_SIZE, 0);
    tertiaryNode.set(0, CHUNK_SIZE, 0);
    blocks.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, &blockNode, 1);
    tertiary.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, &tertiaryNode, 1);
}

void Chunk::setRecyclers(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler) {
    blocks.setArrayRecycler(shortRecycler);
    tertiary.setArrayRecycler(shortRecycler);
}

void Chunk::updateContainers() {
    blocks.update(mutex);
    tertiary.update(mutex);
}
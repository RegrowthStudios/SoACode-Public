#include "stdafx.h"
#include "Chunk.h"

#include "VoxelSpaceConversions.h"


void Chunk::init(WorldCubeFace face) {
    memset(neighbors, 0, sizeof(neighbors));
    distance2 = FLT_MAX;
    // Get position from ID
    m_chunkPosition.x = m_id.x;
    m_chunkPosition.y = m_id.y;
    m_chunkPosition.z = m_id.z;
    m_chunkPosition.face = face;
    m_voxelPosition = VoxelSpaceConversions::chunkToVoxel(m_chunkPosition);
    m_genQueryData.current = nullptr;
    remeshFlags = 1;
    gridData = nullptr;
    m_inLoadRange = false;
    // Maybe do this after its done being used in grid?
    std::vector<ChunkQuery*>().swap(m_genQueryData.pending);
    numBlocks = 0;
    hasCreatedMesh = false;
    genLevel = ChunkGenLevel::GEN_NONE;
    pendingGenLevel = ChunkGenLevel::GEN_NONE;
}

void Chunk::initAndFillEmpty(WorldCubeFace face, vvox::VoxelStorageState /*= vvox::VoxelStorageState::INTERVAL_TREE*/) {
    init(face);
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
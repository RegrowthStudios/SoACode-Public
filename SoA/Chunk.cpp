#include "stdafx.h"
#include "Chunk.h"

#include "VoxelSpaceConversions.h"


void Chunk::init(WorldCubeFace face) {
    // Get position from ID
    m_chunkPosition.pos = i32v3(m_id.x, m_id.y, m_id.z);
    m_chunkPosition.face = face;
    m_voxelPosition = VoxelSpaceConversions::chunkToVoxel(m_chunkPosition);
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
    blocks.update(dataMutex);
    tertiary.update(dataMutex);
}
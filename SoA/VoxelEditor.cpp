#include "stdafx.h"
#include "VoxelEditor.h"

#include "BlockData.h"
#include "Chunk.h"
#include "ChunkGrid.h"
#include "ChunkUpdater.h"
#include "Item.h"
#include "VoxelNavigation.inl"
#include "VoxelSpaceConversions.h"

void VoxelEditor::editVoxels(ChunkGrid& grid, ItemStack* block) {
    if (m_startPosition.x == INT_MAX || m_endPosition.x == INT_MAX) {
        return;
    }

    switch (m_currentTool) {
    case EDITOR_TOOLS::AABOX:
        placeAABox(grid, block);
        break;
    case EDITOR_TOOLS::LINE:
        placeLine(grid, block);
        break;
    }
}

void VoxelEditor::placeAABox(ChunkGrid& grid, ItemStack* block) {
    BlockID blockID;
    int soundNum = 0;
    int yStart, yEnd;
    int zStart, zEnd;
    int xStart, xEnd;

    i32v3 start = m_startPosition;
    i32v3 end = m_endPosition;

    //Set up iteration bounds
    if (start.y < end.y) {
        yStart = start.y;
        yEnd = end.y;
    } else {
        yEnd = start.y;
        yStart = end.y;
    }

    if (start.z < end.z) {
        zStart = start.z;
        zEnd = end.z;
    } else {
        zEnd = start.z;
        zStart = end.z;
    }

    if (start.x < end.x) {
        xStart = start.x;
        xEnd = end.x;
    } else {
        xEnd = start.x;
        xStart = end.x;
    }

    // Keep track of which chunk is locked
    Chunk* chunk = nullptr;
    ChunkID currentID(0xffffffffffffffff);
    bool locked = false;

    std::map<ChunkID, ChunkHandle> modifiedChunks;

    for (int y = yStart; y <= yEnd; y++) {
        for (int z = zStart; z <= zEnd; z++) {
            for (int x = xStart; x <= xEnd; x++) {
                i32v3 chunkPos = VoxelSpaceConversions::voxelToChunk(i32v3(x, y, z));
                ChunkID id(chunkPos);
                if (id != currentID) {
                    currentID = id;
                    if (chunk) {
                        if (locked) {
                            chunk->dataMutex.unlock();
                            locked = false;
                        }
                    }
                    // Only need to aquire once
                    auto& it = modifiedChunks.find(currentID);
                    if (it == modifiedChunks.end()) {
                        chunk = modifiedChunks.insert(std::make_pair(currentID, grid.accessor.acquire(id))).first->second;
                    } else {
                        chunk = it->second;
                    }
                    if (chunk->isAccessible) {
                        chunk->dataMutex.lock();
                        locked = true;
                    }
                }
                if (locked) {
                    i32v3 pos = i32v3(x, y, z) - chunkPos * CHUNK_WIDTH;
                    int voxelIndex = pos.x + pos.y * CHUNK_LAYER + pos.z * CHUNK_WIDTH;
                    blockID = chunk->blocks.get(voxelIndex);

                    if (!block) {
                        // Breaking blocks
                    } else {
                        // Placing blocks
                        block->count--;

                        // ChunkUpdater::placeBlock(chunk, )
                        ChunkUpdater::placeBlockNoUpdate(chunk, voxelIndex, block->pack->operator[](block->id).blockID);
                        if (block->count == 0) {
                            if (locked) chunk->dataMutex.unlock();
                            for (auto& it : modifiedChunks) {
                                if (it.second->isAccessible) {
                                    it.second->DataChange(it.second);
                                }
                                it.second.release();
                            }
                            stopDragging();
                            return;
                        }
                        
                    }
                }
            }
        }
    }
    if (locked) chunk->dataMutex.unlock();
    for (auto& it : modifiedChunks) {
        if (it.second->isAccessible) {
            it.second->DataChange(it.second);
        }
        it.second.release();
    }
    stopDragging();
}

void VoxelEditor::stopDragging() {
    //This means we no longer have a selection box
    m_startPosition = i32v3(INT_MAX);
    m_endPosition = i32v3(INT_MAX);
}

void VoxelEditor::placeLine(ChunkGrid& grid, ItemStack* block) {

}

bool VoxelEditor::isEditing() {
    return (m_startPosition.x != INT_MAX && m_endPosition.x != INT_MAX);
}

void VoxelEditor::drawGuides(vg::GLProgram* program, const f64v3& cameraPos, const f32m4 &VP, int blockID)
{
    switch (m_currentTool) {
        case EDITOR_TOOLS::AABOX:
            const float BOX_PAD = 0.001f;

            i32v3 startPosition;
            startPosition.x = vmath::min(m_startPosition.x, m_endPosition.x);
            startPosition.y = vmath::min(m_startPosition.y, m_endPosition.y);
            startPosition.z = vmath::min(m_startPosition.z, m_endPosition.z);

            const i32v3 size = vmath::abs(m_endPosition - m_startPosition) + i32v3(1);

            if (blockID != 0){
          //      DrawWireBox(program, startPosition.x - BOX_PAD, startPosition.y - BOX_PAD, startPosition.z - BOX_PAD, size.x + BOX_PAD * 2, size.y + BOX_PAD * 2, size.z + BOX_PAD * 2, 2, cameraPos, VP, f32v4(0.0, 0.0, 1.0, 1.0));
          //      DrawWireBox(program, startPosition.x + BOX_PAD, startPosition.y + BOX_PAD, startPosition.z + BOX_PAD, size.x - BOX_PAD * 2, size.y - BOX_PAD * 2, size.z - BOX_PAD * 2, 2, cameraPos, VP, f32v4(0.0, 0.0, 1.0, 1.0));
            } else{
          //      DrawWireBox(program, startPosition.x - BOX_PAD, startPosition.y - BOX_PAD, startPosition.z - BOX_PAD, size.x + BOX_PAD * 2, size.y + BOX_PAD * 2, size.z + BOX_PAD * 2, 2, cameraPos, VP, f32v4(1.0, 0.0, 0.0, 1.0));
          //      DrawWireBox(program, startPosition.x + BOX_PAD, startPosition.y + BOX_PAD, startPosition.z + BOX_PAD, size.x - BOX_PAD * 2, size.y - BOX_PAD * 2, size.z - BOX_PAD * 2, 2, cameraPos, VP, f32v4(1.0, 0.0, 0.0, 1.0));
            }
            break;
    }
}
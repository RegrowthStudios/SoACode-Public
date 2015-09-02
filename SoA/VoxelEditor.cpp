#include "stdafx.h"
#include "VoxelEditor.h"

#include "BlockData.h"
#include "Chunk.h"
#include "ChunkUpdater.h"
#include "Item.h"
#include "VoxelNavigation.inl"

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
    Chunk* chunk = nullptr;
    int blockIndex = -1;
    int blockID;
    int soundNum = 0;
    int yStart, yEnd;
    int zStart, zEnd;
    int xStart, xEnd;

    i32v3 start = m_startPosition;
    i32v3 end = m_endPosition;

    bool breakBlocks = false;
    if (block == nullptr){
        breakBlocks = true;
    }

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
    Chunk* lockedChunk = nullptr;

   /* for (int y = yStart; y <= yEnd; y++) {
        for (int z = zStart; z <= zEnd; z++) {
            for (int x = xStart; x <= xEnd; x++) {

                chunkManager->getBlockAndChunk(i32v3(x, y, z), &chunk, blockIndex);

                if (chunk && chunk->isAccessible) {

                    blockID = chunk->getBlockIDSafe(lockedChunk, blockIndex);

                    if (breakBlocks) {
                        if (blockID != NONE && !(blockID >= LOWWATER && blockID <= FULLWATER)) {
                            if (soundNum < 50) GameManager::soundEngine->PlayExistingSound("BreakBlock", 0, 1.0f, 0, f64v3(x, y, z));
                            soundNum++;
                            ChunkUpdater::removeBlock(chunkManager, physicsEngine, chunk, lockedChunk, blockIndex, true);
                        }
                    } else {
                        if (blockID == NONE || (blockID >= LOWWATER && blockID <= FULLWATER) || (Blocks[blockID].isSupportive == 0)) {
                            if (soundNum < 50) GameManager::soundEngine->PlayExistingSound("PlaceBlock", 0, 1.0f, 0, f64v3(x, y, z));
                            soundNum++;
                            ChunkUpdater::placeBlock(chunk, lockedChunk, blockIndex, block->ID);
                            block->count--;
                            if (block->count == 0) {
                                stopDragging();
                                if (lockedChunk) lockedChunk->unlock();
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
    if (lockedChunk) lockedChunk->unlock(); */
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
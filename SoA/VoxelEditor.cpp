#include "stdafx.h"

#include "VoxelEditor.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "ChunkUpdater.h"
#include "Sound.h"
#include "BlockData.h"
#include "Item.h"


VoxelEditor::VoxelEditor() : _currentTool(EDITOR_TOOLS::AABOX), _startPosition(INT_MAX), _endPosition(INT_MAX) {
}

void VoxelEditor::editVoxels(Item *block) {
    if (_startPosition.x == INT_MAX || _endPosition.x == INT_MAX) {
        return;
    }

    switch (_currentTool) {
    case EDITOR_TOOLS::AABOX:
        placeAABox(block);
        break;
    case EDITOR_TOOLS::LINE:
        placeLine(block);
        break;
    }
}

void VoxelEditor::placeAABox(Item *block) {

    Chunk* chunk = nullptr;
    int blockIndex = -1, blockID;
    int soundNum = 0;
    int yStart, yEnd;
    int zStart, zEnd;
    int xStart, xEnd;

    ChunkManager* chunkManager = GameManager::chunkManager;

    //Coordinates relative to the ChunkManager
    i32v3 relStart = _startPosition - chunkManager->cornerPosition;
    i32v3 relEnd = _endPosition - chunkManager->cornerPosition;

    bool breakBlocks = false;
    if (block == nullptr){
        breakBlocks = true;
    }

    //Set up iteration bounds
    if (relStart.y < relEnd.y) {
        yStart = relStart.y;
        yEnd = relEnd.y;
    } else {
        yEnd = relStart.y;
        yStart = relEnd.y;
    }

    if (relStart.z < relEnd.z) {
        zStart = relStart.z;
        zEnd = relEnd.z;
    } else {
        zEnd = relStart.z;
        zStart = relEnd.z;
    }

    if (relStart.x < relEnd.x) {
        xStart = relStart.x;
        xEnd = relEnd.x;
    } else {
        xEnd = relStart.x;
        xStart = relEnd.x;
    }

    for (int y = yStart; y <= yEnd; y++) {
        for (int z = zStart; z <= zEnd; z++) {
            for (int x = xStart; x <= xEnd; x++) {

                chunkManager->getBlockAndChunk(i32v3(x, y, z), &chunk, blockIndex);

                if (chunk && chunk->isAccessible) {
                    blockID = chunk->getBlockID(blockIndex);

                    if (breakBlocks){
                        if (blockID != NONE && !(blockID >= LOWWATER && blockID <= FULLWATER)){
                            if (soundNum < 50) GameManager::soundEngine->PlayExistingSound("BreakBlock", 0, 1.0f, 0, f64v3(x, y, z) + f64v3(chunkManager->cornerPosition));
                            soundNum++;
                            ChunkUpdater::removeBlock(chunk, blockIndex, true);
                        }
                    } else {
                        if (blockID == NONE || (blockID >= LOWWATER && blockID <= FULLWATER) || (Blocks[blockID].isSupportive == 0))
                        {
                            if (soundNum < 50) GameManager::soundEngine->PlayExistingSound("PlaceBlock", 0, 1.0f, 0, f64v3(x, y, z) + f64v3(chunkManager->cornerPosition));
                            soundNum++;
                            ChunkUpdater::placeBlock(chunk, blockIndex, block->ID);
                            block->count--;
                            if (block->count == 0){
                                stopDragging();
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    stopDragging();
}

void VoxelEditor::stopDragging() {
    //This means we no longer have a selection box
    _startPosition = i32v3(INT_MAX);
    _endPosition = i32v3(INT_MAX);
}

void VoxelEditor::placeLine(Item *block) {

}

bool VoxelEditor::isEditing() {
    return (_startPosition.x != INT_MAX && _endPosition.x != INT_MAX);
}

void VoxelEditor::drawGuides(const f64v3& cameraPos, glm::mat4 &VP, int blockID)
{
    switch (_currentTool) {
        case EDITOR_TOOLS::AABOX:
            const float BOX_PAD = 0.001f;

            i32v3 startPosition;
            startPosition.x = MIN(_startPosition.x, _endPosition.x);
            startPosition.y = MIN(_startPosition.y, _endPosition.y);
            startPosition.z = MIN(_startPosition.z, _endPosition.z);

            const i32v3 size = glm::abs(_endPosition - _startPosition) + i32v3(1);

            if (blockID != 0){

                DrawWireBox(startPosition.x - BOX_PAD, startPosition.y - BOX_PAD, startPosition.z - BOX_PAD, size.x + BOX_PAD * 2, size.y + BOX_PAD * 2, size.z + BOX_PAD * 2, 2, cameraPos, VP, glm::vec4(0.0, 0.0, 1.0, 1.0));
                DrawWireBox(startPosition.x + BOX_PAD, startPosition.y + BOX_PAD, startPosition.z + BOX_PAD, size.x - BOX_PAD * 2, size.y - BOX_PAD * 2, size.z - BOX_PAD * 2, 2, cameraPos, VP, glm::vec4(0.0, 0.0, 1.0, 1.0));
            } else{
                DrawWireBox(startPosition.x - BOX_PAD, startPosition.y - BOX_PAD, startPosition.z - BOX_PAD, size.x + BOX_PAD * 2, size.y + BOX_PAD * 2, size.z + BOX_PAD * 2, 2, cameraPos, VP, glm::vec4(1.0, 0.0, 0.0, 1.0));
                DrawWireBox(startPosition.x + BOX_PAD, startPosition.y + BOX_PAD, startPosition.z + BOX_PAD, size.x - BOX_PAD * 2, size.y - BOX_PAD * 2, size.z - BOX_PAD * 2, 2, cameraPos, VP, glm::vec4(1.0, 0.0, 0.0, 1.0));
            }
            break;
    }
}
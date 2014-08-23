#include "stdafx.h"
#include "CAEngine.h"

#include "BlockData.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "ChunkUpdater.h"
#include "GameManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "VoxelUtils.h"


CAEngine::CAEngine() {
    memset(_blockUpdateFlagList, 0, sizeof(_blockUpdateFlagList));
    //temorary
    _lowIndex = LOWWATER; 
    _range = 100;
    _highIndex = _lowIndex + _range - 1;
}

void CAEngine::update(const ChunkManager &chunkManager) {
    static unsigned int frameCounter = 0;
    static unsigned int powderCounter = 0;
    static unsigned int waterCounter = 0;

    bool updatePowders = false;
    bool updateWater = false;

    if (powderCounter >= 4 || (powderCounter == 2 && physSpeedFactor >= 2.0)){
        if (isWaterUpdating) updatePowders = true;
        powderCounter = 0;
    }

    if (waterCounter >= 2 || (waterCounter == 1 && physSpeedFactor >= 2.0)){
        if (isWaterUpdating) updateWater = true;
        waterCounter = 0;
    }

    const ChunkSlot *allChunkSlots = chunkManager.getAllChunkSlots();

    Chunk* chunk;
    if (updateWater && updatePowders) {
        for (int i = 0; i < chunkManager.csGridSize; i++){
            _chunk = allChunkSlots[i].chunk;
            if (_chunk){
                updateSpawnerBlocks(frameCounter == 0); //spawners and sinks only right now
                updateLiquidBlocks();
                updatePowderBlocks();
            }
        }
    } else if (updateWater) {
        for (int i = 0; i < chunkManager.csGridSize; i++){
            _chunk = allChunkSlots[i].chunk;
            if (_chunk){
                updateLiquidBlocks();
            }
        }
    } else if (updatePowders) {
        for (int i = 0; i < chunkManager.csGridSize; i++){
            _chunk = allChunkSlots[i].chunk;
            if (_chunk){
                updateSpawnerBlocks(frameCounter == 0); //spawners and sinks only right now
                updatePowderBlocks();
            }
        }
    }

    frameCounter++;
    powderCounter++;
    waterCounter++;
    if (frameCounter == 3) frameCounter = 0;
}

void CAEngine::updateSpawnerBlocks(bool powders)
{
    int spawnerVal;
    int sinkVal;
    int c;
    glm::dvec3 physicsBlockPos;
    vector <GLushort> &activeBlocks = _chunk->spawnerBlocks;

    Chunk *bottom = _chunk->bottom;

    for (size_t i = 0; i < activeBlocks.size();){
        c = activeBlocks[i];

        const Block &block = _chunk->getBlock(c);

        spawnerVal = block.spawnerVal;
        sinkVal = block.sinkVal;
        if (spawnerVal == 0 && sinkVal == 0){
            activeBlocks[i] = activeBlocks.back();
            activeBlocks.pop_back();
            continue;
        }
        if (spawnerVal){
            if (_chunk->getBottomBlockData(c) == 0){
                if (c >= CHUNK_LAYER){
                    c = c - CHUNK_LAYER;
                    if (spawnerVal >= LOWWATER) {
                        ChunkUpdater::placeBlock(_chunk, c, spawnerVal);
                        ChunkUpdater::addBlockToUpdateList(_chunk, c);
                    } else if (powders){
                        physicsBlockPos = glm::dvec3((double)_chunk->position.x + c%CHUNK_WIDTH + 0.5, (double)_chunk->position.y + c / CHUNK_LAYER, (double)_chunk->position.z + (c%CHUNK_LAYER) / CHUNK_WIDTH + 0.5);
                        GameManager::physicsEngine->addPhysicsBlock(physicsBlockPos, spawnerVal);
                    }
                } else if (bottom && bottom->isAccessible){
                    c = c - CHUNK_LAYER + CHUNK_SIZE;
                    if (spawnerVal >= LOWWATER){
                        ChunkUpdater::placeBlock(bottom, c, spawnerVal);
                        ChunkUpdater::addBlockToUpdateList(bottom, c);
                    } else if (powders){
                        physicsBlockPos = glm::dvec3((double)bottom->position.x + c%CHUNK_WIDTH + 0.5, (double)bottom->position.y + c / CHUNK_LAYER, (double)bottom->position.z + (c%CHUNK_LAYER) / CHUNK_WIDTH + 0.5);
                        GameManager::physicsEngine->addPhysicsBlock(physicsBlockPos, spawnerVal);
                    }
                }
            }
        }
        if (sinkVal){
            if (GETBLOCKTYPE(_chunk->getTopBlockData(c)) == sinkVal){
                if (c + CHUNK_LAYER < CHUNK_SIZE){
                    c = c + CHUNK_LAYER;
                    _chunk->setBlockData(c, NONE); //TODO: This is incorrect, should call RemoveBlock or something similar
                    ChunkUpdater::addBlockToUpdateList(_chunk, c);
                    _chunk->changeState(ChunkStates::MESH);
                } else if (_chunk->top && _chunk->top->isAccessible){
                    c = c + CHUNK_LAYER - CHUNK_SIZE;
                    _chunk->top->setBlockData(c, NONE);
                    ChunkUpdater::addBlockToUpdateList(_chunk->top, c);
                    _chunk->top->changeState(ChunkStates::MESH);
                }
            }
        }
        i++;
    }
}

void CAEngine::updateLiquidBlocks()
{
    bool *activeUpdateList = _chunk->activeUpdateList;
    vector <GLushort> *blockUpdateList = _chunk->blockUpdateList[0];
    int actv = activeUpdateList[0];
    int size = blockUpdateList[actv].size(); 
    if (size == 0) return;
    activeUpdateList[0] = !(activeUpdateList[0]); //switch to other list
    int c, blockID;
    Uint32 i;

    for (i = 0; i < blockUpdateList[actv].size(); i++){
        c = blockUpdateList[actv][i];
        if (_blockUpdateFlagList[c] == 0){
            _usedUpdateFlagList.push_back(c);
            _blockUpdateFlagList[c] = 1;
            blockID = _chunk->getBlockID(c);
            if (blockID >= _lowIndex) liquidPhysics(c, blockID);
        }
    }
    blockUpdateList[actv].clear();

    for (Uint32 i = 0; i < _usedUpdateFlagList.size(); i++){
        _blockUpdateFlagList[_usedUpdateFlagList[i]] = 0;
    }
    _usedUpdateFlagList.clear();

}

void CAEngine::updatePowderBlocks()
{
    bool *activeUpdateList = _chunk->activeUpdateList;
    vector <GLushort> *blockUpdateList = _chunk->blockUpdateList[1];
    int actv = activeUpdateList[1];

    Uint32 size = blockUpdateList[actv].size();
    if (size != 0){
        activeUpdateList[1] = !(activeUpdateList[1]); //switch to other list
        int b;
        Uint32 i;

        for (i = 0; i < size; i++){ //powder
            b = blockUpdateList[actv][i];
            if (_blockUpdateFlagList[b] == 0){
                _usedUpdateFlagList.push_back(b);
                _blockUpdateFlagList[b] = 1;
                powderPhysics(b);
            }
        }

        blockUpdateList[actv].clear();

        for (i = 0; i < _usedUpdateFlagList.size(); i++){
            _blockUpdateFlagList[_usedUpdateFlagList[i]] = 0;
        }
        _usedUpdateFlagList.clear();
    }

    blockUpdateList = _chunk->blockUpdateList[2];
    actv = activeUpdateList[2];
    size = blockUpdateList[actv].size();

    if (size != 0){
        activeUpdateList[2] = !(activeUpdateList[2]);
        int b;
        Uint32 i;

        for (i = 0; i < size; i++){ //snow
            b = blockUpdateList[actv][i];
            if (_blockUpdateFlagList[b] == 0){
                _usedUpdateFlagList.push_back(b);
                _blockUpdateFlagList[b] = 1;
                if (_chunk->getBlock(b).physicsProperty == P_SNOW){ 
                    snowPhysics(b);
                }
            }
        }
        blockUpdateList[actv].clear(); 

        for (i = 0; i < _usedUpdateFlagList.size(); i++){
            _blockUpdateFlagList[_usedUpdateFlagList[i]] = 0;
        }
        _usedUpdateFlagList.clear();
    }
}

void CAEngine::liquidPhysics(i32 startBlockIndex, i32 startBlockID) {

    i32v3 pos = getPosFromBlockIndex(startBlockIndex);
    Chunk* owner;
    i32 nextIndex;
    i32 nextCoord;
    i32 blockID;
    i32 diff;
    i32 index = 0;
    Chunk* adjOwners[4] = {nullptr, nullptr, nullptr, nullptr};
    Chunk* adjAdjOwners[4] = { nullptr, nullptr, nullptr, nullptr };
    i32 adjIndices[4];
    i32 adjAdjIndices[4];
    i32 diffs[4];

    bool hasChanged = false;
    bool inFrustum = (!_chunk->mesh || _chunk->mesh->inFrustum);
    const i32v3 &position = _chunk->position;

    // Get the block index and owner for the bottom chunk
    if (pos.y > 0){
        nextIndex = startBlockIndex - CHUNK_LAYER;
        owner = _chunk;
    } else if (_chunk->bottom && _chunk->bottom->isAccessible){
        nextIndex = startBlockIndex + CHUNK_SIZE - CHUNK_LAYER;
        owner = _chunk->bottom;
    } else{
        return;
    }

    //Get the block ID
    blockID = owner->getBlockID(nextIndex);

    //If we are falling on an air block
    if (blockID == NONE){ 
        ChunkUpdater::placeBlockFromLiquidPhysics(owner, nextIndex, startBlockID);
        ChunkUpdater::removeBlockFromLiquidPhysics(_chunk, startBlockIndex);

        ChunkUpdater::updateNeighborStates(_chunk, pos, ChunkStates::WATERMESH);
        if (owner != _chunk) ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);

        if (startBlockID > _lowIndex + 10 && inFrustum) particleEngine.addParticles(1, glm::dvec3(position.x + pos.x, position.y + pos.y - 1.0, position.z + pos.z), 0, 0.1, 16665, 1111, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), Blocks[blockID].particleTex, 0.5f, 8);
        return;
    } else if (blockID >= _lowIndex && blockID < _highIndex) { //If we are falling on the same liquid
        //how much empty space there is
        diff = _highIndex - blockID;

        //if we cant completely fill in the empty space, fill it in as best we can
        if (startBlockID - _lowIndex + 1 > diff){
            startBlockID -= diff;
            ChunkUpdater::placeBlockFromLiquidPhysics(owner, nextIndex, blockID + diff);

            if (diff > 10 && inFrustum) particleEngine.addParticles(1, glm::dvec3(position.x + pos.x, position.y + pos.y - 1.0, position.z + pos.z), 0, 0.1, 16665, 1111, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), Blocks[blockID].particleTex, 0.5f, 8);
            hasChanged = 1;
            
            if (owner != _chunk) {
                owner->changeState(ChunkStates::WATERMESH);
                ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);
            }

        } else { //otherwise ALL liquid falls and we are done
            ChunkUpdater::placeBlockFromLiquidPhysics(owner, nextIndex, blockID + (startBlockID - _lowIndex + 1));
            ChunkUpdater::removeBlockFromLiquidPhysics(_chunk, startBlockIndex);

            ChunkUpdater::updateNeighborStates(_chunk, pos, ChunkStates::WATERMESH);
            if (owner != _chunk) ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);

            if (startBlockID > _lowIndex + 10 && inFrustum) particleEngine.addParticles(1, glm::dvec3(position.x + pos.x, position.y + pos.y - 1.0, position.z + pos.z), 0, 0.1, 16665, 1111, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), Blocks[blockID].particleTex, 0.5f, 8);
            return;
        }
    } else if (Blocks[blockID].waterBreak) { //destroy a waterBreak block, such as flora
        ChunkUpdater::removeBlock(owner, nextIndex, true);
        ChunkUpdater::placeBlockFromLiquidPhysics(owner, nextIndex, startBlockID);
        ChunkUpdater::removeBlockFromLiquidPhysics(_chunk, startBlockIndex);

        ChunkUpdater::updateNeighborStates(_chunk, pos, ChunkStates::WATERMESH);
        if (owner != _chunk) ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);

        if (startBlockID > _lowIndex + 10 && inFrustum) particleEngine.addParticles(1, glm::dvec3(position.x + pos.x, position.y + pos.y - 1.0, position.z + pos.z), 0, 0.1, 16665, 1111, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), Blocks[blockID].particleTex, 0.5f, 8);
        return;
    }

    //Left Direction
    if (pos.x > 0) {
        nextIndex = startBlockIndex - 1;
        owner = _chunk;
        nextCoord = pos.x - 1;
    } else if (_chunk->left && _chunk->left->isAccessible) {
        nextIndex = startBlockIndex - 1 + CHUNK_WIDTH;
        owner = _chunk->left;
        nextCoord = CHUNK_WIDTH - 1;
    } else {
        return;
    }

    blockID = owner->getBlockID(nextIndex);

    if (blockID == NONE || Blocks[blockID].waterBreak){ //calculate diffs
        diffs[index] = (startBlockID - (_lowIndex - 1));
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    } else if (blockID >= _lowIndex && blockID < _highIndex){ //tmp CANT FLOW THOUGH FULL WATER
        
        if (nextCoord > 0){ //extra flow. its the secret!
            adjAdjIndices[index] = nextIndex - 1;
            adjAdjOwners[index] = owner;
        } else if (owner->left && owner->left->isAccessible){
            adjAdjIndices[index] = nextIndex + CHUNK_WIDTH - 1;
            adjAdjOwners[index] = owner->left;
        }
        diffs[index] = (startBlockID - blockID);
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    }

    //Back Direction
    if (pos.z > 0) {
        nextIndex = startBlockIndex - CHUNK_WIDTH;
        owner = _chunk;
        nextCoord = pos.z - 1;
    } else if (_chunk->back && _chunk->back->isAccessible) {
        nextIndex = startBlockIndex - CHUNK_WIDTH + CHUNK_LAYER;
        owner = _chunk->back;
        nextCoord = CHUNK_WIDTH - 1;
    } else {
        return;
    }

    blockID = owner->getBlockID(nextIndex);

    if (blockID == NONE || Blocks[blockID].waterBreak){ //calculate diffs
        diffs[index] = (startBlockID - (_lowIndex - 1));
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    } else if (blockID >= _lowIndex && blockID < _highIndex){ //tmp CANT FLOW THOUGH FULL WATER

        if (nextCoord > 0){ //extra flow. its the secret!
            adjAdjIndices[index] = nextIndex - CHUNK_WIDTH;
            adjAdjOwners[index] = owner;
        } else if (owner->back && owner->back->isAccessible){
            adjAdjIndices[index] = nextIndex - CHUNK_WIDTH + CHUNK_LAYER;
            adjAdjOwners[index] = owner->back;
        }
        diffs[index] = (startBlockID - blockID);
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    }

    //Right Direction
    if (pos.x < CHUNK_WIDTH - 1) {
        nextIndex = startBlockIndex + 1;
        owner = _chunk;
        nextCoord = pos.x + 1;
    } else if (_chunk->right && _chunk->right->isAccessible) {
        nextIndex = startBlockIndex + 1 - CHUNK_WIDTH;
        owner = _chunk->right;
        nextCoord = 0;
    } else {
        return;
    }

    blockID = owner->getBlockID(nextIndex);

    if (blockID == NONE || Blocks[blockID].waterBreak){ //calculate diffs
        diffs[index] = (startBlockID - (_lowIndex - 1));
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    } else if (blockID >= _lowIndex && blockID < _highIndex){ //tmp CANT FLOW THOUGH FULL WATER

        if (nextCoord < CHUNK_WIDTH - 1){ //extra flow. its the secret!
            adjAdjIndices[index] = nextIndex + 1;
            adjAdjOwners[index] = owner;
        } else if (owner->right && owner->right->isAccessible){
            adjAdjIndices[index] = nextIndex + 1 - CHUNK_WIDTH;
            adjAdjOwners[index] = owner->right;
        }
        diffs[index] = (startBlockID - blockID);
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    }

    //Front Direction
    if (pos.z < CHUNK_WIDTH - 1) {
        nextIndex = startBlockIndex + CHUNK_WIDTH;
        owner = _chunk;
        nextCoord = pos.z + 1;
    } else if (_chunk->front && _chunk->front->isAccessible) {
        nextIndex = startBlockIndex + CHUNK_WIDTH - CHUNK_LAYER;
        owner = _chunk->front;
        nextCoord = 0;
    } else {
        return;
    }

    blockID = owner->getBlockID(nextIndex);

    if (blockID == NONE || Blocks[blockID].waterBreak){ //calculate diffs
        diffs[index] = (startBlockID - (_lowIndex - 1));
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    } else if (blockID >= _lowIndex && blockID < _highIndex){ //tmp CANT FLOW THOUGH FULL WATER

        if (nextCoord < CHUNK_WIDTH - 1){ //extra flow. its the secret!
            adjAdjIndices[index] = nextIndex + CHUNK_WIDTH;
            adjAdjOwners[index] = owner;
        } else if (owner->front && owner->front->isAccessible){
            adjAdjIndices[index] = nextIndex + CHUNK_WIDTH - CHUNK_LAYER;
            adjAdjOwners[index] = owner->front;
        }
        diffs[index] = (startBlockID - blockID);
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    }

  

    //Spread the liquid
    int numAdj = index + 1;
    for (int i = 0; i < index; i++){
        nextIndex = adjIndices[i];
        owner = adjOwners[i];
        diff = diffs[i] / numAdj;
        blockID = owner->getBlockID(nextIndex);

        if (diff > 0){
            //diff /= num;
            if (diff < (startBlockID - _lowIndex + 1)){
                if (blockID == NONE){
                    ChunkUpdater::placeBlockFromLiquidPhysics(owner, nextIndex, _lowIndex - 1 + diff);
                } else if (Blocks[blockID].waterBreak){
                    ChunkUpdater::removeBlock(owner, nextIndex, true);
                    ChunkUpdater::placeBlockFromLiquidPhysics(owner, nextIndex, _lowIndex - 1 + diff);              
                } else{
                    ChunkUpdater::placeBlockFromLiquidPhysics(owner, nextIndex, blockID + diff);
                }

               
                startBlockID -= diff;
                if (diff > 10 && inFrustum) particleEngine.addParticles(1, glm::dvec3(position.x + pos.x, position.y + pos.y, position.z + pos.z), 0, 0.1, 16665, 1111, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), Blocks[blockID].particleTex, 0.5f, 8);
               
                if (owner != _chunk) {
                    owner->changeState(ChunkStates::WATERMESH);
                    ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);
                }

                hasChanged = 1;
            }
        }
    }
    //Extra movement for more realistic flow
    for (int i = 0; i < index; i++) {
        if (adjAdjOwners[i]){
            owner = adjAdjOwners[i];
            nextIndex = adjAdjIndices[i];
            blockID = owner->getBlockID(nextIndex);
            if (blockID == NONE && startBlockID > _lowIndex){
                diff = (startBlockID - _lowIndex + 1) / 2;
                startBlockID -= diff;

                ChunkUpdater::placeBlockFromLiquidPhysics(owner, nextIndex, _lowIndex - 1 + diff);

                if (owner != _chunk) {
                    owner->changeState(ChunkStates::WATERMESH);
                    ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);
                }     
                hasChanged = 1;
            } else if (blockID >= _lowIndex && blockID < startBlockID - 1){
                diff = (startBlockID - blockID) / 2;

                ChunkUpdater::placeBlockFromLiquidPhysics(owner, nextIndex, blockID + diff);
                startBlockID -= diff;
         
                if (owner != _chunk) {
                    owner->changeState(ChunkStates::WATERMESH);
                    ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);
                }
                hasChanged = 1;
            }
        }
    }

    if (hasChanged) {
        ChunkUpdater::placeBlockFromLiquidPhysics(_chunk, startBlockIndex, startBlockID);

        _chunk->changeState(ChunkStates::WATERMESH);
        ChunkUpdater::updateNeighborStates(_chunk, pos, ChunkStates::WATERMESH);
    }
}

//every permutation of 0-3
const int dirs[96] = { 0, 1, 2, 3, 0, 1, 3, 2, 0, 2, 3, 1, 0, 2, 1, 3, 0, 3, 2, 1, 0, 3, 1, 2,
1, 0, 2, 3, 1, 0, 3, 2, 1, 2, 0, 3, 1, 2, 3, 0, 1, 3, 2, 0, 1, 3, 0, 2,
2, 0, 1, 3, 2, 0, 3, 1, 2, 1, 3, 0, 2, 1, 0, 3, 2, 3, 0, 1, 2, 3, 1, 0,
3, 0, 1, 2, 3, 0, 2, 1, 3, 1, 2, 0, 3, 1, 0, 2, 3, 2, 0, 1, 3, 2, 1, 0 };

//I will refactor this -Ben
void CAEngine::powderPhysics(int c)
{
    int blockType = GETBLOCKTYPE(_chunk->data[c]);
    if (Blocks[blockType].physicsProperty != P_POWDER) return;
    int x = c % CHUNK_WIDTH;
    int y = c / CHUNK_LAYER;
    int z = (c % CHUNK_LAYER) / CHUNK_WIDTH;
    int xz;

    const glm::ivec3 &position = _chunk->position;

    GLushort tmp1;
    int b, c2, c3;
    Chunk *owner, *owner2;
    bool hasChanged = 0;
    int tmp;
    int r;

    //bottom
    if (GETBLOCK(b = _chunk->getBottomBlockData(c, y, &c2, &owner)).isSupportive == 0){
        if (!owner || (owner->isAccessible == 0)) return;
        if (GETBLOCKTYPE(owner->getBottomBlockData(c2, c2 / CHUNK_LAYER, &c3, &owner2)) == NONE && GETBLOCKTYPE(owner2->getBottomBlockData(c3)) == NONE){ //if there is another empty space switch to a physics block

            GameManager::physicsEngine->addPhysicsBlock(glm::dvec3((double)position.x + c%CHUNK_WIDTH + 0.5, (double)position.y + c / CHUNK_LAYER, (double)position.z + (c%CHUNK_LAYER) / CHUNK_WIDTH + 0.5), _chunk->data[c]);

            ChunkUpdater::removeBlock(_chunk, c, false);
            hasChanged = 1;
        } else{ //otherwise do simple cellular automata
            b = GETBLOCKTYPE(b);
            //	if (b != NONE && b < LOWWATER) owner->BreakBlock(c2, owner->data[c2]); //to break blocks
            if (GETBLOCK(owner->data[c2]).powderMove){
                tmp = owner->data[c2];
                ChunkUpdater::placeBlock(owner, c2, _chunk->data[c]);
                ChunkUpdater::placeBlock(_chunk, c, tmp);
            } else{
                ChunkUpdater::placeBlock(owner, c2, _chunk->data[c]);
                ChunkUpdater::removeBlock(_chunk, c, false);
            }
       
            hasChanged = 1;
        }
    }

    //powder can only slide on powder
    if (hasChanged == 0 && GETBLOCK(b).physicsProperty == P_POWDER){
        r = (rand() % 24) * 4;

        for (int i = r; i < r + 4; i++){
            tmp = dirs[i];

            switch (tmp){
                //left
            case 0:
                b = _chunk->getLeftBlockData(c, x, &c2, &owner);
                if (GETBLOCK(b).powderMove){
                    if (GETBLOCK(owner->getBottomBlockData(c2)).powderMove){
                        tmp1 = _chunk->data[c];
                        ChunkUpdater::placeBlock(_chunk, c, owner->data[c2]);
                        ChunkUpdater::placeBlock(owner, c2, tmp1);
                        hasChanged = 1;
                    }
                }
                break;
                //right
            case 1:
                b = _chunk->getRightBlockData(c, x, &c2, &owner);
                if (GETBLOCK(b).powderMove){
                    if (GETBLOCK(owner->getBottomBlockData(c2)).powderMove){
                        tmp1 = _chunk->data[c];
                        ChunkUpdater::placeBlock(_chunk, c, owner->data[c2]);
                        ChunkUpdater::placeBlock(owner, c2, tmp1);
                        hasChanged = 1;
                    }
                }
                break;
                //front
            case 2:
                b = _chunk->getFrontBlockData(c, z, &c2, &owner);
                if (GETBLOCK(b).powderMove){
                    if (GETBLOCK(owner->getBottomBlockData(c2)).powderMove){
                        tmp1 = _chunk->data[c];
                        ChunkUpdater::placeBlock(_chunk, c, owner->data[c2]);
                        ChunkUpdater::placeBlock(owner, c2, tmp1);
                        hasChanged = 1;
                    }
                }
                break;
                //back
            case 3:
                b = _chunk->getBackBlockData(c, z, &c2, &owner);
                if (GETBLOCK(b).powderMove){
                    if (GETBLOCK(owner->getBottomBlockData(c2)).powderMove){
                        tmp1 = _chunk->data[c];
                        ChunkUpdater::placeBlock(_chunk, c, owner->data[c2]);
                        ChunkUpdater::placeBlock(owner, c2, tmp1);
                        hasChanged = 1;
                    }
                }
                break;
            }
            if (hasChanged) break;
        }

    }
}

//I will refactor this -Ben
void CAEngine::snowPhysics(int c)
{
    int tex = Blocks[SNOW].pxTex;
    int x = c % CHUNK_WIDTH;
    int y = c / CHUNK_LAYER;
    int z = (c % CHUNK_LAYER) / CHUNK_WIDTH;
    int xz;
    int blockType = GETBLOCKTYPE(_chunk->data[c]);
    GLushort tmp1;
    int b, c2, c3;
    Chunk *owner, *owner2;
    bool hasChanged = 0;
    int tmp;
    int r;
    bool isSnow = 0; // (blockType == SNOW);

    const glm::ivec3 &position = _chunk->position;

    //bottom
    if (GETBLOCK(b = _chunk->getBottomBlockData(c, y, &c2, &owner)).isSupportive == 0){
        if (!owner || (owner->isAccessible == 0)) return;
        if (GETBLOCKTYPE(owner->getBottomBlockData(c2, c2 / CHUNK_LAYER, &c3, &owner2)) == NONE && GETBLOCKTYPE(owner2->getBottomBlockData(c3)) == NONE){ //if there is another empty space switch to a physics block
            GameManager::physicsEngine->addPhysicsBlock(glm::dvec3((double)position.x + c%CHUNK_WIDTH + 0.5, (double)position.y + c / CHUNK_LAYER, (double)position.z + (c%CHUNK_LAYER) / CHUNK_WIDTH + 0.5), _chunk->data[c]);
            ChunkUpdater::removeBlock(_chunk, c, false);
            hasChanged = 1;
        } else{ //otherwise do simple cellular automata
            b = GETBLOCKTYPE(b);
            //	if (b != NONE && b < LOWWATER) owner->BreakBlock(c2, owner->data[c2]); //to break blocks
            if (GETBLOCK(owner->data[c2]).powderMove){
                tmp = owner->data[c2];
            } else{
                tmp = NONE;
            }

            owner->data[c2] = _chunk->data[c];
            ChunkUpdater::placeBlock(owner, c2, _chunk->data[c]);
            ChunkUpdater::placeBlock(_chunk, c, tmp);
            _chunk->data[c] = tmp;

            owner->changeState(ChunkStates::MESH);
            ChunkUpdater::snowAddBlockToUpdateList(owner, c2);
            if (isSnow) particleEngine.addParticles(1, glm::dvec3(position.x + x, position.y + y - 1.0, position.z + z), 0, 0.2, 10, 6, glm::vec4(255.0f), tex, 1.0f, 8);
            hasChanged = 1;
        }
    }

    //powder can only slide on powder
    if (GETBLOCK(b).physicsProperty == P_SNOW){
        if (!hasChanged){
            r = (rand() % 24) * 4;

            for (int i = r; i < r + 4; i++){
                tmp = dirs[i];

                switch (tmp){
                    //left
                case 0:
                    b = _chunk->getLeftBlockData(c, x, &c2, &owner);
                    if (GETBLOCK(b).powderMove){
                        if (GETBLOCK(owner->getBottomBlockData(c2)).powderMove){
                        
                            tmp1 = _chunk->data[c];
                            ChunkUpdater::placeBlock(_chunk, c, owner->data[c2]);
                            ChunkUpdater::placeBlock(owner, c2, tmp1);

                            ChunkUpdater::snowAddBlockToUpdateList(owner, c2);

                            hasChanged = 1;
                            if (isSnow) particleEngine.addParticles(1, glm::dvec3(position.x + x - 1.0, position.y + y, position.z + z), 0, 0.2, 10, 6, glm::vec4(255.0f), tex, 1.0f, 8);
                        }
                    }
                    break;
                    //right
                case 1:
                    b = _chunk->getRightBlockData(c, x, &c2, &owner);
                    if (GETBLOCK(b).powderMove){
                        if (GETBLOCK(owner->getBottomBlockData(c2)).powderMove){
     
                            tmp1 = _chunk->data[c];
                            ChunkUpdater::placeBlock(_chunk, c, owner->data[c2]);
                            ChunkUpdater::placeBlock(owner, c2, tmp1);

                            ChunkUpdater::snowAddBlockToUpdateList(owner, c2);
 
                            hasChanged = 1;
                            if (isSnow) particleEngine.addParticles(1, glm::dvec3(position.x + x + 1.0, position.y + y, position.z + z), 0, 0.2, 10, 6, glm::vec4(255.0f), tex, 1.0f, 8);
                        }
                    }
                    break;
                    //front
                case 2:
                    b = _chunk->getFrontBlockData(c, z, &c2, &owner);
                    if (GETBLOCK(b).powderMove){
                        if (GETBLOCK(owner->getBottomBlockData(c2)).powderMove){
   
                            tmp1 = _chunk->data[c];
                            ChunkUpdater::placeBlock(_chunk, c, owner->data[c2]);
                            ChunkUpdater::placeBlock(owner, c2, tmp1); 
                        
                            ChunkUpdater::snowAddBlockToUpdateList(owner, c2);

                            hasChanged = 1;
                            if (isSnow) particleEngine.addParticles(1, glm::dvec3(position.x + x, position.y + y, position.z + z + 1.0), 0, 0.2, 10, 6, glm::vec4(255.0f), tex, 1.0f, 8);
                        }
                    }
                    break;
                    //back
                case 3:
                    b = _chunk->getBackBlockData(c, z, &c2, &owner);
                    if (GETBLOCK(b).powderMove){
                        if (GETBLOCK(owner->getBottomBlockData(c2)).powderMove){

                            tmp1 = _chunk->data[c];
                            ChunkUpdater::placeBlock(_chunk, c, owner->data[c2]);
                            ChunkUpdater::placeBlock(owner, c2, tmp1);

                            ChunkUpdater::snowAddBlockToUpdateList(owner, c2);

                            hasChanged = 1;
                            if (isSnow) particleEngine.addParticles(1, glm::dvec3(position.x + x, position.y + y, position.z + z - 1.0), 0, 0.2, 10, 6, glm::vec4(255.0f), tex, 1.0f, 8);
                        }
                    }
                    break;
                }
                if (hasChanged) break;
            }
        }
    }
}
#include "stdafx.h"
#include "CAEngine.h"

#include <Vorb/IOManager.h>

#include "BlockPack.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "ChunkUpdater.h"
#include "GameManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "VoxelUtils.h"

CaPhysicsTypeDict CaPhysicsType::typesCache;
CaPhysicsTypeList CaPhysicsType::typesArray;

KEG_ENUM_INIT_BEGIN(CA_ALGORITHM, CA_ALGORITHM, e)
e->addValue("none", CA_ALGORITHM::NONE);
e->addValue("liquid", CA_ALGORITHM::LIQUID);
e->addValue("powder", CA_ALGORITHM::POWDER);
KEG_ENUM_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(CaPhysicsData)
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("updateRate", Keg::Value::basic(Keg::BasicType::UI32, offsetof(CaPhysicsData, updateRate)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("liquidLevels", Keg::Value::basic(Keg::BasicType::UI32, offsetof(CaPhysicsData, liquidLevels)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("algorithm", Keg::Value::custom("CA_ALGORITHM", offsetof(CaPhysicsData, alg), true));
KEG_TYPE_INIT_END

bool CaPhysicsType::update() {
    _ticks++;
    if (_ticks == _data.updateRate * HALF_CA_TICK_RES) {
        _ticks = 0;
        _isEven = !_isEven;
        return true;
    }
    return false;
}

bool CaPhysicsType::loadFromYml(const nString& filePath, const vio::IOManager* ioManager) {
    // Load the file
    nString fileData;
    ioManager->readFileToString(filePath.c_str(), fileData);
    if (fileData.length()) {
        if (Keg::parse(&_data, fileData.c_str(), "CaPhysicsData") == Keg::Error::NONE) {
            CaPhysicsType::typesCache[filePath] = this;
            _caIndex = typesArray.size();
            typesArray.push_back(this);
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

void CaPhysicsType::clearTypes() {
    // Clear CA physics cache
    for (auto& it : CaPhysicsType::typesCache) {
        delete it.second;
    }
    CaPhysicsType::typesCache.clear();
    typesArray.clear();
}

CAEngine::CAEngine(ChunkManager* chunkManager, PhysicsEngine* physicsEngine) :
    m_chunkManager(chunkManager),
    m_physicsEngine(physicsEngine){
    memset(_blockUpdateFlagList, 0, sizeof(_blockUpdateFlagList));
    //temporary
    _lowIndex = LOWWATER; 
    _range = 100;
    _highIndex = _lowIndex + _range - 1;
}

void CAEngine::updateSpawnerBlocks(bool powders)
{
    _lockedChunk = nullptr;
    int spawnerVal;
    int sinkVal;
    int c;
    glm::dvec3 physicsBlockPos;
    std::vector <GLushort> &activeBlocks = _chunk->spawnerBlocks;

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
            if (vvox::getBottomBlockData(_chunk, _lockedChunk, c) == 0){
                if (c >= CHUNK_LAYER){
                    c = c - CHUNK_LAYER;
                    if (spawnerVal >= LOWWATER) {
                        ChunkUpdater::placeBlock(_chunk, _lockedChunk, c, spawnerVal);
                    } else if (powders){
                        physicsBlockPos = glm::dvec3((double)_chunk->gridPosition.x + c%CHUNK_WIDTH + 0.5, (double)_chunk->gridPosition.y + c / CHUNK_LAYER, (double)_chunk->gridPosition.z + (c%CHUNK_LAYER) / CHUNK_WIDTH + 0.5);
                        m_physicsEngine->addPhysicsBlock(physicsBlockPos, spawnerVal);
                    }
                } else if (bottom && bottom->isAccessible){
                    c = c - CHUNK_LAYER + CHUNK_SIZE;
                    if (spawnerVal >= LOWWATER){
                        ChunkUpdater::placeBlockSafe(bottom, _lockedChunk, c, spawnerVal);
                    } else if (powders){
                        physicsBlockPos = glm::dvec3((double)bottom->gridPosition.x + c%CHUNK_WIDTH + 0.5, (double)bottom->gridPosition.y + c / CHUNK_LAYER, (double)bottom->gridPosition.z + (c%CHUNK_LAYER) / CHUNK_WIDTH + 0.5);
                        m_physicsEngine->addPhysicsBlock(physicsBlockPos, spawnerVal);
                    }
                }
            }
        }
        if (sinkVal){
            if (GETBLOCKID(vvox::getTopBlockData(_chunk, _lockedChunk, c)) == sinkVal){
                if (c + CHUNK_LAYER < CHUNK_SIZE){
                    c = c + CHUNK_LAYER;
                    _chunk->setBlockDataSafe(_lockedChunk, c, NONE); //TODO: This is incorrect, should call RemoveBlock or something similar
                    ChunkUpdater::addBlockToUpdateList(_chunk, _lockedChunk, c);
                    _chunk->changeState(ChunkStates::MESH);
                } else if (_chunk->top && _chunk->top->isAccessible){
                    c = c + CHUNK_LAYER - CHUNK_SIZE;
                    _chunk->top->setBlockDataSafe(_lockedChunk, c, NONE);
                    ChunkUpdater::addBlockToUpdateList(_chunk->top, _lockedChunk, c);
                    _chunk->top->changeState(ChunkStates::MESH);
                }
            }
        }
        i++;
    }
    if (_lockedChunk) {
        _lockedChunk->unlock();
        _lockedChunk = nullptr;
    }
}

void CAEngine::updateLiquidBlocks(int caIndex)
{
    _lockedChunk = nullptr;
    vvox::swapLockedChunk(_chunk, _lockedChunk);
    std::vector<bool>& activeUpdateList = _chunk->activeUpdateList;
    std::vector <ui16> *blockUpdateList = &_chunk->blockUpdateList[caIndex << 1];
    int actv = activeUpdateList[caIndex];
    int size = blockUpdateList[actv].size(); 
    if (size == 0) {
        _lockedChunk->unlock();
        _lockedChunk = nullptr;
        return;
    }
    activeUpdateList[caIndex] = !(activeUpdateList[caIndex]); //switch to other list
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
    if (_lockedChunk) {
        _lockedChunk->unlock();
        _lockedChunk = nullptr;
    }
}

void CAEngine::updatePowderBlocks(int caIndex)
{
    _lockedChunk = nullptr;
    std::vector<bool>& activeUpdateList = _chunk->activeUpdateList;
    std::vector <ui16> *blockUpdateList = &_chunk->blockUpdateList[caIndex << 1];
    int actv = activeUpdateList[caIndex];

    Uint32 size = blockUpdateList[actv].size();
    if (size != 0){
        activeUpdateList[caIndex] = !(activeUpdateList[caIndex]); //switch to other list
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

 //   blockUpdateList = _chunk->blockUpdateList[2];
 //   actv = activeUpdateList[2];
 //   size = blockUpdateList[actv].size();

    //if (size != 0){
    //    activeUpdateList[2] = !(activeUpdateList[2]);
    //    int b;
    //    Uint32 i;

    //    for (i = 0; i < size; i++){ //snow
    //        b = blockUpdateList[actv][i];
    //        if (_blockUpdateFlagList[b] == 0){
    //            _usedUpdateFlagList.push_back(b);
    //            _blockUpdateFlagList[b] = 1;
    //            if (_chunk->getBlock(b).physicsProperty == P_SNOW){ 
    //                powderPhysics(b);
    //            }
    //        }
    //    }
    //    blockUpdateList[actv].clear(); 

    //    for (i = 0; i < _usedUpdateFlagList.size(); i++){
    //        _blockUpdateFlagList[_usedUpdateFlagList[i]] = 0;
    //    }
    //    _usedUpdateFlagList.clear();
    //}
    if (_lockedChunk) {
        _lockedChunk->unlock();
        _lockedChunk = nullptr;
    }
}

void CAEngine::liquidPhysics(i32 startBlockIndex, i32 startBlockID) {
    // Helper function
    #define IS_LIQUID(b) ((b) >= _lowIndex && (b) < _highIndex)

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
    const i32v3 &position = _chunk->gridPosition;

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
    blockID = owner->getBlockIDSafe(_lockedChunk, nextIndex);

    //If we are falling on an air block
    if (blockID == NONE){ 
        ChunkUpdater::placeBlockFromLiquidPhysics(owner, _lockedChunk, nextIndex, startBlockID);
        ChunkUpdater::removeBlockFromLiquidPhysicsSafe(_chunk, _lockedChunk, startBlockIndex);

        ChunkUpdater::updateNeighborStates(_chunk, pos, ChunkStates::WATERMESH);
        if (owner != _chunk) ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);

        if (startBlockID > _lowIndex + 10 && inFrustum) particleEngine.addParticles(m_chunkManager, 1, glm::dvec3(position.x + pos.x, position.y + pos.y - 1.0, position.z + pos.z), 0, 0.1, 16665, 1111, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), Blocks[blockID].particleTex, 0.5f, 8);
        return;
    } else if (IS_LIQUID(blockID)) { //If we are falling on the same liquid
        //how much empty space there is
        diff = _highIndex - blockID;

        //if we cant completely fill in the empty space, fill it in as best we can
        if (startBlockID - _lowIndex + 1 > diff){
            startBlockID -= diff;
            ChunkUpdater::placeBlockFromLiquidPhysics(owner, _lockedChunk, nextIndex, blockID + diff);

            if (diff > 10 && inFrustum) particleEngine.addParticles(m_chunkManager, 1, glm::dvec3(position.x + pos.x, position.y + pos.y - 1.0, position.z + pos.z), 0, 0.1, 16665, 1111, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), Blocks[blockID].particleTex, 0.5f, 8);
            hasChanged = 1;
            
            if (owner != _chunk) {
                owner->changeState(ChunkStates::WATERMESH);
                ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);
            }

        } else { //otherwise ALL liquid falls and we are done
            ChunkUpdater::placeBlockFromLiquidPhysics(owner, _lockedChunk, nextIndex, blockID + (startBlockID - _lowIndex + 1));
            ChunkUpdater::removeBlockFromLiquidPhysicsSafe(_chunk, _lockedChunk, startBlockIndex);

            ChunkUpdater::updateNeighborStates(_chunk, pos, ChunkStates::WATERMESH);
            if (owner != _chunk) ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);

            if (startBlockID > _lowIndex + 10 && inFrustum) particleEngine.addParticles(m_chunkManager, 1, glm::dvec3(position.x + pos.x, position.y + pos.y - 1.0, position.z + pos.z), 0, 0.1, 16665, 1111, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), Blocks[blockID].particleTex, 0.5f, 8);
            return;
        }
    } else if (Blocks[blockID].waterBreak) { //destroy a waterBreak block, such as flora
        ChunkUpdater::removeBlock(m_chunkManager, m_physicsEngine, owner, _lockedChunk, nextIndex, true);
        ChunkUpdater::placeBlockFromLiquidPhysics(owner, _lockedChunk, nextIndex, startBlockID);
        ChunkUpdater::removeBlockFromLiquidPhysicsSafe(_chunk, _lockedChunk, startBlockIndex);

        ChunkUpdater::updateNeighborStates(_chunk, pos, ChunkStates::WATERMESH);
        if (owner != _chunk) ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);

        if (startBlockID > _lowIndex + 10 && inFrustum) particleEngine.addParticles(m_chunkManager, 1, glm::dvec3(position.x + pos.x, position.y + pos.y - 1.0, position.z + pos.z), 0, 0.1, 16665, 1111, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), Blocks[blockID].particleTex, 0.5f, 8);
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

    blockID = owner->getBlockIDSafe(_lockedChunk, nextIndex);

    if (blockID == NONE || Blocks[blockID].waterBreak){ //calculate diffs
        diffs[index] = (startBlockID - (_lowIndex - 1));
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    } else if (IS_LIQUID(blockID)){ //tmp CANT FLOW THOUGH FULL WATER
        
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

    blockID = owner->getBlockIDSafe(_lockedChunk, nextIndex);

    if (blockID == NONE || Blocks[blockID].waterBreak){ //calculate diffs
        diffs[index] = (startBlockID - (_lowIndex - 1));
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    } else if (IS_LIQUID(blockID)){ //tmp CANT FLOW THOUGH FULL WATER

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

    blockID = owner->getBlockIDSafe(_lockedChunk, nextIndex);

    if (blockID == NONE || Blocks[blockID].waterBreak){ //calculate diffs
        diffs[index] = (startBlockID - (_lowIndex - 1));
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    } else if (IS_LIQUID(blockID)){ //tmp CANT FLOW THOUGH FULL WATER

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

    blockID = owner->getBlockIDSafe(_lockedChunk, nextIndex);

    if (blockID == NONE || Blocks[blockID].waterBreak){ //calculate diffs
        diffs[index] = (startBlockID - (_lowIndex - 1));
        adjOwners[index] = owner;
        adjIndices[index++] = nextIndex;
    } else if (IS_LIQUID(blockID)){ //tmp CANT FLOW THOUGH FULL WATER

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
        //TODO(Ben): cache this instead
        blockID = owner->getBlockIDSafe(_lockedChunk, nextIndex);

        if (diff > 0){
            //diff /= num;
            if (diff < (startBlockID - _lowIndex + 1)){
                if (blockID == NONE){
                    ChunkUpdater::placeBlockFromLiquidPhysics(owner, _lockedChunk, nextIndex, _lowIndex - 1 + diff);
                } else if (Blocks[blockID].waterBreak){
                    ChunkUpdater::removeBlock(m_chunkManager, m_physicsEngine, owner, _lockedChunk, nextIndex, true);
                    ChunkUpdater::placeBlockFromLiquidPhysics(owner, _lockedChunk, nextIndex, _lowIndex - 1 + diff);
                } else{
                    ChunkUpdater::placeBlockFromLiquidPhysics(owner, _lockedChunk, nextIndex, blockID + diff);
                }

               
                startBlockID -= diff;
                if (diff > 10 && inFrustum) particleEngine.addParticles(m_chunkManager, 1, glm::dvec3(position.x + pos.x, position.y + pos.y, position.z + pos.z), 0, 0.1, 16665, 1111, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f), Blocks[blockID].particleTex, 0.5f, 8);
               
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
            blockID = owner->getBlockIDSafe(_lockedChunk, nextIndex);
            if (blockID == NONE && startBlockID > _lowIndex){
                diff = (startBlockID - _lowIndex + 1) / 2;
                startBlockID -= diff;

                ChunkUpdater::placeBlockFromLiquidPhysics(owner, _lockedChunk, nextIndex, _lowIndex - 1 + diff);

                if (owner != _chunk) {
                    owner->changeState(ChunkStates::WATERMESH);
                    ChunkUpdater::updateNeighborStates(owner, nextIndex, ChunkStates::WATERMESH);
                }     
                hasChanged = 1;
            } else if (blockID >= _lowIndex && blockID < startBlockID - 1){
                diff = (startBlockID - blockID) / 2;

                ChunkUpdater::placeBlockFromLiquidPhysics(owner, _lockedChunk, nextIndex, blockID + diff);
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
        ChunkUpdater::placeBlockFromLiquidPhysicsSafe(_chunk, _lockedChunk, startBlockIndex, startBlockID);

        _chunk->changeState(ChunkStates::WATERMESH);
        ChunkUpdater::updateNeighborStates(_chunk, pos, ChunkStates::WATERMESH);
    }
}

void CAEngine::powderPhysics(int blockIndex)
{
    // Directional constants
    #define LEFT 0
    #define RIGHT 1
    #define FRONT 2
    #define BACK 3
    // Every permutation of 0-3 for random axis directions
    #define DIRS_SIZE 96
    static const int DIRS[DIRS_SIZE] = { 0, 1, 2, 3, 0, 1, 3, 2, 0, 2, 3, 1, 0, 2, 1, 3, 0, 3, 2, 1, 0, 3, 1, 2,
        1, 0, 2, 3, 1, 0, 3, 2, 1, 2, 0, 3, 1, 2, 3, 0, 1, 3, 2, 0, 1, 3, 0, 2,
        2, 0, 1, 3, 2, 0, 3, 1, 2, 1, 3, 0, 2, 1, 0, 3, 2, 3, 0, 1, 2, 3, 1, 0,
        3, 0, 1, 2, 3, 0, 2, 1, 3, 1, 2, 0, 3, 1, 0, 2, 3, 2, 0, 1, 3, 2, 1, 0 };

    int blockData = _chunk->getBlockDataSafe(_lockedChunk, blockIndex);
    int nextBlockData, nextBlockIndex;
    // Make sure this is a powder block
    if (GETBLOCK(blockData).caAlg != CA_ALGORITHM::POWDER) return;

    Chunk* nextChunk;
    i32v3 pos = getPosFromBlockIndex(blockIndex);

    // *** Falling in Y direction ***
    // Get bottom block
    nextBlockData = vvox::getBottomBlockData(_chunk, _lockedChunk, blockIndex, pos.y, nextBlockIndex, nextChunk);
    // Check for edge
    if (nextBlockData == -1) return;
    // Since getXXXBlockData may lock a chunk, we need this
    _lockedChunk = nextChunk;
    // Get block info
    const Block& bottomBlock = GETBLOCK(nextBlockData);
    // Check if we can move down
    if (bottomBlock.isCrushable) {
        // Move down and crush block
        ChunkUpdater::placeBlock(nextChunk, _lockedChunk, nextBlockIndex, blockData);
        ChunkUpdater::removeBlockSafe(m_chunkManager, m_physicsEngine, _chunk, _lockedChunk, blockIndex, false);
        return;
    } else if (bottomBlock.powderMove) {
        // Move down and swap places
        ChunkUpdater::placeBlock(nextChunk, _lockedChunk, nextBlockIndex, blockData);
        ChunkUpdater::placeBlockSafe(_chunk, _lockedChunk, blockIndex, nextBlockData);
        return;
    } else if (bottomBlock.caAlg != CA_ALGORITHM::POWDER) {
        // We can only slide on powder, so if its not powder, we return.
        return;
    }

    // We use _dirIndex to avoid any calls to rand(). We dont get truly random direction but
    // it appears random.
    #define NUM_AXIS 4
    _dirIndex += NUM_AXIS;
    // Wrap back to zero
    if (_dirIndex == DIRS_SIZE) _dirIndex = 0;
    // Loop through our 4 "random" directions
    for (int i = _dirIndex; i < _dirIndex + NUM_AXIS; i++) {
        // Get the neighbor block in the direction
        switch (DIRS[i]) {
            case LEFT:
                // Only lock the chunk if we know we aren't about to go to a neighbor
                if (pos.x != 0) vvox::swapLockedChunk(_chunk, _lockedChunk);
                nextBlockData = vvox::getLeftBlockData(_chunk, _lockedChunk, blockIndex, pos.x,
                                                         nextBlockIndex, nextChunk);
                break;
            case RIGHT:
                if (pos.x != CHUNK_WIDTH - 1) vvox::swapLockedChunk(_chunk, _lockedChunk);
                nextBlockData = vvox::getRightBlockData(_chunk, _lockedChunk, blockIndex, pos.x,
                                                          nextBlockIndex, nextChunk);
                break;
            case BACK:
                if (pos.z != 0) vvox::swapLockedChunk(_chunk, _lockedChunk);
                nextBlockData = vvox::getBackBlockData(_chunk, _lockedChunk, blockIndex, pos.z,
                                                         nextBlockIndex, nextChunk);
                break;
            case FRONT:
                if (pos.z != CHUNK_WIDTH - 1) vvox::swapLockedChunk(_chunk, _lockedChunk);
                nextBlockData = vvox::getFrontBlockData(_chunk, _lockedChunk, blockIndex, pos.z,
                                                          nextBlockIndex, nextChunk);
                break;
        }
        // Check for edge
        if (nextBlockData == -1) return;
        // Since getXXXBlockData may lock a chunk, we need this
        _lockedChunk = nextChunk;
        // Check bottom
        const Block& diagonalBlock = GETBLOCK(vvox::getBottomBlockData(nextChunk, _lockedChunk, 
                                              nextBlockIndex));
        // We only move to the side if we can fall down the next update
        if (diagonalBlock.powderMove || diagonalBlock.isCrushable) {
            // Get block info
            const Block& nextBlock = GETBLOCK(nextBlockData);
            // Check if we can move
            if (nextBlock.isCrushable) {
                // Move and crush block
                ChunkUpdater::placeBlockSafe(nextChunk, _lockedChunk, nextBlockIndex, blockData);
                ChunkUpdater::removeBlockSafe(m_chunkManager, m_physicsEngine, _chunk, _lockedChunk, blockIndex, false);
                return;
            } else if (nextBlock.powderMove) {
                // Move and swap places
                ChunkUpdater::placeBlockSafe(nextChunk, _lockedChunk, nextBlockIndex, blockData);
                ChunkUpdater::placeBlockSafe(_chunk, _lockedChunk, blockIndex, nextBlockData);
                return;
            }
        }
    }
}

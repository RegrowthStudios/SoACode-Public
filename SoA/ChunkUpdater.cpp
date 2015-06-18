#include "stdafx.h"
#include "ChunkUpdater.h"

#include "BlockPack.h"
#include "CAEngine.h"
#include "Errors.h"
#include "GameManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "VoxelLightEngine.h"
#include "VoxelNavigation.inl"
#include "VoxelUtils.h"

#include "VoxelUpdateOrder.inl"

void ChunkUpdater::randomBlockUpdates(PhysicsEngine* physicsEngine, Chunk* chunk)
{
    //if (!chunk->isAccessible) return;
    //int blockIndex, blockIndex2, blockID;
    //ChunkStates newState;
    //bool needsSetup;
    //Chunk *owner;

    //Chunk* left = chunk->left;
    //Chunk* right = chunk->right;
    //Chunk* front = chunk->front;
    //Chunk* back = chunk->back;
    //Chunk* bottom = chunk->bottom;
    //Chunk* top = chunk->top;

    //i32v3 pos;

    //Chunk* lockedChunk = nullptr;

    //for (int i = 0; i < 15; i++) {
    //    needsSetup = false;

    //    blockIndex = RandomUpdateOrder[chunk->blockUpdateIndex++];
    //    if (chunk->blockUpdateIndex == CHUNK_SIZE) chunk->blockUpdateIndex = 0;

    //    pos = getPosFromBlockIndex(blockIndex);
    //    
    //    blockID = chunk->getBlockIDSafe(lockedChunk, blockIndex);

    //    if (Blocks[blockID].emitterRandom){
    //        // uncomment for falling leaves
    //        //    particleEngine.AddAnimatedParticles(1, glm::dvec3(position.x + x, Y + y, position.z + z), Blocks[b].emitterRandom);
    //    }

    //    //TODO: Replace most of this with block update scripts
    //    //TODO(Ben): There are race conditions here!
    //    if (blockID >= LOWWATER && blockID < LOWWATER + 5 && (GETBLOCKID(vvox::getBottomBlockData(chunk, lockedChunk, blockIndex, pos.y, blockIndex2, owner)) < LOWWATER)) {
    //        chunk->setBlockDataSafe(lockedChunk, blockIndex, NONE);
    //        owner->numBlocks--;
    //        needsSetup = true;
    //        newState = ChunkStates::WATERMESH;
    //        ChunkUpdater::addBlockToUpdateList(chunk, lockedChunk, blockIndex);
    //    } else if (blockID == FIRE) {
    //        // TODO(Ben): Update
    //        //updateFireBlock(chunkManager, physicsEngine, chunk, blockIndex);
    //        needsSetup = true;
    //        newState = ChunkStates::MESH;
    //    } else if (blockID == DIRTGRASS){
    //        ui32 bt = GETBLOCKID(vvox::getTopBlockData(chunk, lockedChunk, blockIndex, pos.y, blockIndex2, owner));
    //        // Tmp debugging           
    //        if (bt > Blocks.size()) {
    //            pError("Invalid block in update!: " + std::to_string(bt));
    //        }
    //        if ((Blocks[bt].collide && bt != LEAVES1) || bt >= LOWWATER){
    //            chunk->setBlockDataSafe(lockedChunk, blockIndex, DIRT);
    //            needsSetup = true;
    //            newState = ChunkStates::MESH;
    //        }
    //    } else if (blockID == DIRT){
    //        if ((rand() % 10 == 0) && (GETBLOCKID(vvox::getTopBlockData(chunk, lockedChunk, blockIndex, pos.y, blockIndex2, owner)) == NONE) &&
    //            (GETBLOCKID(vvox::getLeftBlockData(chunk, lockedChunk, blockIndex, pos.x, blockIndex2, owner)) == DIRTGRASS ||
    //            GETBLOCKID(vvox::getRightBlockData(chunk, lockedChunk, blockIndex, pos.x, blockIndex2, owner)) == DIRTGRASS ||
    //            GETBLOCKID(vvox::getFrontBlockData(chunk, lockedChunk, blockIndex, pos.z, blockIndex2, owner)) == DIRTGRASS ||
    //            GETBLOCKID(vvox::getBackBlockData(chunk, lockedChunk, blockIndex, pos.z, blockIndex2, owner)) == DIRTGRASS)) {
    //            chunk->setBlockDataSafe(lockedChunk, blockIndex, DIRTGRASS);
    //            needsSetup = true;
    //            newState = ChunkStates::MESH;
    //        }
    //    }

    //    if (needsSetup){
    //        
    //        chunk->changeState(newState);
    //        if (pos.x == 0){
    //            if (left && left->isAccessible){
    //                left->changeState(newState);
    //            }
    //        } else if (pos.x == CHUNK_WIDTH - 1){
    //            if (right && right->isAccessible){
    //                right->changeState(newState);
    //            }
    //        }
    //        if (pos.y == 0){
    //            if (bottom && bottom->isAccessible){
    //                bottom->changeState(newState);
    //            }
    //        } else if (pos.y == CHUNK_WIDTH - 1){
    //            if (top && top->isAccessible){
    //                top->changeState(newState);
    //            }
    //        }
    //        if (pos.z == 0){
    //            if (back && back->isAccessible){
    //                back->changeState(newState);
    //            }
    //        } else if (pos.z == CHUNK_WIDTH - 1){
    //            if (front && front->isAccessible){
    //                front->changeState(newState);
    //            }
    //        }
    //    }
    //}
    //if (lockedChunk) lockedChunk->unlock();
}

void ChunkUpdater::placeBlockSafe(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int blockData) {
   /* vvox::swapLockedChunk(chunk, lockedChunk);
    placeBlock(chunk, lockedChunk, blockIndex, blockData);*/
}

void ChunkUpdater::placeBlockNoUpdate(Chunk* chunk, int blockIndex, int blockType) {
 
    //Block &block = GETBLOCK(blockType);

    //if (chunk->getBlockData(blockIndex) == NONE) {
    //    chunk->numBlocks++;
    //}
    //chunk->setBlockData(blockIndex, blockType);

    //if (block.spawnerVal || block.sinkVal) {
    //    chunk->spawnerBlocks.push_back(blockIndex);
    //}

    //const i32v3 pos = getPosFromBlockIndex(blockIndex);

    //if (block.emitter) {
    //    particleEngine.addEmitter(block.emitter, glm::dvec3(chunk->voxelPosition.x + pos.x, chunk->voxelPosition.y + pos.y, chunk->voxelPosition.z + pos.z), blockType);
    //}

    //// If its a plant, we need to do some extra iteration
    //if (block.floraHeight) {
    //    placeFlora(chunk, blockIndex, blockType);
    //}

    ////Check for light removal due to block occlusion
    //if (block.blockLight) {

    //    if (chunk->getSunlight(blockIndex)) {
    //        if (chunk->getSunlight(blockIndex) == MAXLIGHT) {
    //            chunk->setSunlight(blockIndex, 0);
    //            chunk->sunRemovalList.push_back(blockIndex);
    //        } else {
    //            chunk->sunlightRemovalQueue.emplace(blockIndex, chunk->getSunlight(blockIndex));
    //            chunk->setSunlight(blockIndex, 0);
    //        }
    //    }

    //    if (chunk->getLampLight(blockIndex)) {
    //        chunk->lampLightRemovalQueue.emplace(blockIndex, chunk->getLampLight(blockIndex));
    //        chunk->setLampLight(blockIndex, 0);
    //    }
    //} else if (block.colorFilter != f32v3(1.0f)) {
    //    //This will pull light from neighbors
    //    chunk->lampLightRemovalQueue.emplace(blockIndex, chunk->getLampLight(blockIndex));
    //    chunk->setLampLight(blockIndex, 0);
    //}
    ////Light placement
    //if (block.lightColorPacked) {
    //    chunk->setLampLight(blockIndex, block.lightColorPacked);
    //    chunk->lampLightUpdateQueue.emplace(blockIndex, block.lightColorPacked);
    //}

    //if (GETBLOCKID(blockType) >= LOWWATER) {
    //    chunk->changeState(ChunkStates::WATERMESH);
    //    updateNeighborStates(chunk, pos, ChunkStates::WATERMESH);
    //} else {
    //    chunk->changeState(ChunkStates::MESH);
    //    updateNeighborStates(chunk, pos, ChunkStates::MESH);

    //}
    //chunk->dirty = true;
}

//Simplified placeBlock for liquid physics
void ChunkUpdater::placeBlockFromLiquidPhysics(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int blockType)
{
    //Block &block = GETBLOCK(blockType);

    //if (chunk->getBlockData(blockIndex) == NONE) {
    //    chunk->numBlocks++;
    //}
    //chunk->setBlockData(blockIndex, blockType);

    ////Check for light removal due to block occlusion
    //if (block.blockLight) {

    //    if (chunk->getSunlight(blockIndex)){
    //        if (chunk->getSunlight(blockIndex) == MAXLIGHT){
    //            chunk->setSunlight(blockIndex, 0);
    //            chunk->sunRemovalList.push_back(blockIndex);
    //        } else {
    //            chunk->sunlightRemovalQueue.emplace(blockIndex, chunk->getSunlight(blockIndex));
    //            chunk->setSunlight(blockIndex, 0);
    //        }
    //    }

    //    if (chunk->getLampLight(blockIndex)){
    //        chunk->lampLightRemovalQueue.emplace(blockIndex, chunk->getLampLight(blockIndex));
    //        chunk->setLampLight(blockIndex, 0);
    //    }
    //}

    ////Light placement
    //if (block.lightColorPacked) {
    //    chunk->setLampLight(blockIndex, block.lightColorPacked);
    //    chunk->lampLightUpdateQueue.emplace(blockIndex, block.lightColorPacked);
    //}

    //ChunkUpdater::addBlockToUpdateList(chunk, lockedChunk, blockIndex);
  
    //chunk->dirty = true;
}

void ChunkUpdater::placeBlockFromLiquidPhysicsSafe(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int blockType) {
    /*  vvox::swapLockedChunk(chunk, lockedChunk);
      placeBlockFromLiquidPhysics(chunk, lockedChunk, blockIndex, blockType);*/
}

void ChunkUpdater::removeBlock(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk* chunk, Chunk*& lockedChunk, int blockIndex, bool isBreak, double force, glm::vec3 explodeDir)
{
    //int blockID = chunk->getBlockID(blockIndex);
    //const Block &block = Blocks[blockID];

    //float explodeDist = glm::length(explodeDir);

    //const i32v3 pos = getPosFromBlockIndex(blockIndex);

    //if (chunk->getBlockID(blockIndex) == 0) return;

    //GLbyte da, db, dc;

    ////Falling check
    //if (explodeDist){
    //    GLubyte d;
    //    da = (GLbyte)(explodeDir.x);
    //    db = (GLbyte)(explodeDir.y);
    //    dc = (GLbyte)(explodeDir.z);
    //    if (explodeDist > 255){
    //        d = 255;
    //    } else{
    //        d = (GLubyte)(explodeDist);
    //    }
    //    physicsEngine->addFallingCheckNode(FallingCheckNode(chunk, blockIndex, da, db, dc, d));
    //} else{
    //    physicsEngine->addFallingCheckNode(FallingCheckNode(chunk, blockIndex));
    //}

    ////If we are braking the block rather than removing it, it should explode or emit particles
    //if (isBreak) {
    //    if (block.explosivePower){
    //        glm::dvec3 dtmp(chunk->voxelPosition.x + pos.x, chunk->voxelPosition.y + pos.y, chunk->voxelPosition.z + pos.z);
    //        physicsEngine->addExplosion(ExplosionNode(dtmp, blockID));
    //    }

    //    if (block.emitterOnBreak && block.explosivePower == 0){ // 
    //        particleEngine.addEmitter(block.emitterOnBreak, glm::dvec3(chunk->voxelPosition.x + blockIndex%CHUNK_WIDTH, chunk->voxelPosition.y + blockIndex / CHUNK_LAYER, chunk->voxelPosition.z + (blockIndex%CHUNK_LAYER) / CHUNK_WIDTH), blockID);
    //    }
    //    if (explodeDist){
    //        f32 expForce = glm::length(explodeDir);
    //        expForce = pow(0.89f, expForce)*0.6f;
    //        if (expForce < 0.0f) expForce = 0.0f;
    //        breakBlock(chunk, pos.x, pos.y, pos.z, blockID, force, -glm::normalize(explodeDir)*expForce);
    //    } else{
    //        breakBlock(chunk, pos.x, pos.y, pos.z, blockID, force);
    //    }
    //}
    //chunk->setBlockData(blockIndex, NONE);

    ////Update lighting
    //if (block.blockLight || block.lightColorPacked) {
    //    //This will pull light from neighbors
    //    chunk->lampLightRemovalQueue.emplace(blockIndex, chunk->getLampLight(blockIndex));
    //    chunk->setLampLight(blockIndex, 0);

    //    //sunlight update
    //    if (pos.y < CHUNK_WIDTH - 1) {
    //        if (chunk->getSunlight(blockIndex + CHUNK_LAYER) == MAXLIGHT) {
    //            chunk->setSunlight(blockIndex, MAXLIGHT);
    //            chunk->sunExtendList.push_back(blockIndex);
    //        } else {
    //            //This will pull light from neighbors
    //            chunk->sunlightRemovalQueue.emplace(blockIndex, chunk->getSunlight(blockIndex));
    //            chunk->setSunlight(blockIndex, 0);
    //        }
    //    } else if (chunk->top && chunk->top->isAccessible) {
    //        if (chunk->top->getSunlight(blockIndex + CHUNK_LAYER - CHUNK_SIZE) == MAXLIGHT) {
    //            chunk->setSunlight(blockIndex, MAXLIGHT);
    //            chunk->sunExtendList.push_back(blockIndex);
    //        } else {
    //            //This will pull light from neighbors
    //            chunk->sunlightRemovalQueue.emplace(blockIndex, chunk->getSunlight(blockIndex));
    //            chunk->setSunlight(blockIndex, 0);
    //        }
    //    } else {
    //        //This will pull light from neighbors
    //        chunk->sunlightRemovalQueue.emplace(blockIndex, chunk->getSunlight(blockIndex));
    //        chunk->setSunlight(blockIndex, 0);
    //    }
    //}

    //// If its a plant, we need to do some extra iteration
    //if (block.floraHeight) {
    //    removeFlora(chunkManager, physicsEngine, chunk, lockedChunk, blockIndex, blockID);
    //}

    //ChunkUpdater::addBlockToUpdateList(chunk, lockedChunk, blockIndex);
    //chunk->numBlocks--;
    //if (chunk->numBlocks < 0) chunk->numBlocks = 0;


    //chunk->changeState(ChunkStates::MESH);
    //chunk->dirty = 1;

    //updateNeighborStates(chunk, pos, ChunkStates::MESH);
}

void ChunkUpdater::removeBlockSafe(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk* chunk, Chunk*& lockedChunk, int blockIndex, bool isBreak, double force, glm::vec3 explodeDir) {
    /* vvox::swapLockedChunk(chunk, lockedChunk);
     removeBlock(chunkManager, physicsEngine, chunk, lockedChunk, blockIndex, isBreak, force, explodeDir);*/
}

void ChunkUpdater::removeBlockFromLiquidPhysics(Chunk* chunk, Chunk*& lockedChunk, int blockIndex)
{
    //const Block &block = chunk->getBlock(blockIndex);

    //const i32v3 pos = getPosFromBlockIndex(blockIndex);
  
    //chunk->setBlockData(blockIndex, NONE);

    ////Update lighting
    //if (block.blockLight || block.lightColorPacked) {
    //    //This will pull light from neighbors
    //    chunk->lampLightRemovalQueue.emplace(blockIndex, chunk->getLampLight(blockIndex));
    //    chunk->setLampLight(blockIndex, 0);

    //    //sunlight update
    //    if (pos.y < CHUNK_WIDTH - 1) {
    //        if (chunk->getSunlight(blockIndex + CHUNK_LAYER) == MAXLIGHT) {
    //            chunk->setSunlight(blockIndex, MAXLIGHT);
    //            chunk->sunExtendList.push_back(blockIndex);
    //        } else {
    //            //This will pull light from neighbors
    //            chunk->sunlightRemovalQueue.emplace(blockIndex, chunk->getSunlight(blockIndex));
    //            chunk->setSunlight(blockIndex, 0);
    //        }
    //    } else if (chunk->top && chunk->top->isAccessible) {
    //        if (chunk->top->getSunlight(blockIndex + CHUNK_LAYER - CHUNK_SIZE) == MAXLIGHT) {
    //            chunk->setSunlight(blockIndex, MAXLIGHT);
    //            chunk->sunExtendList.push_back(blockIndex);
    //        } else {
    //            //This will pull light from neighbors
    //            chunk->sunlightRemovalQueue.emplace(blockIndex, chunk->getSunlight(blockIndex));
    //            chunk->setSunlight(blockIndex, 0);
    //        }
    //    } else {
    //        //This will pull light from neighbors
    //        chunk->sunlightRemovalQueue.emplace(blockIndex, chunk->getSunlight(blockIndex));
    //        chunk->setSunlight(blockIndex, 0);
    //    }
    //}

    //ChunkUpdater::addBlockToUpdateList(chunk, lockedChunk, blockIndex);
    //chunk->numBlocks--;
    //if (chunk->numBlocks < 0) chunk->numBlocks = 0;


    //chunk->changeState(ChunkStates::WATERMESH);
    //chunk->dirty = 1;
}

void ChunkUpdater::removeBlockFromLiquidPhysicsSafe(Chunk* chunk, Chunk*& lockedChunk, int blockIndex) {
   /* vvox::swapLockedChunk(chunk, lockedChunk);
    removeBlockFromLiquidPhysics(chunk, lockedChunk, blockIndex);*/
}

void ChunkUpdater::updateNeighborStates(Chunk* chunk, const i32v3& pos, ChunkStates state) {
  /*  if (pos.x == 0){
        if (chunk->left){
            chunk->left->changeState(state);
        }
    } else if (pos.x == CHUNK_WIDTH - 1){
        if (chunk->right){
            chunk->right->changeState(state);
        }
    }
    if (pos.y == 0){
        if (chunk->bottom){
            chunk->bottom->changeState(state);
        }
    } else if (pos.y == CHUNK_WIDTH - 1){
        if (chunk->top){
            chunk->top->changeState(state);
        }
    }
    if (pos.z == 0){
        if (chunk->back){
            chunk->back->changeState(state);
        }
    } else if (pos.z == CHUNK_WIDTH - 1){
        if (chunk->front){
            chunk->front->changeState(state);
        }
    }*/
}

void ChunkUpdater::updateNeighborStates(Chunk* chunk, int blockIndex, ChunkStates state) {
   /* const i32v3 pos = getPosFromBlockIndex(blockIndex);
    if (pos.x == 0){
        if (chunk->left){
            chunk->left->changeState(state);
        }
    } else if (pos.x == CHUNK_WIDTH - 1){
        if (chunk->right){
            chunk->right->changeState(state);
        }
    }
    if (pos.y == 0){
        if (chunk->bottom){
            chunk->bottom->changeState(state);
        }
    } else if (pos.y == CHUNK_WIDTH - 1){
        if (chunk->top){
            chunk->top->changeState(state);
        }
    }
    if (pos.z == 0){
        if (chunk->back){
            chunk->back->changeState(state);
        }
    } else if (pos.z == CHUNK_WIDTH - 1){
        if (chunk->front){
            chunk->front->changeState(state);
        }
    }*/
}

void ChunkUpdater::addBlockToUpdateList(Chunk* chunk, Chunk*& lockedChunk, int c)
{
    //int phys;
    //const i32v3 pos = getPosFromBlockIndex(c);

    //Chunk*& left = chunk->left;
    //Chunk*& right = chunk->right;
    //Chunk*& front = chunk->front;
    //Chunk*& back = chunk->back;
    //Chunk*& top = chunk->top;
    //Chunk*& bottom = chunk->bottom;

    //if ((phys = chunk->getBlockSafe(lockedChunk, c).caIndex) > -1) {
    //    chunk->addPhysicsUpdate(phys, c);
    //}

    //if (pos.x > 0){ //left
    //    if ((phys = chunk->getBlockSafe(lockedChunk, c - 1).caIndex) > -1) {
    //        chunk->addPhysicsUpdate(phys, c - 1);
    //    }
    //} else if (left && left->isAccessible){
    //    if ((phys = left->getBlockSafe(lockedChunk, c + CHUNK_WIDTH - 1).caIndex) > -1) {
    //        left->addPhysicsUpdate(phys, c + CHUNK_WIDTH - 1);
    //    }
    //} else{
    //    return;
    //}

    //if (pos.x < CHUNK_WIDTH - 1){ //right
    //    if ((phys = chunk->getBlockSafe(lockedChunk, c + 1).caIndex) > -1) {
    //        chunk->addPhysicsUpdate(phys, c + 1);
    //    }
    //} else if (right && right->isAccessible){
    //    if ((phys = right->getBlockSafe(lockedChunk, c - CHUNK_WIDTH + 1).caIndex) > -1) {
    //        right->addPhysicsUpdate(phys, c - CHUNK_WIDTH + 1);
    //    }
    //} else{
    //    return;
    //}

    //if (pos.z > 0){ //back
    //    if ((phys = chunk->getBlockSafe(lockedChunk, c - CHUNK_WIDTH).caIndex) > -1) {
    //        chunk->addPhysicsUpdate(phys, c - CHUNK_WIDTH);
    //    }
    //} else if (back && back->isAccessible){
    //    if ((phys = back->getBlockSafe(lockedChunk, c + CHUNK_LAYER - CHUNK_WIDTH).caIndex) > -1) {
    //        back->addPhysicsUpdate(phys, c + CHUNK_LAYER - CHUNK_WIDTH);
    //    }
    //} else{
    //    return;
    //}

    //if (pos.z < CHUNK_WIDTH - 1){ //front
    //    if ((phys = chunk->getBlockSafe(lockedChunk, c + CHUNK_WIDTH).caIndex) > -1) {
    //        chunk->addPhysicsUpdate(phys, c + CHUNK_WIDTH);
    //    }
    //} else if (front && front->isAccessible){
    //    if ((phys = front->getBlockSafe(lockedChunk, c - CHUNK_LAYER + CHUNK_WIDTH).caIndex) > -1) {
    //        front->addPhysicsUpdate(phys, c - CHUNK_LAYER + CHUNK_WIDTH);
    //    }
    //} else{
    //    return;
    //}

    //if (pos.y > 0){ //bottom
    //    if ((phys = chunk->getBlockSafe(lockedChunk, c - CHUNK_LAYER).caIndex) > -1) {
    //        chunk->addPhysicsUpdate(phys, c - CHUNK_LAYER);
    //    }
    //} else if (bottom && bottom->isAccessible){
    //    if ((phys = bottom->getBlockSafe(lockedChunk, CHUNK_SIZE - CHUNK_LAYER + c).caIndex) > -1) {
    //        bottom->addPhysicsUpdate(phys, CHUNK_SIZE - CHUNK_LAYER + c);
    //    }
    //} else{
    //    return;
    //}

    //if (pos.y < CHUNK_WIDTH - 1){ //top
    //    if ((phys = chunk->getBlockSafe(lockedChunk, c + CHUNK_LAYER).caIndex) > -1) {
    //        chunk->addPhysicsUpdate(phys, c + CHUNK_LAYER);
    //    }
    //} else if (top && top->isAccessible){
    //    if ((phys = top->getBlockSafe(lockedChunk, c - CHUNK_SIZE + CHUNK_LAYER).caIndex) > -1) {
    //        top->addPhysicsUpdate(phys, c - CHUNK_SIZE + CHUNK_LAYER);
    //    }
    //}
}

void ChunkUpdater::snowAddBlockToUpdateList(Chunk* chunk, int c)
{
    //int phys;
    //const i32v3 pos = getPosFromBlockIndex(c);

    //if ((phys = chunk->getBlock(c).caIndex) > -1) {
    //    chunk->addPhysicsUpdate(phys, c);
    //}

    //if (pos.y > 0){ //bottom
    //    if ((phys = chunk->getBlock(c - CHUNK_LAYER).caIndex) > -1) {
    //        chunk->addPhysicsUpdate(phys, c - CHUNK_LAYER);
    //    }
    //} else if (chunk->bottom && chunk->bottom->isAccessible){
    //    if ((phys = chunk->bottom->getBlock(CHUNK_SIZE - CHUNK_LAYER + c).caIndex) > -1) {
    //        chunk->addPhysicsUpdate(phys, CHUNK_SIZE - CHUNK_LAYER + c);
    //    }
    //} else{
    //    return;
    //}

    //if (pos.y < CHUNK_WIDTH - 1){ //top
    //    if ((phys = chunk->getBlock(c + CHUNK_LAYER).caIndex) > -1) {
    //        chunk->addPhysicsUpdate(phys, c + CHUNK_LAYER);
    //    }
    //} else if (chunk->top && chunk->top->isAccessible){
    //    if ((phys = chunk->top->getBlock(c - CHUNK_SIZE + CHUNK_LAYER).caIndex) > -1) {
    //        chunk->top->addPhysicsUpdate(phys, c - CHUNK_SIZE + CHUNK_LAYER);
    //    }
    //}
}

//TODO: Replace this with simple emitterOnBreak
//This function name is misleading, ignore for now
void ChunkUpdater::breakBlock(Chunk* chunk, int x, int y, int z, int blockType, double force, glm::vec3 extraForce)
{
//    glm::vec4 color;
//    int btype = GETBLOCKID(blockType);
//    GLuint flags = GETFLAGS(blockType);
//
//    color.a = 255;
//
//    if (Blocks[btype].altColors.size() >= flags && flags){
//        color.r = Blocks[btype].altColors[flags - 1].r;
//        color.g = Blocks[btype].altColors[flags - 1].g;
//        color.b = Blocks[btype].altColors[flags - 1].b;
//        //    cout << btype << " " << flags-1 << " ";
//    } else{
//        color.r = Blocks[btype].color.r;
//        color.g = Blocks[btype].color.g;
//        color.b = Blocks[btype].color.b;
//    }
//
//    if (Blocks[btype].meshType != MeshType::NONE && Blocks[btype].explosivePower == 0){
//        if (!chunk->mesh || chunk->mesh->inFrustum){
////            particleEngine.addParticles(BPARTICLES, glm::dvec3(chunk->gridPosition.x + x, chunk->gridPosition.y + y, chunk->gridPosition.z + z), 0, 0.1, 0, 1, color, Blocks[btype].base.px, 2.0f, 4, extraForce);
//        }
//    }
}

// TODO(Ben): Make this cleaner
void ChunkUpdater::placeFlora(Chunk* chunk, int blockIndex, int blockID) {
    //
    //// Start it out at -1 so when we add 1 we get 0.
    //ui16 floraHeight = -1;
    //ui16 floraYpos = -1;
    //ui16 tertiaryData; 
    //// Get tertiary data
    //if (blockIndex > CHUNK_LAYER) {
    //    // Only need the data if its the same plant as we are
    //    if (chunk->getBlockID(blockIndex - CHUNK_LAYER) == blockID) {
    //        tertiaryData = chunk->getTertiaryData(blockIndex - CHUNK_LAYER);
    //        // Grab height and position
    //        floraHeight = VoxelBits::getFloraHeight(tertiaryData);
    //        floraYpos = VoxelBits::getFloraPosition(tertiaryData);
    //    }
    //} else {

    //    if (chunk->bottom && chunk->bottom->isAccessible) {
    //        // Only need the data if its the same plant as we are
    //        if (chunk->bottom->getBlockID(blockIndex - CHUNK_LAYER + CHUNK_SIZE) == blockIndex) {
    //            tertiaryData = chunk->bottom->getTertiaryData(blockIndex - CHUNK_LAYER + CHUNK_SIZE);
    //            // Grab height and position
    //            floraHeight = VoxelBits::getFloraHeight(tertiaryData);
    //            floraYpos = VoxelBits::getFloraPosition(tertiaryData);
    //        }
    //    } else {
    //        return;
    //    }
    //}
    //tertiaryData = 0;
    //floraHeight += 1; // add one since we are bigger now
    //// Add 1 to the tertiary data
    //VoxelBits::setFloraHeight(tertiaryData, floraHeight);
    //VoxelBits::setFloraPosition(tertiaryData, floraYpos + 1);
    //// Set it
    //chunk->setTertiaryData(blockIndex, tertiaryData);
    //// Loop downwards through all flora blocks of the same type and increase their height by 1
    //while (true) {
    //    // Move the index down
    //    if (blockIndex >= CHUNK_LAYER) {
    //        blockIndex -= CHUNK_LAYER;
    //    } else {
    //        if (chunk->bottom && chunk->bottom->isAccessible) {
    //            chunk = chunk->bottom;
    //        } else {
    //            return;
    //        }
    //        blockIndex = blockIndex - CHUNK_LAYER + CHUNK_SIZE;
    //    }

    //    // Loop while this is the same block type
    //    if (chunk->getBlockID(blockIndex) == blockID) {
    //        tertiaryData = chunk->getTertiaryData(blockIndex);
    //        // Set new flora height
    //        VoxelBits::setFloraHeight(tertiaryData, floraHeight);
    //        chunk->setTertiaryData(blockIndex, tertiaryData);
    //    } else {
    //        return;
    //    }

    //}
}

void ChunkUpdater::removeFlora(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int blockID) {
    //// Grab tertiary data
    //ui16 tertiaryData = chunk->getTertiaryData(blockIndex);
    //// Grab height and position
    //ui16 floraYpos = VoxelBits::getFloraPosition(tertiaryData);
    //// Set tertiary data to 0
    //chunk->setTertiaryData(blockIndex, 0);

    //// Recursively kill above flora blocks
    //blockIndex += CHUNK_LAYER;
    //if (blockIndex < CHUNK_SIZE) {
    //    if (chunk->getBlockID(blockIndex) == blockID) {
    //        removeBlockSafe(chunkManager, physicsEngine, chunk, lockedChunk, blockIndex, true);
    //    }
    //} else if (chunk->top && chunk->top->isAccessible) {
    //    blockIndex -= CHUNK_SIZE;
    //    if (chunk->top->getBlockID(blockIndex) == blockID) {
    //        removeBlockSafe(chunkManager, physicsEngine, chunk->top, lockedChunk, blockIndex, true);
    //    }
    //}
}

float ChunkUpdater::getBurnProbability(Chunk* chunk, Chunk*& lockedChunk, int blockIndex)
{

    //float flammability = 0.0f;
    //// Bottom
    //int bt = vvox::getBottomBlockData(chunk, lockedChunk, blockIndex);
    //if (bt == -1) return 0.0f;
    //flammability += GETBLOCK(bt).flammability;
    //// Left
    //bt = vvox::getLeftBlockData(chunk, lockedChunk, blockIndex);
    //if (bt == -1) return 0.0f;
    //flammability += GETBLOCK(bt).flammability;
    //// Right
    //bt = vvox::getRightBlockData(chunk, lockedChunk, blockIndex);
    //if (bt == -1) return 0.0f;
    //flammability += GETBLOCK(bt).flammability;
    //// Back
    //bt = vvox::getBackBlockData(chunk, lockedChunk, blockIndex);
    //if (bt == -1) return 0.0f;
    //flammability += GETBLOCK(bt).flammability;
    //// Front
    //bt = vvox::getFrontBlockData(chunk, lockedChunk, blockIndex);
    //if (bt == -1) return 0.0f;
    //flammability += GETBLOCK(bt).flammability;
    //// Top
    //bt = vvox::getTopBlockData(chunk, lockedChunk, blockIndex);
    //if (bt == -1) return 0.0f;
    //flammability += GETBLOCK(bt).flammability;

    //if (flammability < 0) return 0.0f;

    //return flammability / 6.0f;
    return 0.0f;
}

void ChunkUpdater::updateFireBlock(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk* chunk, int blockIndex) {
    ////left
    //int blockIndex2, blockIndex3, blockIndex4;
    //Chunk *owner2, *owner3, *owner4;
    //int bt;

    //const i32v3 pos = getPosFromBlockIndex(blockIndex);

    //const f32 sideTopMult = 1.5f;
    //const f32 topMult = 2.0f;
    //const f32 sideBotMult = 0.5f;
    //const f32 botMult = 0.8f;

    //Chunk* lockedChunk = nullptr;

    //burnAdjacentBlocks(chunkManager, physicsEngine, chunk, lockedChunk, blockIndex);

    ////********************************************************left
    //bt = vvox::getLeftBlockData(chunk, lockedChunk, blockIndex, pos.x, blockIndex2, owner2);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex2, lockedChunk, bt, owner2);

    ////left front
    //bt = vvox::getFrontBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3);

    ////left back
    //bt = vvox::getBackBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3);

    ////left top
    //bt = vvox::getTopBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideTopMult);


    ////left top front
    //bt = vvox::getFrontBlockData(owner3, lockedChunk, blockIndex3, blockIndex4, owner4);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex4, lockedChunk, bt, owner4, sideTopMult);

    ////left top back
    //bt = vvox::getBackBlockData(owner3, lockedChunk, blockIndex3, blockIndex4, owner4);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex4, lockedChunk, bt, owner4, sideTopMult);

    ////left bottom
    //bt = vvox::getBottomBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideBotMult);

    ////left bottom front
    //bt = vvox::getFrontBlockData(owner3, lockedChunk, blockIndex3, blockIndex4, owner4);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex4, lockedChunk, bt, owner4, sideBotMult);

    ////left bottom back
    //bt = vvox::getBackBlockData(owner3, lockedChunk, blockIndex3, blockIndex4, owner4);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex4, lockedChunk, bt, owner4, sideBotMult);

    ////********************************************************right
    //bt = vvox::getRightBlockData(chunk, lockedChunk, blockIndex, pos.x, blockIndex2, owner2);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex2, lockedChunk, bt, owner2);

    ////left front
    //bt = vvox::getFrontBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3);

    ////left back
    //bt = vvox::getBackBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3);

    ////left top
    //bt = vvox::getTopBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideTopMult);


    ////left top front
    //bt = vvox::getFrontBlockData(owner3, lockedChunk, blockIndex3, blockIndex4, owner4);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex4, lockedChunk, bt, owner4, sideTopMult);

    ////left top back
    //bt = vvox::getBackBlockData(owner3, lockedChunk, blockIndex3, blockIndex4, owner4);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex4, lockedChunk, bt, owner4, sideTopMult);

    ////left bottom
    //bt = vvox::getBottomBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideBotMult);

    ////left bottom front
    //bt = vvox::getFrontBlockData(owner3, lockedChunk, blockIndex3, blockIndex4, owner4);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex4, lockedChunk, bt, owner4, sideBotMult);

    ////left bottom back
    //bt = vvox::getBackBlockData(owner3, lockedChunk, blockIndex3, blockIndex4, owner4);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex4, lockedChunk, bt, owner4, sideBotMult);

    ////******************************************************front
    //bt = vvox::getFrontBlockData(chunk, lockedChunk, blockIndex, pos.z, blockIndex2, owner2);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex2, lockedChunk, bt, owner2);

    ////front top
    //bt = vvox::getTopBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideTopMult);

    ////front bottom
    //bt = vvox::getBottomBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideBotMult);

    ////********************************************************back
    //bt = vvox::getBackBlockData(chunk, lockedChunk, blockIndex, pos.z, blockIndex2, owner2);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex2, lockedChunk, bt, owner2);

    ////back top
    //bt = vvox::getTopBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideTopMult);

    ////back bottom
    //bt = vvox::getBottomBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideBotMult);

    ////********************************************************top
    //bt = vvox::getTopBlockData(chunk, lockedChunk, blockIndex, pos.y, blockIndex2, owner2);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex2, lockedChunk, bt, owner2, topMult);

    ////top front
    //bt = vvox::getFrontBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideTopMult);

    ////top back
    //bt = vvox::getBackBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideTopMult);


    ////********************************************************bottom
    //bt = vvox::getBottomBlockData(chunk, lockedChunk, blockIndex, pos.y, blockIndex2, owner2);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex2, lockedChunk, bt, owner2, botMult);

    ////bottom front
    //bt = vvox::getFrontBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideBotMult);

    ////bottom back
    //bt = vvox::getBackBlockData(owner2, lockedChunk, blockIndex2, blockIndex3, owner3);
    //if (bt == -1) { if (lockedChunk) { lockedChunk->unlock(); }; return; }
    //checkBurnBlock(blockIndex3, lockedChunk, bt, owner3, sideBotMult);

    //removeBlockSafe(chunkManager, physicsEngine, chunk, lockedChunk, blockIndex, false);

    //if (lockedChunk) lockedChunk->unlock();
}

void ChunkUpdater::burnAdjacentBlocks(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk* chunk, Chunk*& lockedChunk, int blockIndex){

    //int blockIndex2;
    //Chunk *owner2;
    //Block *b;

    //const i32v3 pos = getPosFromBlockIndex(blockIndex);

    //int bt = vvox::getBottomBlockData(chunk, lockedChunk, blockIndex, pos.y, blockIndex2, owner2);
    //b = &(GETBLOCK(bt));
    //if (b->flammability){
    //    if (b->burnTransformID == NONE){
    //        removeBlockSafe(chunkManager, physicsEngine, owner2, lockedChunk, blockIndex2, true);
    //    } else{
    //        if (Blocks[b->burnTransformID].emitter){
    //            particleEngine.addEmitter(Blocks[b->burnTransformID].emitter, glm::dvec3(owner2->voxelPosition.x + blockIndex2%CHUNK_WIDTH, owner2->voxelPosition.y + blockIndex2 / CHUNK_LAYER, owner2->voxelPosition.z + (blockIndex2%CHUNK_LAYER) / CHUNK_WIDTH), b->burnTransformID);
    //        }
    //        owner2->setBlockDataSafe(lockedChunk, blockIndex2, b->burnTransformID);
    //    }
    //    owner2->changeState(ChunkStates::MESH);
    //}
    ////left
    //bt = vvox::getLeftBlockData(chunk, lockedChunk, blockIndex, pos.x, blockIndex2, owner2);
    //b = &(GETBLOCK(bt));
    //if (b->flammability){
    //    if (b->burnTransformID == NONE){
    //        removeBlockSafe(chunkManager, physicsEngine, owner2, lockedChunk, blockIndex2, true);
    //    } else{
    //        if (Blocks[b->burnTransformID].emitter){
    //            particleEngine.addEmitter(Blocks[b->burnTransformID].emitter, glm::dvec3(owner2->voxelPosition.x + blockIndex2%CHUNK_WIDTH, owner2->voxelPosition.y + blockIndex2 / CHUNK_LAYER, owner2->voxelPosition.z + (blockIndex2%CHUNK_LAYER) / CHUNK_WIDTH), b->burnTransformID);
    //        }
    //        owner2->setBlockDataSafe(lockedChunk, blockIndex2, b->burnTransformID);
    //    }
    //    owner2->changeState(ChunkStates::MESH);
    //}
    ////right
    //bt = vvox::getRightBlockData(chunk, lockedChunk, blockIndex, pos.x, blockIndex2, owner2);
    //b = &(GETBLOCK(bt));
    //if (b->flammability){
    //    if (b->burnTransformID == NONE){
    //        removeBlockSafe(chunkManager, physicsEngine, owner2, lockedChunk, blockIndex2, true);
    //    } else{
    //        if (Blocks[b->burnTransformID].emitter){
    //            particleEngine.addEmitter(Blocks[b->burnTransformID].emitter, glm::dvec3(owner2->voxelPosition.x + blockIndex2%CHUNK_WIDTH, owner2->voxelPosition.y + blockIndex2 / CHUNK_LAYER, owner2->voxelPosition.z + (blockIndex2%CHUNK_LAYER) / CHUNK_WIDTH), b->burnTransformID);
    //        }
    //        owner2->setBlockDataSafe(lockedChunk, blockIndex2, b->burnTransformID);
    //    }
    //    owner2->changeState(ChunkStates::MESH);
    //}
    ////back
    //bt = vvox::getBackBlockData(chunk, lockedChunk, blockIndex, pos.z, blockIndex2, owner2);
    //b = &(GETBLOCK(bt));
    //if (b->flammability){
    //    if (b->burnTransformID == NONE){
    //        removeBlockSafe(chunkManager, physicsEngine, owner2, lockedChunk, blockIndex2, true);
    //    } else{
    //        if (Blocks[b->burnTransformID].emitter){
    //            particleEngine.addEmitter(Blocks[b->burnTransformID].emitter, glm::dvec3(owner2->voxelPosition.x + blockIndex2%CHUNK_WIDTH, owner2->voxelPosition.y + blockIndex2 / CHUNK_LAYER, owner2->voxelPosition.z + (blockIndex2%CHUNK_LAYER) / CHUNK_WIDTH), b->burnTransformID);
    //        }
    //        owner2->setBlockDataSafe(lockedChunk, blockIndex2, b->burnTransformID);
    //    }
    //    owner2->changeState(ChunkStates::MESH);
    //}
    ////front
    //bt = vvox::getFrontBlockData(chunk, lockedChunk, blockIndex, pos.z, blockIndex2, owner2);
    //b = &(GETBLOCK(bt));
    //if (b->flammability){
    //    if (b->burnTransformID == NONE){
    //        removeBlockSafe(chunkManager, physicsEngine, owner2, lockedChunk, blockIndex2, true);
    //    } else{
    //        if (Blocks[b->burnTransformID].emitter){
    //            particleEngine.addEmitter(Blocks[b->burnTransformID].emitter, glm::dvec3(owner2->voxelPosition.x + blockIndex2%CHUNK_WIDTH, owner2->voxelPosition.y + blockIndex2 / CHUNK_LAYER, owner2->voxelPosition.z + (blockIndex2%CHUNK_LAYER) / CHUNK_WIDTH), b->burnTransformID);
    //        }
    //        owner2->setBlockDataSafe(lockedChunk, blockIndex2, b->burnTransformID);
    //    }
    //    owner2->changeState(ChunkStates::MESH);
    //}
    ////top
    //bt = vvox::getTopBlockData(chunk, lockedChunk, blockIndex, pos.y, blockIndex2, owner2);
    //b = &(GETBLOCK(bt));
    //if (b->flammability){
    //    if (b->burnTransformID == NONE){
    //        removeBlockSafe(chunkManager, physicsEngine, owner2, lockedChunk, blockIndex2, true);
    //    } else{
    //        if (Blocks[b->burnTransformID].emitter){
    //            particleEngine.addEmitter(Blocks[b->burnTransformID].emitter, glm::dvec3(owner2->voxelPosition.x + blockIndex2%CHUNK_WIDTH, owner2->voxelPosition.y + blockIndex2 / CHUNK_LAYER, owner2->voxelPosition.z + (blockIndex2%CHUNK_LAYER) / CHUNK_WIDTH), b->burnTransformID);
    //        }
    //        owner2->setBlockDataSafe(lockedChunk, blockIndex2, b->burnTransformID);
    //    }
    //    owner2->changeState(ChunkStates::MESH);
    //}
}

void ChunkUpdater::checkBurnBlock(int blockIndex, Chunk*& lockedChunk, int blockType, Chunk *owner, float burnMult)
{
   /* float burnProb;
    if ((blockType == NONE || GETBLOCK(blockType).waterBreak)){
        burnProb = getBurnProbability(owner, lockedChunk, blockIndex) * burnMult;
        if (burnProb > 0){
            float r = rand() / (float)RAND_MAX;
            if (r <= burnProb){
               placeBlockSafe(owner, lockedChunk, blockIndex, FIRE);
            }
        }
    }*/
}
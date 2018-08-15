#include "stdafx.h"
#include "VoxelLightEngine.h"

#include "Errors.h"
#include "VoxelNavigation.inl"

// TODO: Do we still want this system as is? If so reimplement and remove VORB_UNUSED tags.

void VoxelLightEngine::calculateLight(Chunk* chunk VORB_UNUSED)
{
    ////Flush all edge queues
    //_lockedChunk = nullptr;

    ////Sunlight Calculation
    //if (chunk->sunlightRemovalQueue.size()) {
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //    //Removal
    //    while (chunk->sunlightRemovalQueue.size()){
    //        auto& node = chunk->sunlightRemovalQueue.front();
    //        removeSunlightBFS(chunk, (int)node.blockIndex, node.oldLightVal);
    //        chunk->sunlightRemovalQueue.pop();
    //    }
    //    std::queue<SunlightRemovalNode>().swap(chunk->sunlightRemovalQueue); //forces memory to be freed
    //}

    //if (chunk->sunlightUpdateQueue.size()) {
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //    //Addition
    //    while (chunk->sunlightUpdateQueue.size()){
    //        auto& node = chunk->sunlightUpdateQueue.front();
    //        placeSunlightBFS(chunk, (int)node.blockIndex, (int)node.lightVal);
    //        chunk->sunlightUpdateQueue.pop();
    //    }
    //    std::queue<SunlightUpdateNode>().swap(chunk->sunlightUpdateQueue); //forces memory to be freed
    //}

    ////Voxel Light Calculation
    //if (chunk->lampLightRemovalQueue.size()) {
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //    //Removal
    //    while (chunk->lampLightRemovalQueue.size()){
    //        auto& node = chunk->lampLightRemovalQueue.front();
    //        removeLampLightBFS(chunk, (int)node.blockIndex, node.oldLightColor);
    //        chunk->lampLightRemovalQueue.pop();
    //    }
    //    std::queue<LampLightRemovalNode>().swap(chunk->lampLightRemovalQueue); //forces memory to be freed
    //}

    //if (chunk->lampLightUpdateQueue.size()) {
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //    //Addition
    //    while (chunk->lampLightUpdateQueue.size()) {
    //        auto& node = chunk->lampLightUpdateQueue.front();
    //        placeLampLightBFS(chunk, (int)node.blockIndex, node.lightColor);
    //        chunk->lampLightUpdateQueue.pop();
    //    }
    //    std::queue<LampLightUpdateNode>().swap(chunk->lampLightUpdateQueue); //forces memory to be freed
    //}
    //if (_lockedChunk) {
    //    _lockedChunk->unlock();
    //    _lockedChunk = nullptr;
    //}
}

void VoxelLightEngine::calculateSunlightExtend(Chunk* chunk VORB_UNUSED)
{
    //int blockIndex;
    //int y;

    //_lockedChunk = nullptr;
    //vvox::swapLockedChunk(chunk, _lockedChunk);

    //for (ui32 i = 0; i < chunk->sunExtendList.size(); i++){
    //    blockIndex = chunk->sunExtendList[i];
    //    if (chunk->getSunlight(blockIndex) == MAXLIGHT){
    //        y = blockIndex / CHUNK_LAYER;
    //        extendSunRay(chunk, blockIndex - y * CHUNK_LAYER, y);
    //    }
    //}
    //std::vector<GLushort>().swap(chunk->sunExtendList); //forces memory to be freed
    //if (_lockedChunk) {
    //    _lockedChunk->unlock();
    //    _lockedChunk = nullptr;
    //}
}

void VoxelLightEngine::calculateSunlightRemoval(Chunk* chunk VORB_UNUSED)
{
    //int blockIndex;
    //int y;

    //_lockedChunk = nullptr;
    //vvox::swapLockedChunk(chunk, _lockedChunk);

    //for (ui32 i = 0; i < chunk->sunRemovalList.size(); i++){
    //    blockIndex = chunk->sunRemovalList[i];
    //    if (chunk->getSunlight(blockIndex) == 0){
    //        y = blockIndex / CHUNK_LAYER;
    //        blockSunRay(chunk, blockIndex - y * CHUNK_LAYER, y - 1);
    //    }
    //}
    //std::vector<GLushort>().swap(chunk->sunRemovalList); //forces memory to be freed
    //if (_lockedChunk) {
    //    _lockedChunk->unlock();
    //    _lockedChunk = nullptr;
    //}
}

//Check for sun rays from the top chunk
void VoxelLightEngine::checkTopForSunlight(Chunk* chunk VORB_UNUSED)
{
    //int blockIndex;
    //ui16 topLight;

    //static ui16 topSunlight[CHUNK_LAYER];

    //if (chunk->top && chunk->top->isAccessible) {
    //    // First grab the sunlight from the top
    //    chunk->top->lock();
    //    for (int i = 0; i < CHUNK_LAYER; i++) {
    //        topSunlight[i] = chunk->top->getSunlight(i);
    //    }
    //    chunk->top->unlock();
    //    // Now check for sun extend
    //    chunk->lock();
    //    for (int i = 0; i < CHUNK_LAYER; i++) {
    //        blockIndex = i + 31 * CHUNK_LAYER;
    //        topLight = topSunlight[i];

    //        if ((chunk->getBlock(blockIndex).blockLight == 0) && topLight == MAXLIGHT) {
    //            chunk->setSunlight(blockIndex, MAXLIGHT);
    //            chunk->sunExtendList.push_back(blockIndex);
    //        } else if (topLight == 0) { //if the top is blocking sunlight
    //            if (chunk->getSunlight(blockIndex) == MAXLIGHT) {
    //                chunk->setSunlight(blockIndex, 0);
    //                chunk->sunRemovalList.push_back(blockIndex);
    //            }
    //        }
    //    }
    //    chunk->unlock();
    //}
}

void VoxelLightEngine::blockSunRay(Chunk* chunk VORB_UNUSED, int xz VORB_UNUSED, int y VORB_UNUSED)
{
    //int i = y; //start at the current block
    //chunk->changeState(ChunkStates::MESH);
   
    //while (true){ //do the light removal iteration
    //    if (i == -1){ //bottom chunk
    //        if (chunk->bottom && chunk->bottom->isAccessible){
    //            vvox::swapLockedChunk(chunk->bottom, _lockedChunk);
    //            VoxelLightEngine::blockSunRay(chunk->bottom, xz, 31); //continue the algorithm
    //            vvox::swapLockedChunk(chunk, _lockedChunk);
    //        }
    //        return;
    //    } else{
    //        if (chunk->getSunlight(xz + i*CHUNK_LAYER) == MAXLIGHT){
    //            chunk->setSunlight(xz + i*CHUNK_LAYER, 0);
    //            VoxelLightEngine::removeSunlightBFS(chunk, xz + i*CHUNK_LAYER, MAXLIGHT);
    //        } else{
    //            return;
    //        }
    //        i--;
    //    }
    //}

}

void VoxelLightEngine::extendSunRay(Chunk* chunk VORB_UNUSED, int xz VORB_UNUSED, int y VORB_UNUSED)
{
    //int i = y; //start at the current block, for extension to other chunks
    //int blockIndex;
    //chunk->changeState(ChunkStates::MESH);
    //while (true){
    //    if (i == -1){
    //        if (chunk->bottom && chunk->bottom->isAccessible){
    //            vvox::swapLockedChunk(chunk->bottom, _lockedChunk);
    //            extendSunRay(chunk->bottom, xz, 31); //continue the algorithm
    //            vvox::swapLockedChunk(chunk, _lockedChunk);
    //        }
    //        return;
    //    } else{
    //        blockIndex = xz + i*CHUNK_LAYER;
    //        if (chunk->getBlock(blockIndex).blockLight == 0){

    //            if (chunk->getSunlight(blockIndex) != MAXLIGHT){
    //                chunk->sunlightUpdateQueue.emplace(blockIndex, MAXLIGHT);
    //            }
    //        
    //        } else{
    //            return;
    //        }
    //    }
    //    i--;
    //}
}

inline void removeSunlightNeighborUpdate(Chunk* chunk VORB_UNUSED, int blockIndex VORB_UNUSED, ui16 light VORB_UNUSED) {
   /* ui8 lightVal = chunk->getSunlight(blockIndex);
    if (lightVal > 0){
        if (lightVal <= light){
            chunk->setSunlight(blockIndex, 0);
            chunk->sunlightRemovalQueue.emplace(blockIndex, light);
        } else {
            chunk->sunlightUpdateQueue.emplace(blockIndex, 0);
        }
    }*/
}

void VoxelLightEngine::removeSunlightBFS(Chunk* chunk VORB_UNUSED, int blockIndex VORB_UNUSED, ui8 oldLightVal VORB_UNUSED)
{
    //ui8 nextIntensity;
    //if (oldLightVal > 0) {
    //    nextIntensity = oldLightVal - 1;
    //} else {
    //    nextIntensity = 0;
    //}

    //int x = blockIndex % CHUNK_WIDTH;
    //int y = blockIndex / CHUNK_LAYER;
    //int z = (blockIndex % CHUNK_LAYER) / CHUNK_WIDTH;

    //Chunk*& left = chunk->left;
    //Chunk*& right = chunk->right;
    //Chunk*& back = chunk->back;
    //Chunk*& front = chunk->front;
    //Chunk*& top = chunk->top;
    //Chunk*& bottom = chunk->bottom;

    //if (x > 0){ //left
    //    removeSunlightNeighborUpdate(chunk, blockIndex - 1, nextIntensity);
    //} else if (left && left->isAccessible){
    //    vvox::swapLockedChunk(left, _lockedChunk);
    //    removeSunlightNeighborUpdate(left, blockIndex + CHUNK_WIDTH - 1, nextIntensity);
    //    left->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //if (x < CHUNK_WIDTH - 1){ //right
    //    removeSunlightNeighborUpdate(chunk, blockIndex + 1, nextIntensity);
    //} else if (right && right->isAccessible){
    //    vvox::swapLockedChunk(right, _lockedChunk);
    //    removeSunlightNeighborUpdate(right, blockIndex - CHUNK_WIDTH + 1, nextIntensity);
    //    right->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //if (z > 0){ //back
    //    removeSunlightNeighborUpdate(chunk, blockIndex - CHUNK_WIDTH, nextIntensity);
    //} else if (back && back->isAccessible){
    //    vvox::swapLockedChunk(back, _lockedChunk);
    //    removeSunlightNeighborUpdate(back, blockIndex + CHUNK_LAYER - CHUNK_WIDTH, nextIntensity);
    //    back->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //if (z < CHUNK_WIDTH - 1){ //front
    //    removeSunlightNeighborUpdate(chunk, blockIndex + CHUNK_WIDTH, nextIntensity);
    //} else if (front && front->isAccessible){
    //    vvox::swapLockedChunk(front, _lockedChunk);
    //    removeSunlightNeighborUpdate(front, blockIndex - CHUNK_LAYER + CHUNK_WIDTH, nextIntensity);
    //    front->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //if (y > 0){ //bottom
    //    removeSunlightNeighborUpdate(chunk, blockIndex - CHUNK_LAYER, nextIntensity);
    //} else if (bottom && bottom->isAccessible){
    //    vvox::swapLockedChunk(bottom, _lockedChunk);
    //    removeSunlightNeighborUpdate(bottom, CHUNK_SIZE - CHUNK_LAYER + blockIndex, nextIntensity);
    //    bottom->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //if (y < CHUNK_WIDTH - 1){ //top
    //    removeSunlightNeighborUpdate(chunk, blockIndex + CHUNK_LAYER, nextIntensity);
    //} else if (top && top->isAccessible){
    //    vvox::swapLockedChunk(top, _lockedChunk);
    //    removeSunlightNeighborUpdate(top, blockIndex - CHUNK_SIZE + CHUNK_LAYER, nextIntensity);
    //    top->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}
    //chunk->changeState(ChunkStates::MESH);

}

inline void placeSunlightNeighborUpdate(Chunk* chunk VORB_UNUSED, int blockIndex VORB_UNUSED, ui16 light VORB_UNUSED) {
    //if (chunk->getSunlight(blockIndex) < light){
    //    if (chunk->getBlock(blockIndex).allowLight){
    //        chunk->setSunlight(blockIndex, (ui8)light); // TODO(Ben) Wrong type?
    //        chunk->sunlightUpdateQueue.emplace(blockIndex, light);
    //    }
    //}
}

void VoxelLightEngine::placeSunlightBFS(Chunk* chunk VORB_UNUSED, int blockIndex VORB_UNUSED, ui8 intensity VORB_UNUSED)
{
    //if (intensity > chunk->getSunlight(blockIndex)) {
    //    //Set the light value
    //    chunk->setSunlight(blockIndex, intensity);
    //} else {
    //    //If intensity is less that the actual light value, use the actual
    //    intensity = chunk->getSunlight(blockIndex);
    //}

    //if (intensity <= 1) return;
    ////Reduce by 1 to prevent a bunch of -1
    //ui8 newIntensity = (intensity - 1);

    //chunk->dirty = 1;

    //int x = blockIndex % CHUNK_WIDTH;
    //int y = blockIndex / CHUNK_LAYER;
    //int z = (blockIndex % CHUNK_LAYER) / CHUNK_WIDTH;

    //Chunk*& left = chunk->left;
    //Chunk*& right = chunk->right;
    //Chunk*& back = chunk->back;
    //Chunk*& front = chunk->front;
    //Chunk*& top = chunk->top;
    //Chunk*& bottom = chunk->bottom;

    //if (x > 0){ //left
    //    placeSunlightNeighborUpdate(chunk, blockIndex - 1, newIntensity);
    //} else if (left && left->isAccessible){
    //    vvox::swapLockedChunk(left, _lockedChunk);
    //    placeSunlightNeighborUpdate(left, blockIndex + CHUNK_WIDTH - 1, newIntensity);
    //    left->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //if (x < CHUNK_WIDTH - 1){ //right
    //    placeSunlightNeighborUpdate(chunk, blockIndex + 1, newIntensity);
    //} else if (right && right->isAccessible){
    //    vvox::swapLockedChunk(right, _lockedChunk);
    //    placeSunlightNeighborUpdate(right, blockIndex - CHUNK_WIDTH + 1, newIntensity);
    //    right->changeState(ChunkStates::MESH); 
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //if (z > 0){ //back
    //    placeSunlightNeighborUpdate(chunk, blockIndex - CHUNK_WIDTH, newIntensity);
    //} else if (back && back->isAccessible){
    //    vvox::swapLockedChunk(back, _lockedChunk);
    //    placeSunlightNeighborUpdate(back, blockIndex + CHUNK_LAYER - CHUNK_WIDTH, newIntensity);
    //    back->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //if (z < CHUNK_WIDTH - 1){ //front
    //    placeSunlightNeighborUpdate(chunk, blockIndex + CHUNK_WIDTH, newIntensity);
    //} else if (front && front->isAccessible){
    //    vvox::swapLockedChunk(front, _lockedChunk);
    //    placeSunlightNeighborUpdate(front, blockIndex - CHUNK_LAYER + CHUNK_WIDTH, newIntensity);
    //    front->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //if (y > 0){ //bottom
    //    placeSunlightNeighborUpdate(chunk, blockIndex - CHUNK_LAYER, newIntensity);
    //} else if (bottom && bottom->isAccessible){
    //    vvox::swapLockedChunk(bottom, _lockedChunk);
    //    placeSunlightNeighborUpdate(bottom, CHUNK_SIZE - CHUNK_LAYER + blockIndex, newIntensity);
    //    bottom->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //if (y < CHUNK_WIDTH - 1){ //top
    //    placeSunlightNeighborUpdate(chunk, blockIndex + CHUNK_LAYER, newIntensity);
    //} else if (top && top->isAccessible){
    //    vvox::swapLockedChunk(top, _lockedChunk);
    //    placeSunlightNeighborUpdate(top, blockIndex - CHUNK_SIZE + CHUNK_LAYER, newIntensity);
    //    top->changeState(ChunkStates::MESH);
    //    vvox::swapLockedChunk(chunk, _lockedChunk);
    //}

    //chunk->changeState(ChunkStates::MESH);
}

inline ui16 getMaxLampColors(const ui16 redA VORB_UNUSED, const ui16 greenA VORB_UNUSED, const ui16 blueA VORB_UNUSED, const ui16 b VORB_UNUSED) {
    /*   ui16 redB = b & LAMP_RED_MASK;
       ui16 greenB = b & LAMP_GREEN_MASK;
       ui16 blueB = b & LAMP_BLUE_MASK;
       return MAX(redA, redB) | MAX(greenA, greenB) | MAX(blueA, blueB);*/
       return 0;
}

inline void getLampColors(const ui16 l VORB_UNUSED, ui16 &r VORB_UNUSED, ui16 &g VORB_UNUSED, ui16 &b VORB_UNUSED) {
    /*  r = l & LAMP_RED_MASK;
      g = l & LAMP_GREEN_MASK;
      b = l & LAMP_BLUE_MASK;*/
}

inline void removeLampNeighborUpdate(Chunk* chunk VORB_UNUSED, int blockIndex VORB_UNUSED, ui16 intensityRed VORB_UNUSED, ui16 intensityGreen VORB_UNUSED, ui16 intensityBlue VORB_UNUSED, ui16 light VORB_UNUSED) {
   /* ui16 nextRed, nextGreen, nextBlue;
    ui16 nextLight = chunk->getLampLight(blockIndex);
    getLampColors(nextLight, nextRed, nextGreen, nextBlue);

    if ((nextRed && nextRed <= intensityRed) || (nextGreen && nextGreen <= intensityGreen) || (nextBlue && nextBlue <= intensityBlue)){
        chunk->setLampLight(blockIndex, 0);
        chunk->lampLightRemovalQueue.emplace(blockIndex, light);

        if (nextRed > intensityRed || nextGreen > intensityGreen || nextBlue > intensityBlue){
            chunk->lampLightUpdateQueue.emplace(blockIndex, 0);
        }
    } else if (nextLight > 0) {
        chunk->lampLightUpdateQueue.emplace(blockIndex, 0);
    }*/
}

void VoxelLightEngine::removeLampLightBFS(Chunk* chunk VORB_UNUSED, int blockIndex VORB_UNUSED, ui16 light VORB_UNUSED)
{
//#define RED1 0x400
//#define GREEN1 0x20
//#define BLUE1 1
//
//    ui16 intensityRed = light & LAMP_RED_MASK;
//    ui16 intensityGreen = light & LAMP_GREEN_MASK;
//    ui16 intensityBlue = light & LAMP_BLUE_MASK;
//
//    //Reduce by 1
//    if (intensityRed != 0) {
//        intensityRed -= RED1;
//        light -= RED1;
//    }
//    if (intensityGreen != 0) {
//        intensityGreen -= GREEN1;
//        light -= GREEN1;
//    }
//    if (intensityBlue != 0) {
//        intensityBlue -= BLUE1;
//        light -= BLUE1;
//    }
//
//    int x = blockIndex % CHUNK_WIDTH;
//    int y = blockIndex / CHUNK_LAYER;
//    int z = (blockIndex % CHUNK_LAYER) / CHUNK_WIDTH;
//
//    Chunk*& left = chunk->left;
//    Chunk*& right = chunk->right;
//    Chunk*& back = chunk->back;
//    Chunk*& front = chunk->front;
//    Chunk*& top = chunk->top;
//    Chunk*& bottom = chunk->bottom;
//
//    if (x > 0){ //left
//        removeLampNeighborUpdate(chunk, blockIndex - 1, intensityRed, intensityGreen, intensityBlue, light);    
//    } else if (left && left->isAccessible){
//        vvox::swapLockedChunk(left, _lockedChunk);
//        removeLampNeighborUpdate(left, blockIndex + CHUNK_WIDTH - 1, intensityRed, intensityGreen, intensityBlue, light);
//        left->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//
//    if (x < CHUNK_WIDTH - 1){ //right
//        removeLampNeighborUpdate(chunk, blockIndex + 1, intensityRed, intensityGreen, intensityBlue, light);
//    } else if (right && right->isAccessible){
//        vvox::swapLockedChunk(right, _lockedChunk);
//        removeLampNeighborUpdate(right, blockIndex - CHUNK_WIDTH + 1, intensityRed, intensityGreen, intensityBlue, light);
//        right->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//
//    if (z > 0){ //back
//        removeLampNeighborUpdate(chunk, blockIndex - CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
//    } else if (back && back->isAccessible){
//        vvox::swapLockedChunk(back, _lockedChunk);
//        removeLampNeighborUpdate(back, blockIndex + CHUNK_LAYER - CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
//        back->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//
//    if (z < CHUNK_WIDTH - 1){ //front
//        removeLampNeighborUpdate(chunk, blockIndex + CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
//    } else if (front && front->isAccessible){
//        vvox::swapLockedChunk(front, _lockedChunk);
//        removeLampNeighborUpdate(front, blockIndex - CHUNK_LAYER + CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
//        front->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//
//    if (y > 0){ //bottom
//        removeLampNeighborUpdate(chunk, blockIndex - CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue, light);
//    } else if (bottom && bottom->isAccessible){
//        vvox::swapLockedChunk(bottom, _lockedChunk);
//        removeLampNeighborUpdate(bottom, CHUNK_SIZE - CHUNK_LAYER + blockIndex, intensityRed, intensityGreen, intensityBlue, light);
//        bottom->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//
//    if (y < CHUNK_WIDTH - 1){ //top
//        removeLampNeighborUpdate(chunk, blockIndex + CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue, light);
//    } else if (top && top->isAccessible){
//        vvox::swapLockedChunk(top, _lockedChunk);
//        removeLampNeighborUpdate(top, blockIndex - CHUNK_SIZE + CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue, light);
//        top->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//    chunk->changeState(ChunkStates::MESH);

}

inline void placeLampNeighborUpdate(Chunk* chunk VORB_UNUSED, int blockIndex VORB_UNUSED, ui16 intensityRed VORB_UNUSED, ui16 intensityGreen VORB_UNUSED, ui16 intensityBlue VORB_UNUSED) {
  /*  ui16 currentLight = chunk->getLampLight(blockIndex);
    const Block& block = chunk->getBlock(blockIndex);

    intensityRed = (ui16)((intensityRed >> LAMP_RED_SHIFT) * block.colorFilter.r) << LAMP_RED_SHIFT;
    intensityGreen = (ui16)((intensityGreen >> LAMP_GREEN_SHIFT) * block.colorFilter.g) << LAMP_GREEN_SHIFT;
    intensityBlue = (ui16)(intensityBlue * block.colorFilter.b);

    ui16 nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
    if (nextLight != currentLight){
        if (chunk->getBlock(blockIndex).allowLight){
            chunk->setLampLight(blockIndex, nextLight);
            chunk->lampLightUpdateQueue.emplace(blockIndex, nextLight);
        }
    }*/
}

void VoxelLightEngine::placeLampLightBFS(Chunk* chunk VORB_UNUSED, int blockIndex VORB_UNUSED, ui16 intensity VORB_UNUSED)
{
//#define RED1 0x400
//#define GREEN1 0x20
//#define BLUE1 1
//
//    ui16 currentLight = chunk->getLampLight(blockIndex);
//
//    ui16 currentRed = currentLight & LAMP_RED_MASK;
//    ui16 currentGreen = currentLight & LAMP_GREEN_MASK;
//    ui16 currentBlue = currentLight & LAMP_BLUE_MASK;
//
//    ui16 intensityRed = intensity & LAMP_RED_MASK;
//    ui16 intensityGreen = intensity & LAMP_GREEN_MASK;
//    ui16 intensityBlue = intensity & LAMP_BLUE_MASK;
//
//    intensityRed = MAX(currentRed, intensityRed);
//    intensityGreen = MAX(currentGreen, intensityGreen);
//    intensityBlue = MAX(currentBlue, intensityBlue);
//
//    const Block& currentBlock = chunk->getBlock(blockIndex);
//
//    intensityRed = (ui16)((intensityRed >> LAMP_RED_SHIFT) * currentBlock.colorFilter.r) << LAMP_RED_SHIFT;
//    intensityGreen = (ui16)((intensityGreen >> LAMP_GREEN_SHIFT) * currentBlock.colorFilter.g) << LAMP_GREEN_SHIFT;
//    intensityBlue = (ui16)(intensityBlue * currentBlock.colorFilter.b);
//    intensity = intensityRed | intensityGreen | intensityBlue;
//
//    if (intensity != currentLight) { 
//        //Set the light value
//        chunk->setLampLight(blockIndex, intensity);
//    }
//
//    if (intensityRed <= RED1 && intensityGreen <= GREEN1 && intensityBlue <= BLUE1) return;
//    //Reduce by 1
//    if (intensityRed != 0) {
//        intensityRed -= RED1;
//        intensity -= RED1;
//    }
//    if (intensityGreen != 0) {
//        intensityGreen -= GREEN1;
//        intensity -= GREEN1;
//    }
//    if (intensityBlue != 0) {
//        intensityBlue -= BLUE1;
//        intensity -= BLUE1;
//    }
//
//    chunk->dirty = 1;
//
//    int x = blockIndex % CHUNK_WIDTH;
//    int y = blockIndex / CHUNK_LAYER;
//    int z = (blockIndex % CHUNK_LAYER) / CHUNK_WIDTH;
//
//    Chunk*& left = chunk->left;
//    Chunk*& right = chunk->right;
//    Chunk*& back = chunk->back;
//    Chunk*& front = chunk->front;
//    Chunk*& top = chunk->top;
//    Chunk*& bottom = chunk->bottom;
//
//    if (x > 0){ //left
//        placeLampNeighborUpdate(chunk, blockIndex - 1, intensityRed, intensityGreen, intensityBlue);
//    } else if (left && left->isAccessible){
//        vvox::swapLockedChunk(left, _lockedChunk);
//        placeLampNeighborUpdate(left, blockIndex + CHUNK_WIDTH - 1, intensityRed, intensityGreen, intensityBlue);
//        left->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//
//    if (x < CHUNK_WIDTH - 1){ //right
//        placeLampNeighborUpdate(chunk, blockIndex + 1, intensityRed, intensityGreen, intensityBlue);
//    } else if (right && right->isAccessible){
//        vvox::swapLockedChunk(right, _lockedChunk);
//        placeLampNeighborUpdate(right, blockIndex - CHUNK_WIDTH + 1, intensityRed, intensityGreen, intensityBlue);
//        right->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//
//    if (z > 0){ //back
//        placeLampNeighborUpdate(chunk, blockIndex - CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue);
//    } else if (back && back->isAccessible){
//        vvox::swapLockedChunk(back, _lockedChunk);
//        placeLampNeighborUpdate(back, blockIndex + CHUNK_LAYER - CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue);
//        back->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//
//    if (z < CHUNK_WIDTH - 1){ //front
//        placeLampNeighborUpdate(chunk, blockIndex + CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue);
//    } else if (front && front->isAccessible){
//        vvox::swapLockedChunk(front, _lockedChunk);
//        placeLampNeighborUpdate(front, blockIndex - CHUNK_LAYER + CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue);
//        front->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//
//    if (y > 0){ //bottom
//        placeLampNeighborUpdate(chunk, blockIndex - CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue);
//    } else if (bottom && bottom->isAccessible){
//        vvox::swapLockedChunk(bottom, _lockedChunk);
//        placeLampNeighborUpdate(bottom, CHUNK_SIZE - CHUNK_LAYER + blockIndex, intensityRed, intensityGreen, intensityBlue);
//        bottom->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//    if (y < CHUNK_WIDTH - 1){ //top
//        placeLampNeighborUpdate(chunk, blockIndex + CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue);
//    } else if (top && top->isAccessible){
//        vvox::swapLockedChunk(top, _lockedChunk);
//        placeLampNeighborUpdate(top, blockIndex - CHUNK_SIZE + CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue);
//        top->changeState(ChunkStates::MESH);
//        vvox::swapLockedChunk(chunk, _lockedChunk);
//    }
//
//    chunk->changeState(ChunkStates::MESH);
}
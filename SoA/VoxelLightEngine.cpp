#include "stdafx.h"

#include "VoxelLightEngine.h"

#include "Chunk.h"
#include "Errors.h"

void VoxelLightEngine::calculateLight(Chunk* chunk)
{
    //Flush all edge queues
  /*  flushLightQueue(chunk, chunk->lightFromMain);
    flushLightQueue(chunk, chunk->lightFromRight);
    flushLightQueue(chunk, chunk->lightFromLeft);
    flushLightQueue(chunk, chunk->lightFromFront);
    flushLightQueue(chunk, chunk->lightFromBack);
    flushLightQueue(chunk, chunk->lightFromTop);
    flushLightQueue(chunk, chunk->lightFromBottom);*/


    //Sunlight Calculation
    if (chunk->sunlightRemovalQueue.size()) {
        //Removal
        for (ui32 i = 0; i < chunk->sunlightRemovalQueue.size(); i++){
            removeSunlightBFS(chunk, (int)chunk->sunlightRemovalQueue[i].blockIndex, chunk->sunlightRemovalQueue[i].oldLightVal);
        }
        vector<SunlightRemovalNode>().swap(chunk->sunlightRemovalQueue); //forces memory to be freed
    }

    if (chunk->sunlightUpdateQueue.size()) {
        //Addition
        for (ui32 i = 0; i < chunk->sunlightUpdateQueue.size(); i++){
            placeSunlightBFS(chunk, (int)chunk->sunlightUpdateQueue[i].blockIndex, (int)chunk->sunlightUpdateQueue[i].lightVal);
        }
        vector<SunlightUpdateNode>().swap(chunk->sunlightUpdateQueue); //forces memory to be freed
    }

    //Voxel Light Calculation
    if (chunk->lampLightRemovalQueue.size()) {
        //Removal
        for (ui32 i = 0; i < chunk->lampLightRemovalQueue.size(); i++){
            removeLampLightBFS(chunk, (int)chunk->lampLightRemovalQueue[i].blockIndex, chunk->lampLightRemovalQueue[i].oldLightColor);
        }
        vector<LampLightRemovalNode>().swap(chunk->lampLightRemovalQueue); //forces memory to be freed
    }

    if (chunk->lampLightUpdateQueue.size()) {
        //Addition
        for (ui32 i = 0; i < chunk->lampLightUpdateQueue.size(); i++){
            placeLampLightBFS(chunk, (int)chunk->lampLightUpdateQueue[i].blockIndex, chunk->lampLightUpdateQueue[i].lightColor);
        }
        vector<LampLightUpdateNode>().swap(chunk->lampLightUpdateQueue); //forces memory to be freed
    }

}

void VoxelLightEngine::flushLightQueue(Chunk* chunk, moodycamel::ReaderWriterQueue<ui32>& queue) {
    //ui32 data;
    //LightMessage result;
    //while (queue.try_dequeue(data)) {
    //    result = *((LightMessage*)&data);
    //    //If light value is < 0 that indicates a removal.
    //    if (result.lightValue < 0) {
    //        chunk->lightData[result.lightType][result.blockIndex] = 0;
    //        removeLightBFS(chunk, (int)result.blockIndex, (int)result.lightType, -result.lightValue);
    //    } else { //Else its an addition
    //        chunk->lightData[result.lightType][result.blockIndex] = result.lightValue;
    //        placeLight(chunk, (int)result.blockIndex, result.lightType, result.lightValue);
    //    }
    //}
}

void VoxelLightEngine::calculateSunlightExtend(Chunk* chunk)
{
    int blockIndex;
    int y;
    for (ui32 i = 0; i < chunk->sunExtendList.size(); i++){
        blockIndex = chunk->sunExtendList[i];
        if (chunk->getSunlight(blockIndex) == MAXLIGHT){
            y = blockIndex / CHUNK_LAYER;
            extendSunRay(chunk, blockIndex - y * CHUNK_LAYER, y);
        }
    }
    vector<GLushort>().swap(chunk->sunExtendList); //forces memory to be freed
}

void VoxelLightEngine::calculateSunlightRemoval(Chunk* chunk)
{
    int blockIndex;
    int y;
    for (ui32 i = 0; i < chunk->sunRemovalList.size(); i++){
        blockIndex = chunk->sunRemovalList[i];
        if (chunk->getSunlight(blockIndex) == 0){
            y = blockIndex / CHUNK_LAYER;
            blockSunRay(chunk, blockIndex - y * CHUNK_LAYER, y - 1);
        }
    }
    vector<GLushort>().swap(chunk->sunRemovalList); //forces memory to be freed
}

//Check for sun rays from the top chunk
void VoxelLightEngine::checkTopForSunlight(Chunk* chunk)
{
    int topLight, blockIndex;
    if (chunk->top && chunk->top->isAccessible){
        for (int i = 0; i < CHUNK_LAYER; i++){
            blockIndex = i + 31 * CHUNK_LAYER;
            topLight = chunk->top->getSunlight(i);
            //If the top has a sun ray
            if ((chunk->getBlock(blockIndex).blockLight == 0) && topLight == MAXLIGHT) {
                chunk->setSunlight(blockIndex, MAXLIGHT);
                chunk->sunExtendList.push_back(blockIndex);
            } else if (topLight == 0) { //if the top is blocking sunlight
                if (chunk->getSunlight(blockIndex) == MAXLIGHT) {
                    chunk->setSunlight(blockIndex, 0);
                    chunk->sunRemovalList.push_back(blockIndex);
                }
            }
        }
    }
}

void VoxelLightEngine::blockSunRay(Chunk* chunk, int xz, int y)
{
    int i = y; //start at the current block
    chunk->changeState(ChunkStates::MESH);
   
    while (true){ //do the light removal iteration
        if (i == -1){ //bottom chunk
            if (chunk->bottom && chunk->bottom->isAccessible){
                VoxelLightEngine::blockSunRay(chunk->bottom, xz, 31); //continue the algorithm
            }
            return;
        } else{
            if (chunk->getSunlight(xz + i*CHUNK_LAYER) == MAXLIGHT){
                chunk->setSunlight(xz + i*CHUNK_LAYER, 0);
                VoxelLightEngine::removeSunlightBFS(chunk, xz + i*CHUNK_LAYER, MAXLIGHT);
            } else{
                return;
            }
            i--;
        }
    }

}

void VoxelLightEngine::extendSunRay(Chunk* chunk, int xz, int y)
{
    int i = y; //start at the current block, for extension to other chunks
    int blockIndex;
    chunk->changeState(ChunkStates::MESH);
    while (true){
        if (i == -1){
            if (chunk->bottom && chunk->bottom->isAccessible){
                extendSunRay(chunk->bottom, xz, 31); //continue the algorithm
            }
            return;
        } else{
            blockIndex = xz + i*CHUNK_LAYER;
            if (chunk->getBlock(blockIndex).blockLight == 0){

                if (chunk->getSunlight(blockIndex) != MAXLIGHT){
                    chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(blockIndex, MAXLIGHT));
                }
            } else{
                return;
            }
        }
        i--;
    }
}

void VoxelLightEngine::removeSunlightBFS(Chunk* chunk, int blockIndex, ui8 oldLightVal)
{
    ui8 nextIntensity;
    if (oldLightVal > 0) {
        nextIntensity = oldLightVal - 1;
    } else {
        nextIntensity = 0;
    }

    int x = blockIndex % CHUNK_WIDTH;
    int y = blockIndex / CHUNK_LAYER;
    int z = (blockIndex % CHUNK_LAYER) / CHUNK_WIDTH;
    int nextBlockIndex;

    ui8 lightVal;

    Chunk* left = chunk->left;
    Chunk* right = chunk->right;
    Chunk* back = chunk->back;
    Chunk* front = chunk->front;
    Chunk* top = chunk->top;
    Chunk* bottom = chunk->bottom;

    if (x > 0){ //left
        nextBlockIndex = blockIndex - 1;
        lightVal = chunk->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                chunk->setSunlight(nextBlockIndex, 0);
                chunk->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
            } else {
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
            }
        }
    } else if (left && left->isAccessible){
        nextBlockIndex = blockIndex + CHUNK_WIDTH - 1;
        lightVal = left->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                left->setSunlight(nextBlockIndex, 0);
                left->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
             //   left->lightFromRight.enqueue(*((ui32*)&LightMessage(blockIndex + CHUNK_WIDTH - 1, type, -lightVal)));
            } else {
                left->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
             //   left->lightFromRight.enqueue(*((ui32*)&LightMessage(blockIndex + CHUNK_WIDTH - 1, type, 0)));
            }
        }
        left->changeState(ChunkStates::MESH);
    }

    if (x < CHUNK_WIDTH - 1){ //right
        nextBlockIndex = blockIndex + 1;
        lightVal = chunk->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                chunk->setSunlight(nextBlockIndex, 0);
                chunk->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
            } else {
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
            }
        }

    } else if (right && right->isAccessible){
        nextBlockIndex = blockIndex - CHUNK_WIDTH + 1;
        lightVal = right->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                right->setSunlight(nextBlockIndex, 0);
                right->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
             //   right->lightFromLeft.enqueue(*((ui32*)&LightMessage(blockIndex - CHUNK_WIDTH + 1, type, -lightVal)));
            } else {
                right->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
             //   right->lightFromLeft.enqueue(*((ui32*)&LightMessage(blockIndex - CHUNK_WIDTH + 1, type, 0)));
            }
        }
        right->changeState(ChunkStates::MESH);
    }

    if (z > 0){ //back
        nextBlockIndex = blockIndex - CHUNK_WIDTH;
        lightVal = chunk->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                chunk->setSunlight(nextBlockIndex, 0);
                chunk->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
            } else {
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
            }
        }
    } else if (back && back->isAccessible){
        nextBlockIndex = blockIndex + CHUNK_LAYER - CHUNK_WIDTH;
        lightVal = back->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                back->setSunlight(nextBlockIndex, 0);
                back->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
            //    back->lightFromFront.enqueue(*((ui32*)&LightMessage(blockIndex + CHUNK_LAYER - CHUNK_WIDTH, type, -lightVal)));
            } else {
                back->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
            //    back->lightFromFront.enqueue(*((ui32*)&LightMessage(blockIndex + CHUNK_LAYER - CHUNK_WIDTH, type, 0)));
            }
        }
        back->changeState(ChunkStates::MESH);
    }

    if (z < CHUNK_WIDTH - 1){ //front
        nextBlockIndex = blockIndex + CHUNK_WIDTH;
        lightVal = chunk->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                chunk->setSunlight(nextBlockIndex, 0);
                chunk->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
            } else {
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
            }
        }
    } else if (front && front->isAccessible){
        nextBlockIndex = blockIndex - CHUNK_LAYER + CHUNK_WIDTH;
        lightVal = front->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                front->setSunlight(nextBlockIndex, 0);
                front->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
             //   front->lightFromBack.enqueue(*((ui32*)&LightMessage(blockIndex - CHUNK_LAYER + CHUNK_WIDTH, type, -lightVal)));
            } else {
                front->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
             //   front->lightFromBack.enqueue(*((ui32*)&LightMessage(blockIndex - CHUNK_LAYER + CHUNK_WIDTH, type, 0)));
            }
        }
        front->changeState(ChunkStates::MESH);
    }

    if (y > 0){ //bottom
        nextBlockIndex = blockIndex - CHUNK_LAYER;
        lightVal = chunk->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                chunk->setSunlight(nextBlockIndex, 0);
                chunk->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
            } else {
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
            }
        }
    } else if (bottom && bottom->isAccessible){
        nextBlockIndex = CHUNK_SIZE - CHUNK_LAYER + blockIndex;
        lightVal = bottom->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                bottom->setSunlight(nextBlockIndex, 0);
                bottom->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
            //    bottom->lightFromTop.enqueue(*((ui32*)&LightMessage(CHUNK_SIZE - CHUNK_LAYER + blockIndex, type, -lightVal)));
            } else {
                bottom->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
            //    bottom->lightFromTop.enqueue(*((ui32*)&LightMessage(CHUNK_SIZE - CHUNK_LAYER + blockIndex, type, 0)));
            }
        }
        bottom->changeState(ChunkStates::MESH);
    }

    if (y < CHUNK_WIDTH - 1){ //top
        nextBlockIndex = blockIndex + CHUNK_LAYER;
        lightVal = chunk->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                chunk->setSunlight(nextBlockIndex, 0);
                chunk->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
            } else {
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
            }
        }
    } else if (top && top->isAccessible){
        nextBlockIndex = blockIndex - CHUNK_SIZE + CHUNK_LAYER;
        lightVal = top->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                top->setSunlight(nextBlockIndex, 0);
                top->sunlightRemovalQueue.push_back(SunlightRemovalNode(nextBlockIndex, nextIntensity));
              //  top->lightFromBottom.enqueue(*((ui32*)&LightMessage(blockIndex - CHUNK_SIZE + CHUNK_LAYER, type, -lightVal)));
            } else {
                top->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextBlockIndex, 0));
             //   top->lightFromBottom.enqueue(*((ui32*)&LightMessage(blockIndex - CHUNK_SIZE + CHUNK_LAYER, type, 0)));
            }
        }
        top->changeState(ChunkStates::MESH);
    }
    chunk->changeState(ChunkStates::MESH);

}

void VoxelLightEngine::placeSunlightBFS(Chunk* chunk, int blockIndex, ui8 intensity)
{
    if (intensity > chunk->getSunlight(blockIndex)) {
        //Set the light value
        chunk->setSunlight(blockIndex, intensity);
    } else {
        //If intensity is less that the actual light value, use the actual
        intensity = chunk->getSunlight(blockIndex);
    }

    if (intensity <= 1) return;
    //Reduce by 1 to prevent a bunch of -1
    ui8 newIntensity = (intensity - 1);
    int nextIndex;

    chunk->dirty = 1;

    int x = blockIndex % CHUNK_WIDTH;
    int y = blockIndex / CHUNK_LAYER;
    int z = (blockIndex % CHUNK_LAYER) / CHUNK_WIDTH;

    Chunk* left = chunk->left;
    Chunk* right = chunk->right;
    Chunk* back = chunk->back;
    Chunk* front = chunk->front;
    Chunk* top = chunk->top;
    Chunk* bottom = chunk->bottom;

    if (x > 0){ //left
        nextIndex = blockIndex - 1;
        if (chunk->getSunlight(nextIndex) < newIntensity){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setSunlight(nextIndex, newIntensity);
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
            }
        }
    } else if (left && left->isAccessible){
        nextIndex = blockIndex + CHUNK_WIDTH - 1;
        if (left->getSunlight(nextIndex) < newIntensity){
            if (left->getBlock(nextIndex).allowLight){
                left->setSunlight(nextIndex, newIntensity);
                left->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
               // left->lightFromRight.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            left->changeState(ChunkStates::MESH);
        }
    }

    if (x < CHUNK_WIDTH - 1){ //right
        nextIndex = blockIndex + 1;
        if (chunk->getSunlight(nextIndex) < newIntensity){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setSunlight(nextIndex, newIntensity);
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
            }
        }
    } else if (right && right->isAccessible){
        nextIndex = blockIndex - CHUNK_WIDTH + 1;
        if (right->getSunlight(nextIndex) < newIntensity){
            if (right->getBlock(nextIndex).allowLight){
                right->setSunlight(nextIndex, newIntensity);
                right->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
              //  right->lightFromLeft.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            right->changeState(ChunkStates::MESH);
        }
    }

    if (z > 0){ //back
        nextIndex = blockIndex - CHUNK_WIDTH;
        if (chunk->getSunlight(nextIndex) < newIntensity){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setSunlight(nextIndex, newIntensity);
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
            }
        }
    } else if (back && back->isAccessible){
        nextIndex = blockIndex + CHUNK_LAYER - CHUNK_WIDTH;
        if (back->getSunlight(nextIndex) < newIntensity){
            if (back->getBlock(nextIndex).allowLight){
                back->setSunlight(nextIndex, newIntensity);
                back->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
              //  back->lightFromFront.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            back->changeState(ChunkStates::MESH);
        }
    }

    if (z < CHUNK_WIDTH - 1){ //front
        nextIndex = blockIndex + CHUNK_WIDTH;
        if (chunk->getSunlight(nextIndex) < newIntensity){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setSunlight(nextIndex, newIntensity);
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
            }
        }
    } else if (front && front->isAccessible){
        nextIndex = blockIndex - CHUNK_LAYER + CHUNK_WIDTH;
        if (front->getSunlight(nextIndex) < newIntensity){
            if (front->getBlock(nextIndex).allowLight){
                front->setSunlight(nextIndex, newIntensity);
                front->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
              //  front->lightFromBack.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            front->changeState(ChunkStates::MESH);
        }
    }

    if (y > 0){ //bottom
        nextIndex = blockIndex - CHUNK_LAYER;
        if (chunk->getSunlight(nextIndex) < newIntensity){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setSunlight(nextIndex, newIntensity);
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
            }
        }
    } else if (bottom && bottom->isAccessible){
        nextIndex = CHUNK_SIZE - CHUNK_LAYER + blockIndex;
        if (bottom->getSunlight(nextIndex) < newIntensity){
            if (bottom->getBlock(nextIndex).allowLight){
                bottom->setSunlight(nextIndex, newIntensity);
                bottom->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
              //  bottom->lightFromTop.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            bottom->changeState(ChunkStates::MESH);
        }
    }
    if (y < CHUNK_WIDTH - 1){ //top
        nextIndex = blockIndex + CHUNK_LAYER;
        if (chunk->getSunlight(nextIndex) < newIntensity){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setSunlight(nextIndex, newIntensity);
                chunk->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
            }
        }
    } else if (top && top->isAccessible){
        nextIndex = blockIndex - CHUNK_SIZE + CHUNK_LAYER;
        if (top->getSunlight(nextIndex) < newIntensity){
            if (top->getBlock(nextIndex).allowLight){
                top->setSunlight(nextIndex, newIntensity);
                top->sunlightUpdateQueue.push_back(SunlightUpdateNode(nextIndex, newIntensity));
            //    top->lightFromBottom.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            top->changeState(ChunkStates::MESH);
        }
    }

    chunk->changeState(ChunkStates::MESH);
}

inline ui16 getMaxLampColors(const ui16 redA, const ui16 greenA, const ui16 blueA, const ui16 b) {
    ui16 redB = b & LAMP_RED_MASK;
    ui16 greenB = b & LAMP_GREEN_MASK;
    ui16 blueB = b & LAMP_BLUE_MASK;
    return MAX(redA, redB) | MAX(greenA, greenB) | MAX(blueA, blueB);
}

inline void getLampColors(const ui16 l, ui16 &r, ui16 &g, ui16 &b) {
    r = l & LAMP_RED_MASK;
    g = l & LAMP_GREEN_MASK;
    b = l & LAMP_BLUE_MASK;
}

inline void removeLampNeighborUpdate(Chunk* chunk, int blockIndex, ui16 intensityRed, ui16 intensityGreen, ui16 intensityBlue, ui16 light) {
    ui16 nextRed, nextGreen, nextBlue;
    ui16 nextLight = chunk->getLampLight(blockIndex);
    getLampColors(nextLight, nextRed, nextGreen, nextBlue);

    if ((nextRed && nextRed <= intensityRed) || (nextGreen && nextGreen <= intensityGreen) || (nextBlue && nextBlue <= intensityBlue)){
        chunk->setLampLight(blockIndex, 0);
        chunk->lampLightRemovalQueue.push_back(LampLightRemovalNode(blockIndex, light));

        if (nextRed > intensityRed || nextGreen > intensityGreen || nextBlue > intensityBlue){
            chunk->lampLightUpdateQueue.push_back(LampLightUpdateNode(blockIndex, 0));
        }
    } else if (nextLight > 0) {
        chunk->lampLightUpdateQueue.push_back(LampLightUpdateNode(blockIndex, 0));
    }
}

void VoxelLightEngine::removeLampLightBFS(Chunk* chunk, int blockIndex, ui16 light)
{
#define RED1 0x400
#define GREEN1 0x20
#define BLUE1 1

//#define L_MAX MAX(intensityRed, nextRed) | MAX(intensityGreen, nextGreen) | MAX(intensityBlue, nextBlue)

    ui16 intensityRed = light & LAMP_RED_MASK;
    ui16 intensityGreen = light & LAMP_GREEN_MASK;
    ui16 intensityBlue = light & LAMP_BLUE_MASK;

    //Reduce by 1
    if (intensityRed != 0) {
        intensityRed -= RED1;
        light -= RED1;
    }
    if (intensityGreen != 0) {
        intensityGreen -= GREEN1;
        light -= GREEN1;
    }
    if (intensityBlue != 0) {
        intensityBlue -= BLUE1;
        light -= BLUE1;
    }

    int x = blockIndex % CHUNK_WIDTH;
    int y = blockIndex / CHUNK_LAYER;
    int z = (blockIndex % CHUNK_LAYER) / CHUNK_WIDTH;

    Chunk* left = chunk->left;
    Chunk* right = chunk->right;
    Chunk* back = chunk->back;
    Chunk* front = chunk->front;
    Chunk* top = chunk->top;
    Chunk* bottom = chunk->bottom;

    if (x > 0){ //left
        removeLampNeighborUpdate(chunk, blockIndex - 1, intensityRed, intensityGreen, intensityBlue, light);    
    } else if (left && left->isAccessible){
        removeLampNeighborUpdate(left, blockIndex + CHUNK_WIDTH - 1, intensityRed, intensityGreen, intensityBlue, light);
        left->changeState(ChunkStates::MESH);
    }

    if (x < CHUNK_WIDTH - 1){ //right
        removeLampNeighborUpdate(chunk, blockIndex + 1, intensityRed, intensityGreen, intensityBlue, light);

    } else if (right && right->isAccessible){
        removeLampNeighborUpdate(right, blockIndex - CHUNK_WIDTH + 1, intensityRed, intensityGreen, intensityBlue, light);
        right->changeState(ChunkStates::MESH);
    }

    if (z > 0){ //back
        removeLampNeighborUpdate(chunk, blockIndex - CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
    } else if (back && back->isAccessible){
        removeLampNeighborUpdate(back, blockIndex + CHUNK_LAYER - CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
        back->changeState(ChunkStates::MESH);
    }

    if (z < CHUNK_WIDTH - 1){ //front
        removeLampNeighborUpdate(chunk, blockIndex + CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
    } else if (front && front->isAccessible){
        removeLampNeighborUpdate(front, blockIndex - CHUNK_LAYER + CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
        front->changeState(ChunkStates::MESH);
    }

    if (y > 0){ //bottom
        removeLampNeighborUpdate(chunk, blockIndex - CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue, light);
    } else if (bottom && bottom->isAccessible){
        removeLampNeighborUpdate(bottom, CHUNK_SIZE - CHUNK_LAYER + blockIndex, intensityRed, intensityGreen, intensityBlue, light);
        bottom->changeState(ChunkStates::MESH);
    }

    if (y < CHUNK_WIDTH - 1){ //top
        removeLampNeighborUpdate(chunk, blockIndex + CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue, light);
    } else if (top && top->isAccessible){
        removeLampNeighborUpdate(top, blockIndex - CHUNK_SIZE + CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue, light);
        top->changeState(ChunkStates::MESH);
    }
    chunk->changeState(ChunkStates::MESH);

}

void VoxelLightEngine::placeLampLightBFS(Chunk* chunk, int blockIndex, ui16 intensity)
{
#define RED1 0x400
#define GREEN1 0x20
#define BLUE1 1

    ui16 currentLight = chunk->getLampLight(blockIndex);
    ui16 nextLight;

    ui16 currentRed = currentLight & LAMP_RED_MASK;
    ui16 currentGreen = currentLight & LAMP_GREEN_MASK;
    ui16 currentBlue = currentLight & LAMP_BLUE_MASK;

    ui16 intensityRed = intensity & LAMP_RED_MASK;
    ui16 intensityGreen = intensity & LAMP_GREEN_MASK;
    ui16 intensityBlue = intensity & LAMP_BLUE_MASK;

    //get new intensity
    intensity = MAX(currentRed, intensityRed) | MAX(currentGreen, intensityGreen) | MAX(currentBlue, intensityBlue);

    intensityRed = intensity & LAMP_RED_MASK;
    intensityGreen = intensity & LAMP_GREEN_MASK;
    intensityBlue = intensity & LAMP_BLUE_MASK;

    if (intensity != currentLight) {
        //Set the light value
        chunk->setLampLight(blockIndex, intensity);
    }

    if (currentRed <= RED1 && currentGreen <= GREEN1 && intensityBlue <= BLUE1) return;
    //Reduce by 1
    if (intensityRed != 0) {
        intensityRed -= RED1;
        intensity -= RED1;
    }
    if (intensityGreen != 0) {
        intensityGreen -= GREEN1;
        intensity -= GREEN1;
    }
    if (intensityBlue != 0) {
        intensityBlue -= BLUE1;
        intensity -= BLUE1;
    }

    int nextIndex;

    chunk->dirty = 1;

    int x = blockIndex % CHUNK_WIDTH;
    int y = blockIndex / CHUNK_LAYER;
    int z = (blockIndex % CHUNK_LAYER) / CHUNK_WIDTH;

    Chunk* left = chunk->left;
    Chunk* right = chunk->right;
    Chunk* back = chunk->back;
    Chunk* front = chunk->front;
    Chunk* top = chunk->top;
    Chunk* bottom = chunk->bottom;

    if (x > 0){ //left
        nextIndex = blockIndex - 1;
        currentLight = chunk->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setLampLight(nextIndex, nextLight);
                chunk->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
            }
        }
    } else if (left && left->isAccessible){
        nextIndex = blockIndex + CHUNK_WIDTH - 1;
        currentLight = left->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (left->getBlock(nextIndex).allowLight){
                left->setLampLight(nextIndex, nextLight);
                left->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
                // left->lightFromRight.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            left->changeState(ChunkStates::MESH);
        }
    }

    if (x < CHUNK_WIDTH - 1){ //right
        nextIndex = blockIndex + 1;
        currentLight = chunk->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setLampLight(nextIndex, nextLight);
                chunk->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
            }
        }
    } else if (right && right->isAccessible){
        nextIndex = blockIndex - CHUNK_WIDTH + 1;
        currentLight = right->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (right->getBlock(nextIndex).allowLight){
                right->setLampLight(nextIndex, nextLight);
                right->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
                //  right->lightFromLeft.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            right->changeState(ChunkStates::MESH);
        }
    }

    if (z > 0){ //back
        nextIndex = blockIndex - CHUNK_WIDTH;
        currentLight = chunk->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setLampLight(nextIndex, nextLight);
                chunk->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
            }
        }
    } else if (back && back->isAccessible){
        nextIndex = blockIndex + CHUNK_LAYER - CHUNK_WIDTH;
        currentLight = back->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (back->getBlock(nextIndex).allowLight){
                back->setLampLight(nextIndex, nextLight);
                back->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
                //  back->lightFromFront.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            back->changeState(ChunkStates::MESH);
        }
    }

    if (z < CHUNK_WIDTH - 1){ //front
        nextIndex = blockIndex + CHUNK_WIDTH;
        currentLight = chunk->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setLampLight(nextIndex, nextLight);
                chunk->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
            }
        }
    } else if (front && front->isAccessible){
        nextIndex = blockIndex - CHUNK_LAYER + CHUNK_WIDTH;
        currentLight = front->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (front->getBlock(nextIndex).allowLight){
                front->setLampLight(nextIndex, nextLight);
                front->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
                //  front->lightFromBack.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            front->changeState(ChunkStates::MESH);
        }
    }

    if (y > 0){ //bottom
        nextIndex = blockIndex - CHUNK_LAYER;
        currentLight = chunk->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setLampLight(nextIndex, nextLight);
                chunk->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
            }
        }
    } else if (bottom && bottom->isAccessible){
        nextIndex = CHUNK_SIZE - CHUNK_LAYER + blockIndex;
        currentLight = bottom->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (bottom->getBlock(nextIndex).allowLight){
                bottom->setLampLight(nextIndex, nextLight);
                bottom->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
                //  bottom->lightFromTop.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            bottom->changeState(ChunkStates::MESH);
        }
    }
    if (y < CHUNK_WIDTH - 1){ //top
        nextIndex = blockIndex + CHUNK_LAYER;
        currentLight = chunk->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (chunk->getBlock(nextIndex).allowLight){
                chunk->setLampLight(nextIndex, nextLight);
                chunk->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
            }
        }
    } else if (top && top->isAccessible){
        nextIndex = blockIndex - CHUNK_SIZE + CHUNK_LAYER;
        currentLight = top->getLampLight(nextIndex);
        nextLight = getMaxLampColors(intensityRed, intensityGreen, intensityBlue, currentLight);
        if (nextLight != currentLight){
            if (top->getBlock(nextIndex).allowLight){
                top->setLampLight(nextIndex, nextLight);
                top->lampLightUpdateQueue.push_back(LampLightUpdateNode(nextIndex, nextLight));
                //    top->lightFromBottom.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            top->changeState(ChunkStates::MESH);
        }
    }

    chunk->changeState(ChunkStates::MESH);
}
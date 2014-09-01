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

    if (chunk->lightRemovalQueue.size()) {
        //Removal
        for (ui32 i = 0; i < chunk->lightRemovalQueue.size(); i++){
            removeSunlightBFS(chunk, (int)chunk->lightRemovalQueue[i].c, chunk->lightRemovalQueue[i].oldLightVal);
        }
        vector<LightRemovalNode>().swap(chunk->lightRemovalQueue); //forces memory to be freed
    }

    if (chunk->lightUpdateQueue.size()) {
        //Addition
        for (ui32 i = 0; i < chunk->lightUpdateQueue.size(); i++){
            placeSunlight(chunk, (int)chunk->lightUpdateQueue[i].c, (int)chunk->lightUpdateQueue[i].lightVal);
        }
        vector<LightUpdateNode>().swap(chunk->lightUpdateQueue); //forces memory to be freed
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
                    chunk->lightUpdateQueue.push_back(LightUpdateNode(blockIndex, 1, MAXLIGHT));
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
                chunk->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
            } else {
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
            }
        }
    } else if (left && left->isAccessible){
        nextBlockIndex = blockIndex + CHUNK_WIDTH - 1;
        lightVal = left->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                left->setSunlight(nextBlockIndex, 0);
                left->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
             //   left->lightFromRight.enqueue(*((ui32*)&LightMessage(blockIndex + CHUNK_WIDTH - 1, type, -lightVal)));
            } else {
                left->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
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
                chunk->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
            } else {
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
            }
        }

    } else if (right && right->isAccessible){
        nextBlockIndex = blockIndex - CHUNK_WIDTH + 1;
        lightVal = right->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                right->setSunlight(nextBlockIndex, 0);
                right->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
             //   right->lightFromLeft.enqueue(*((ui32*)&LightMessage(blockIndex - CHUNK_WIDTH + 1, type, -lightVal)));
            } else {
                right->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
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
                chunk->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
            } else {
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
            }
        }
    } else if (back && back->isAccessible){
        nextBlockIndex = blockIndex + CHUNK_LAYER - CHUNK_WIDTH;
        lightVal = back->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                back->setSunlight(nextBlockIndex, 0);
                back->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
            //    back->lightFromFront.enqueue(*((ui32*)&LightMessage(blockIndex + CHUNK_LAYER - CHUNK_WIDTH, type, -lightVal)));
            } else {
                back->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
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
                chunk->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
            } else {
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
            }
        }
    } else if (front && front->isAccessible){
        nextBlockIndex = blockIndex - CHUNK_LAYER + CHUNK_WIDTH;
        lightVal = front->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                front->setSunlight(nextBlockIndex, 0);
                front->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
             //   front->lightFromBack.enqueue(*((ui32*)&LightMessage(blockIndex - CHUNK_LAYER + CHUNK_WIDTH, type, -lightVal)));
            } else {
                front->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
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
                chunk->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
            } else {
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
            }
        }
    } else if (bottom && bottom->isAccessible){
        nextBlockIndex = CHUNK_SIZE - CHUNK_LAYER + blockIndex;
        lightVal = bottom->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                bottom->setSunlight(nextBlockIndex, 0);
                bottom->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
            //    bottom->lightFromTop.enqueue(*((ui32*)&LightMessage(CHUNK_SIZE - CHUNK_LAYER + blockIndex, type, -lightVal)));
            } else {
                bottom->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
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
                chunk->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
            } else {
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
            }
        }
    } else if (top && top->isAccessible){
        nextBlockIndex = blockIndex - CHUNK_SIZE + CHUNK_LAYER;
        lightVal = top->getSunlight(nextBlockIndex);
        if (lightVal > 0){
            if (lightVal <= nextIntensity){
                top->setSunlight(nextBlockIndex, 0);
                top->lightRemovalQueue.push_back(LightRemovalNode(nextBlockIndex, 1, nextIntensity));
              //  top->lightFromBottom.enqueue(*((ui32*)&LightMessage(blockIndex - CHUNK_SIZE + CHUNK_LAYER, type, -lightVal)));
            } else {
                top->lightUpdateQueue.push_back(LightUpdateNode(nextBlockIndex, 1, 0));
             //   top->lightFromBottom.enqueue(*((ui32*)&LightMessage(blockIndex - CHUNK_SIZE + CHUNK_LAYER, type, 0)));
            }
        }
        top->changeState(ChunkStates::MESH);
    }
    chunk->changeState(ChunkStates::MESH);

}

void VoxelLightEngine::placeSunlight(Chunk* chunk, int blockIndex, int intensity)
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
    ui8 newIntensity = (ui8)(intensity - 1);
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
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
            }
        }
    } else if (left && left->isAccessible){
        nextIndex = blockIndex + CHUNK_WIDTH - 1;
        if (left->getSunlight(nextIndex) < newIntensity){
            if (left->getBlock(nextIndex).allowLight){
                left->setSunlight(nextIndex, newIntensity);
                left->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
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
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
            }
        }
    } else if (right && right->isAccessible){
        nextIndex = blockIndex - CHUNK_WIDTH + 1;
        if (right->getSunlight(nextIndex) < newIntensity){
            if (right->getBlock(nextIndex).allowLight){
                right->setSunlight(nextIndex, newIntensity);
                right->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
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
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
            }
        }
    } else if (back && back->isAccessible){
        nextIndex = blockIndex + CHUNK_LAYER - CHUNK_WIDTH;
        if (back->getSunlight(nextIndex) < newIntensity){
            if (back->getBlock(nextIndex).allowLight){
                back->setSunlight(nextIndex, newIntensity);
                back->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
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
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
            }
        }
    } else if (front && front->isAccessible){
        nextIndex = blockIndex - CHUNK_LAYER + CHUNK_WIDTH;
        if (front->getSunlight(nextIndex) < newIntensity){
            if (front->getBlock(nextIndex).allowLight){
                front->setSunlight(nextIndex, newIntensity);
                front->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
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
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
            }
        }
    } else if (bottom && bottom->isAccessible){
        nextIndex = CHUNK_SIZE - CHUNK_LAYER + blockIndex;
        if (bottom->getSunlight(nextIndex) < newIntensity){
            if (bottom->getBlock(nextIndex).allowLight){
                bottom->setSunlight(nextIndex, newIntensity);
                bottom->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
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
                chunk->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
            }
        }
    } else if (top && top->isAccessible){
        nextIndex = blockIndex - CHUNK_SIZE + CHUNK_LAYER;
        if (top->getSunlight(nextIndex) < newIntensity){
            if (top->getBlock(nextIndex).allowLight){
                top->setSunlight(nextIndex, newIntensity);
                top->lightUpdateQueue.push_back(LightUpdateNode(nextIndex, 1, newIntensity));
            //    top->lightFromBottom.enqueue(*((ui32*)&LightMessage(nextIndex, type, newIntensity)));
            }
            top->changeState(ChunkStates::MESH);
        }
    }

    chunk->changeState(ChunkStates::MESH);
}
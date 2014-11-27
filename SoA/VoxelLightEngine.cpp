#include "stdafx.h"

#include "VoxelLightEngine.h"

#include "Chunk.h"
#include "Errors.h"

void VoxelLightEngine::calculateLight(Chunk* chunk)
{
    //Flush all edge queues
    _lockedChunk = nullptr;

    //Sunlight Calculation
    if (chunk->sunlightRemovalQueue.size()) {
        lockChunk(chunk);
        //Removal
        while (chunk->sunlightRemovalQueue.size()){
            auto& node = chunk->sunlightRemovalQueue.front();
            removeSunlightBFS(chunk, (int)node.blockIndex, node.oldLightVal);
            chunk->sunlightRemovalQueue.pop();
        }
        std::queue<SunlightRemovalNode>().swap(chunk->sunlightRemovalQueue); //forces memory to be freed
    }

    if (chunk->sunlightUpdateQueue.size()) {
        lockChunk(chunk);
        //Addition
        while (chunk->sunlightUpdateQueue.size()){
            auto& node = chunk->sunlightUpdateQueue.front();
            placeSunlightBFS(chunk, (int)node.blockIndex, (int)node.lightVal);
            chunk->sunlightUpdateQueue.pop();
        }
        std::queue<SunlightUpdateNode>().swap(chunk->sunlightUpdateQueue); //forces memory to be freed
    }

    //Voxel Light Calculation
    if (chunk->lampLightRemovalQueue.size()) {
        lockChunk(chunk);
        //Removal
        while (chunk->lampLightRemovalQueue.size()){
            auto& node = chunk->lampLightRemovalQueue.front();
            removeLampLightBFS(chunk, (int)node.blockIndex, node.oldLightColor);
            chunk->lampLightRemovalQueue.pop();
        }
        std::queue<LampLightRemovalNode>().swap(chunk->lampLightRemovalQueue); //forces memory to be freed
    }

    if (chunk->lampLightUpdateQueue.size()) {
        lockChunk(chunk);
        //Addition
        while (chunk->lampLightUpdateQueue.size()) {
            auto& node = chunk->lampLightUpdateQueue.front();
            placeLampLightBFS(chunk, (int)node.blockIndex, node.lightColor);
            chunk->lampLightUpdateQueue.pop();
        }
        std::queue<LampLightUpdateNode>().swap(chunk->lampLightUpdateQueue); //forces memory to be freed
    }
    if (_lockedChunk) {
        _lockedChunk->unlock();
        _lockedChunk = nullptr;
    }
}

void VoxelLightEngine::calculateSunlightExtend(Chunk* chunk)
{
    int blockIndex;
    int y;

    _lockedChunk = nullptr;
    lockChunk(chunk);

    for (ui32 i = 0; i < chunk->sunExtendList.size(); i++){
        blockIndex = chunk->sunExtendList[i];
        if (chunk->getSunlight(blockIndex) == MAXLIGHT){
            y = blockIndex / CHUNK_LAYER;
            extendSunRay(chunk, blockIndex - y * CHUNK_LAYER, y);
        }
    }
    vector<GLushort>().swap(chunk->sunExtendList); //forces memory to be freed
    if (_lockedChunk) {
        _lockedChunk->unlock();
        _lockedChunk = nullptr;
    }
}

void VoxelLightEngine::calculateSunlightRemoval(Chunk* chunk)
{
    int blockIndex;
    int y;

    _lockedChunk = nullptr;
    lockChunk(chunk);

    for (ui32 i = 0; i < chunk->sunRemovalList.size(); i++){
        blockIndex = chunk->sunRemovalList[i];
        if (chunk->getSunlight(blockIndex) == 0){
            y = blockIndex / CHUNK_LAYER;
            blockSunRay(chunk, blockIndex - y * CHUNK_LAYER, y - 1);
        }
    }
    vector<GLushort>().swap(chunk->sunRemovalList); //forces memory to be freed
    if (_lockedChunk) {
        _lockedChunk->unlock();
        _lockedChunk = nullptr;
    }
}

//Check for sun rays from the top chunk
void VoxelLightEngine::checkTopForSunlight(Chunk* chunk)
{
    int topLight, blockIndex;
    if (chunk->top && chunk->top->isAccessible){
        lockChunk(chunk->top);
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
    if (_lockedChunk) {
        _lockedChunk->unlock();
        _lockedChunk = nullptr;
    }
}

void VoxelLightEngine::blockSunRay(Chunk* chunk, int xz, int y)
{
    int i = y; //start at the current block
    chunk->changeState(ChunkStates::MESH);
   
    while (true){ //do the light removal iteration
        if (i == -1){ //bottom chunk
            if (chunk->bottom && chunk->bottom->isAccessible){
                lockChunk(chunk->bottom);
                VoxelLightEngine::blockSunRay(chunk->bottom, xz, 31); //continue the algorithm
                lockChunk(chunk);
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
                lockChunk(chunk->bottom);
                extendSunRay(chunk->bottom, xz, 31); //continue the algorithm
                lockChunk(chunk);
            }
            return;
        } else{
            blockIndex = xz + i*CHUNK_LAYER;
            if (chunk->getBlock(blockIndex).blockLight == 0){

                if (chunk->getSunlight(blockIndex) != MAXLIGHT){
                    chunk->sunlightUpdateQueue.emplace(blockIndex, MAXLIGHT);
                }
            
            } else{
                return;
            }
        }
        i--;
    }
}

inline void removeSunlightNeighborUpdate(Chunk* chunk, int blockIndex, ui16 light) {
    ui8 lightVal = chunk->getSunlight(blockIndex);
    if (lightVal > 0){
        if (lightVal <= light){
            chunk->setSunlight(blockIndex, 0);
            chunk->sunlightRemovalQueue.emplace(blockIndex, light);
        } else {
            chunk->sunlightUpdateQueue.emplace(blockIndex, 0);
        }
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

    Chunk*& left = chunk->left;
    Chunk*& right = chunk->right;
    Chunk*& back = chunk->back;
    Chunk*& front = chunk->front;
    Chunk*& top = chunk->top;
    Chunk*& bottom = chunk->bottom;

    if (x > 0){ //left
        removeSunlightNeighborUpdate(chunk, blockIndex - 1, nextIntensity);
    } else if (left && left->isAccessible){
        lockChunk(left);
        removeSunlightNeighborUpdate(left, blockIndex + CHUNK_WIDTH - 1, nextIntensity);
        left->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (x < CHUNK_WIDTH - 1){ //right
        removeSunlightNeighborUpdate(chunk, blockIndex + 1, nextIntensity);
    } else if (right && right->isAccessible){
        lockChunk(right);
        removeSunlightNeighborUpdate(right, blockIndex - CHUNK_WIDTH + 1, nextIntensity);
        right->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (z > 0){ //back
        removeSunlightNeighborUpdate(chunk, blockIndex - CHUNK_WIDTH, nextIntensity);
    } else if (back && back->isAccessible){
        lockChunk(back);
        removeSunlightNeighborUpdate(back, blockIndex + CHUNK_LAYER - CHUNK_WIDTH, nextIntensity);
        back->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (z < CHUNK_WIDTH - 1){ //front
        removeSunlightNeighborUpdate(chunk, blockIndex + CHUNK_WIDTH, nextIntensity);
    } else if (front && front->isAccessible){
        lockChunk(front);
        removeSunlightNeighborUpdate(front, blockIndex - CHUNK_LAYER + CHUNK_WIDTH, nextIntensity);
        front->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (y > 0){ //bottom
        removeSunlightNeighborUpdate(chunk, blockIndex - CHUNK_LAYER, nextIntensity);
    } else if (bottom && bottom->isAccessible){
        lockChunk(bottom);
        removeSunlightNeighborUpdate(bottom, CHUNK_SIZE - CHUNK_LAYER + blockIndex, nextIntensity);
        bottom->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (y < CHUNK_WIDTH - 1){ //top
        removeSunlightNeighborUpdate(chunk, blockIndex + CHUNK_LAYER, nextIntensity);
    } else if (top && top->isAccessible){
        lockChunk(top);
        removeSunlightNeighborUpdate(top, blockIndex - CHUNK_SIZE + CHUNK_LAYER, nextIntensity);
        top->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }
    chunk->changeState(ChunkStates::MESH);

}

inline void placeSunlightNeighborUpdate(Chunk* chunk, int blockIndex, ui16 light) {
    if (chunk->getSunlight(blockIndex) < light){
        if (chunk->getBlock(blockIndex).allowLight){
            chunk->setSunlight(blockIndex, light);
            chunk->sunlightUpdateQueue.emplace(blockIndex, light);
        }
    }
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

    chunk->dirty = 1;

    int x = blockIndex % CHUNK_WIDTH;
    int y = blockIndex / CHUNK_LAYER;
    int z = (blockIndex % CHUNK_LAYER) / CHUNK_WIDTH;

    Chunk*& left = chunk->left;
    Chunk*& right = chunk->right;
    Chunk*& back = chunk->back;
    Chunk*& front = chunk->front;
    Chunk*& top = chunk->top;
    Chunk*& bottom = chunk->bottom;

    if (x > 0){ //left
        placeSunlightNeighborUpdate(chunk, blockIndex - 1, newIntensity);
    } else if (left && left->isAccessible){
        lockChunk(left);
        placeSunlightNeighborUpdate(left, blockIndex + CHUNK_WIDTH - 1, newIntensity);
        left->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (x < CHUNK_WIDTH - 1){ //right
        placeSunlightNeighborUpdate(chunk, blockIndex + 1, newIntensity);
    } else if (right && right->isAccessible){
        lockChunk(right);
        placeSunlightNeighborUpdate(right, blockIndex - CHUNK_WIDTH + 1, newIntensity);
        right->changeState(ChunkStates::MESH); 
        lockChunk(chunk);
    }

    if (z > 0){ //back
        placeSunlightNeighborUpdate(chunk, blockIndex - CHUNK_WIDTH, newIntensity);
    } else if (back && back->isAccessible){
        lockChunk(back);
        placeSunlightNeighborUpdate(back, blockIndex + CHUNK_LAYER - CHUNK_WIDTH, newIntensity);
        back->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (z < CHUNK_WIDTH - 1){ //front
        placeSunlightNeighborUpdate(chunk, blockIndex + CHUNK_WIDTH, newIntensity);
    } else if (front && front->isAccessible){
        lockChunk(front);
        placeSunlightNeighborUpdate(front, blockIndex - CHUNK_LAYER + CHUNK_WIDTH, newIntensity);
        front->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (y > 0){ //bottom
        placeSunlightNeighborUpdate(chunk, blockIndex - CHUNK_LAYER, newIntensity);
    } else if (bottom && bottom->isAccessible){
        lockChunk(bottom);
        placeSunlightNeighborUpdate(bottom, CHUNK_SIZE - CHUNK_LAYER + blockIndex, newIntensity);
        bottom->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (y < CHUNK_WIDTH - 1){ //top
        placeSunlightNeighborUpdate(chunk, blockIndex + CHUNK_LAYER, newIntensity);
    } else if (top && top->isAccessible){
        lockChunk(top);
        placeSunlightNeighborUpdate(top, blockIndex - CHUNK_SIZE + CHUNK_LAYER, newIntensity);
        top->changeState(ChunkStates::MESH);
        lockChunk(chunk);
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
        chunk->lampLightRemovalQueue.emplace(blockIndex, light);

        if (nextRed > intensityRed || nextGreen > intensityGreen || nextBlue > intensityBlue){
            chunk->lampLightUpdateQueue.emplace(blockIndex, 0);
        }
    } else if (nextLight > 0) {
        chunk->lampLightUpdateQueue.emplace(blockIndex, 0);
    }
}

void VoxelLightEngine::removeLampLightBFS(Chunk* chunk, int blockIndex, ui16 light)
{
#define RED1 0x400
#define GREEN1 0x20
#define BLUE1 1

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

    Chunk*& left = chunk->left;
    Chunk*& right = chunk->right;
    Chunk*& back = chunk->back;
    Chunk*& front = chunk->front;
    Chunk*& top = chunk->top;
    Chunk*& bottom = chunk->bottom;

    if (x > 0){ //left
        removeLampNeighborUpdate(chunk, blockIndex - 1, intensityRed, intensityGreen, intensityBlue, light);    
    } else if (left && left->isAccessible){
        lockChunk(left);
        removeLampNeighborUpdate(left, blockIndex + CHUNK_WIDTH - 1, intensityRed, intensityGreen, intensityBlue, light);
        left->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (x < CHUNK_WIDTH - 1){ //right
        removeLampNeighborUpdate(chunk, blockIndex + 1, intensityRed, intensityGreen, intensityBlue, light);
    } else if (right && right->isAccessible){
        lockChunk(right);
        removeLampNeighborUpdate(right, blockIndex - CHUNK_WIDTH + 1, intensityRed, intensityGreen, intensityBlue, light);
        right->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (z > 0){ //back
        removeLampNeighborUpdate(chunk, blockIndex - CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
    } else if (back && back->isAccessible){
        lockChunk(back);
        removeLampNeighborUpdate(back, blockIndex + CHUNK_LAYER - CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
        back->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (z < CHUNK_WIDTH - 1){ //front
        removeLampNeighborUpdate(chunk, blockIndex + CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
    } else if (front && front->isAccessible){
        lockChunk(front);
        removeLampNeighborUpdate(front, blockIndex - CHUNK_LAYER + CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue, light);
        front->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (y > 0){ //bottom
        removeLampNeighborUpdate(chunk, blockIndex - CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue, light);
    } else if (bottom && bottom->isAccessible){
        lockChunk(bottom);
        removeLampNeighborUpdate(bottom, CHUNK_SIZE - CHUNK_LAYER + blockIndex, intensityRed, intensityGreen, intensityBlue, light);
        bottom->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (y < CHUNK_WIDTH - 1){ //top
        removeLampNeighborUpdate(chunk, blockIndex + CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue, light);
    } else if (top && top->isAccessible){
        lockChunk(top);
        removeLampNeighborUpdate(top, blockIndex - CHUNK_SIZE + CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue, light);
        top->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }
    chunk->changeState(ChunkStates::MESH);

}

inline void placeLampNeighborUpdate(Chunk* chunk, int blockIndex, ui16 intensityRed, ui16 intensityGreen, ui16 intensityBlue) {
    ui16 currentLight = chunk->getLampLight(blockIndex);
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
    }
}

void VoxelLightEngine::placeLampLightBFS(Chunk* chunk, int blockIndex, ui16 intensity)
{
#define RED1 0x400
#define GREEN1 0x20
#define BLUE1 1

    ui16 currentLight = chunk->getLampLight(blockIndex);

    ui16 currentRed = currentLight & LAMP_RED_MASK;
    ui16 currentGreen = currentLight & LAMP_GREEN_MASK;
    ui16 currentBlue = currentLight & LAMP_BLUE_MASK;

    ui16 intensityRed = intensity & LAMP_RED_MASK;
    ui16 intensityGreen = intensity & LAMP_GREEN_MASK;
    ui16 intensityBlue = intensity & LAMP_BLUE_MASK;

    intensityRed = MAX(currentRed, intensityRed);
    intensityGreen = MAX(currentGreen, intensityGreen);
    intensityBlue = MAX(currentBlue, intensityBlue);

    const Block& currentBlock = chunk->getBlock(blockIndex);

    intensityRed = (ui16)((intensityRed >> LAMP_RED_SHIFT) * currentBlock.colorFilter.r) << LAMP_RED_SHIFT;
    intensityGreen = (ui16)((intensityGreen >> LAMP_GREEN_SHIFT) * currentBlock.colorFilter.g) << LAMP_GREEN_SHIFT;
    intensityBlue = (ui16)(intensityBlue * currentBlock.colorFilter.b);
    intensity = intensityRed | intensityGreen | intensityBlue;

    if (intensity != currentLight) { 
        //Set the light value
        chunk->setLampLight(blockIndex, intensity);
    }

    if (intensityRed <= RED1 && intensityGreen <= GREEN1 && intensityBlue <= BLUE1) return;
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

    chunk->dirty = 1;

    int x = blockIndex % CHUNK_WIDTH;
    int y = blockIndex / CHUNK_LAYER;
    int z = (blockIndex % CHUNK_LAYER) / CHUNK_WIDTH;

    Chunk*& left = chunk->left;
    Chunk*& right = chunk->right;
    Chunk*& back = chunk->back;
    Chunk*& front = chunk->front;
    Chunk*& top = chunk->top;
    Chunk*& bottom = chunk->bottom;

    if (x > 0){ //left
        placeLampNeighborUpdate(chunk, blockIndex - 1, intensityRed, intensityGreen, intensityBlue);
    } else if (left && left->isAccessible){
        lockChunk(left);
        placeLampNeighborUpdate(left, blockIndex + CHUNK_WIDTH - 1, intensityRed, intensityGreen, intensityBlue);
        left->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (x < CHUNK_WIDTH - 1){ //right
        placeLampNeighborUpdate(chunk, blockIndex + 1, intensityRed, intensityGreen, intensityBlue);
    } else if (right && right->isAccessible){
        lockChunk(right);
        placeLampNeighborUpdate(right, blockIndex - CHUNK_WIDTH + 1, intensityRed, intensityGreen, intensityBlue);
        right->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (z > 0){ //back
        placeLampNeighborUpdate(chunk, blockIndex - CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue);
    } else if (back && back->isAccessible){
        lockChunk(back);
        placeLampNeighborUpdate(back, blockIndex + CHUNK_LAYER - CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue);
        back->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (z < CHUNK_WIDTH - 1){ //front
        placeLampNeighborUpdate(chunk, blockIndex + CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue);
    } else if (front && front->isAccessible){
        lockChunk(front);
        placeLampNeighborUpdate(front, blockIndex - CHUNK_LAYER + CHUNK_WIDTH, intensityRed, intensityGreen, intensityBlue);
        front->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    if (y > 0){ //bottom
        placeLampNeighborUpdate(chunk, blockIndex - CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue);
    } else if (bottom && bottom->isAccessible){
        lockChunk(bottom);
        placeLampNeighborUpdate(bottom, CHUNK_SIZE - CHUNK_LAYER + blockIndex, intensityRed, intensityGreen, intensityBlue);
        bottom->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }
    if (y < CHUNK_WIDTH - 1){ //top
        placeLampNeighborUpdate(chunk, blockIndex + CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue);
    } else if (top && top->isAccessible){
        lockChunk(top);
        placeLampNeighborUpdate(top, blockIndex - CHUNK_SIZE + CHUNK_LAYER, intensityRed, intensityGreen, intensityBlue);
        top->changeState(ChunkStates::MESH);
        lockChunk(chunk);
    }

    chunk->changeState(ChunkStates::MESH);
}

void VoxelLightEngine::lockChunk(Chunk* chunk) {
    if (_lockedChunk) _lockedChunk->unlock();
    _lockedChunk = chunk;
    chunk->lock();
}
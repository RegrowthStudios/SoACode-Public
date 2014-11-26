#pragma once
#include "Chunk.h"
#include "global.h"
#include "BlockData.h"


inline void Chunk::changeState(ChunkStates State)
{
    //Only set the state if the new state is higher priority
    if (_state > State){
        _state = State;

        //isAccessible is a flag that prevents threads from trying to
        //acces the chunk when its loading or generating
        if (_state > ChunkStates::GENERATE){
            isAccessible = true;
        } else{
            isAccessible = false;
        }
    }
}

inline int Chunk::getLeftBlockData(int blockIndex)
{
    if (getXFromBlockIndex(blockIndex) > 0){
        return getBlockData(blockIndex - 1);
    } else if (left && left->isAccessible){
        unlock();
        left->lock();
        return left->getBlockData(blockIndex + CHUNK_WIDTH - 1);
    }
    return -1;
}

inline int Chunk::getLeftBlockData(int blockIndex, int x, int& nextBlockIndex, Chunk*& owner)
{
    if (x > 0){
        owner = this;
        nextBlockIndex = blockIndex - 1;
        return getBlockData(nextBlockIndex);
    } else if (left && left->isAccessible){
        owner = left;
        nextBlockIndex = blockIndex + CHUNK_WIDTH - 1;
        unlock();
        left->lock();
        return left->getBlockData(nextBlockIndex);
    }
    return -1;
}

inline int Chunk::getRightBlockData(int blockIndex)
{
    if (getXFromBlockIndex(blockIndex) < CHUNK_WIDTH - 1) {
        return getBlockData(blockIndex + 1);
    } else if (right && right->isAccessible){
        unlock();
        right->lock();
        return right->getBlockData(blockIndex - CHUNK_WIDTH + 1);
    }
    return -1;
}

inline int Chunk::getRightBlockData(int blockIndex, int x, int& nextBlockIndex, Chunk*& owner)
{
    if (x < CHUNK_WIDTH - 1){
        owner = this;
        nextBlockIndex = blockIndex + 1;
        return getBlockData(nextBlockIndex);
    } else if (right && right->isAccessible){
        owner = right;
        nextBlockIndex =blockIndex - CHUNK_WIDTH + 1;
        unlock();
        right->lock();
        return right->getBlockData(nextBlockIndex);
    }
    return -1;
}

inline int Chunk::getFrontBlockData(int blockIndex)
{
    if (getZFromBlockIndex(blockIndex) < CHUNK_WIDTH - 1) {
        return getBlockData(blockIndex + CHUNK_WIDTH);
    } else if (front && front->isAccessible){
        unlock();
        front->lock();
        ui16 blockData = front->getBlockData(blockIndex - CHUNK_LAYER + CHUNK_WIDTH);
        front->unlock();
        lock();
        return blockData;
    }
    return -1;
}

inline int Chunk::getFrontBlockData(int blockIndex, int z, int& nextBlockIndex, Chunk*& owner)
{
    if (z < CHUNK_WIDTH - 1){
        owner = this;
        nextBlockIndex = blockIndex + CHUNK_WIDTH;
        return getBlockData(nextBlockIndex);
    } else if (front && front->isAccessible){
        owner = front;
        nextBlockIndex = blockIndex - CHUNK_LAYER + CHUNK_WIDTH;
        unlock();
        front->lock();
        return front->getBlockData(nextBlockIndex);
    }
    nextBlockIndex = NULL;
    return 33;
}

inline int Chunk::getBackBlockData(int blockIndex)
{
    if (getZFromBlockIndex(blockIndex) > 0) {
        return getBlockData(blockIndex - CHUNK_WIDTH);
    } else if (back && back->isAccessible){
        unlock();
        back->lock();
        return back->getBlockData(blockIndex + CHUNK_LAYER - CHUNK_WIDTH);
    }
    return -1;
}

inline int Chunk::getBackBlockData(int blockIndex, int z, int& nextBlockIndex, Chunk*& owner)
{
    if (z > 0){
        owner = this;
        nextBlockIndex = blockIndex - CHUNK_WIDTH;
        return getBlockData(nextBlockIndex);
    } else if (back && back->isAccessible){
        owner = back;
        nextBlockIndex = blockIndex + CHUNK_LAYER - CHUNK_WIDTH;
        unlock();
        back->lock();
        return back->getBlockData(nextBlockIndex);
    }

    return -1;
}

inline int Chunk::getBottomBlockData(int blockIndex)
{
    if (getYFromBlockIndex(blockIndex) > 0) {
        return getBlockData(blockIndex - CHUNK_LAYER);
    } else if (bottom && bottom->isAccessible){
        unlock();
        bottom->lock();
        return bottom->getBlockData(blockIndex + CHUNK_SIZE - CHUNK_LAYER);
    }
    return -1;
}

inline int Chunk::getBottomBlockData(int blockIndex, int y, int& nextBlockIndex, Chunk*& owner)
{
    if (y > 0){
        owner = this;
        nextBlockIndex = blockIndex - CHUNK_LAYER;
        return getBlockData(nextBlockIndex);
    } else if (bottom && bottom->isAccessible){
        owner = bottom;
        nextBlockIndex = blockIndex + CHUNK_SIZE - CHUNK_LAYER;
        unlock();
        bottom->lock();
        return bottom->getBlockData(nextBlockIndex);
    }
    return -1;
}

inline int Chunk::getBottomBlockData(int blockIndex, int y) {
    if (y > 0) {
        return getBlockData(blockIndex - CHUNK_LAYER);
    } else if (bottom && bottom->isAccessible) {
        unlock();
        bottom->lock();
        return bottom->getBlockData(blockIndex + CHUNK_SIZE - CHUNK_LAYER);
        bottom->unlock();
        lock();
    }
    return -1;
}

inline int Chunk::getTopBlockData(int blockIndex)
{
    if (getYFromBlockIndex(blockIndex) < CHUNK_WIDTH - 1) {
        return getBlockData(blockIndex + CHUNK_LAYER);
    } else if (top && top->isAccessible){
        unlock();
        top->lock();
        ui16 blockData = top->getBlockData(blockIndex - CHUNK_SIZE + CHUNK_LAYER);
        return blockData;
    }
    return -1;
}

inline int Chunk::getTopBlockData(int blockIndex, int& nextBlockIndex, Chunk*& owner)
{
    if (getYFromBlockIndex(blockIndex) < CHUNK_WIDTH - 1) {
        owner = this;
        nextBlockIndex = blockIndex + CHUNK_LAYER;
        return getBlockData(nextBlockIndex);
    } else if (top && top->isAccessible){
        owner = top;
        nextBlockIndex = blockIndex - CHUNK_SIZE + CHUNK_LAYER;
        unlock();
        top->lock();
        return top->getBlockData(nextBlockIndex);
    }
    return -1;
}

inline int Chunk::getTopBlockData(int blockIndex, int y, int& nextBlockIndex, Chunk*& owner)
{
    if (y < CHUNK_WIDTH - 1){
        owner = this;
        nextBlockIndex = blockIndex + CHUNK_LAYER;
        return getBlockData(nextBlockIndex);
    } else if (top && top->isAccessible){
        owner = top;
        nextBlockIndex = blockIndex - CHUNK_SIZE + CHUNK_LAYER;
        unlock();
        top->lock();
        return top->getBlockData(nextBlockIndex);
    }
    return -1;
}

inline int Chunk::getTopSunlight(int c) {
    if (getYFromBlockIndex(c) < CHUNK_WIDTH - 1) {
        return getSunlight(c + CHUNK_LAYER);
    } else if (top && top->isAccessible) {
        return top->getSunlight(c - CHUNK_SIZE + CHUNK_LAYER);
    }
    return 0;
}

inline int Chunk::getSunlight(int c) const {
    return _sunlightContainer.get(c);
}

inline ui16 Chunk::getTertiaryData(int c) const {
    return _tertiaryDataContainer.get(c);
}

inline int Chunk::getFloraHeight(int c) const {
    return _tertiaryDataContainer.get(c) & FLORA_HEIGHT_MASK;
}

inline ui16 Chunk::getLampLight(int c) const {
    return _lampLightContainer.get(c);
}

inline ui16 Chunk::getLampRed(int c) const {
    return _lampLightContainer.get(c) & LAMP_RED_MASK;
}

inline ui16 Chunk::getLampGreen(int c) const {
    return _lampLightContainer.get(c) & LAMP_GREEN_MASK;
}

inline ui16 Chunk::getLampBlue(int c) const {
    return _lampLightContainer.get(c) & LAMP_BLUE_MASK;
}

inline void Chunk::setSunlight(int c, ui8 val) {
    _sunlightContainer.set(c, val);
}

inline void Chunk::setLampLight(int c, ui16 val) {
    _lampLightContainer.set(c, val);
}

// TODO(Ben): .setWithMask to avoid an extra traversal
inline void Chunk::setFloraHeight(int c, ui16 val) {
    _tertiaryDataContainer.set(c, (_tertiaryDataContainer.get(c) & (~FLORA_HEIGHT_MASK)) | val);
}

inline void Chunk::setBlockID(int c, int val) {
    _blockIDContainer.set(c, val);
}

inline void Chunk::setBlockData(int c, ui16 val) {
    _blockIDContainer.set(c, val);
}

inline void Chunk::setTertiaryData(int c, ui16 val) {
    _tertiaryDataContainer.set(c, val);
}

inline GLushort Chunk::getBlockData(int c) const {
    return _blockIDContainer.get(c);
}

inline int Chunk::getBlockID(int c) const {
    return _blockIDContainer.get(c) & 0x0FFF;
}

inline const Block& Chunk::getBlock(int c) const {
    return Blocks[getBlockData(c)];
}
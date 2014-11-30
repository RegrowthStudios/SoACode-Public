#pragma once
#include "Chunk.h"
#include "global.h"
#include "BlockData.h"
#include "VoxelNavigation.inl"


inline void Chunk::changeState(ChunkStates State)
{
    //Only set the state if the new state is higher priority
    if (_state > State) {
        // Don't try to remesh it if its already queued for meshing.
        if (State == ChunkStates::MESH || State == ChunkStates::WATERMESH) {
            if (queuedForMesh) return;
        }
        _state = State;

        //isAccessible is a flag that prevents threads from trying to
        //access the chunk when its loading or generating
        if (_state > ChunkStates::GENERATE){
            isAccessible = true;
        } else{
            isAccessible = false;
        }
    }
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

inline int Chunk::getSunlightSafe(int c, Chunk*& lockedChunk) {
    vvox::swapLockedChunk(this, lockedChunk);
    return getSunlight(c);
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

inline void Chunk::setSunlightSafe(Chunk*& lockedChunk, int c, ui8 val) {
    vvox::swapLockedChunk(this, lockedChunk);
    setSunlight(c, val);
}

inline void Chunk::setLampLight(int c, ui16 val) {
    _lampLightContainer.set(c, val);
}

inline void Chunk::setLampLightSafe(Chunk*& lockedChunk, int c, ui16 val) {
    vvox::swapLockedChunk(this, lockedChunk);
    setLampLight(c, val);
}

// TODO(Ben): .setWithMask to avoid an extra traversal
inline void Chunk::setFloraHeight(int c, ui16 val) {
    _tertiaryDataContainer.set(c, (_tertiaryDataContainer.get(c) & (~FLORA_HEIGHT_MASK)) | val);
}

inline void Chunk::setBlockData(int c, ui16 val) {
    _blockIDContainer.set(c, val);
}

inline void Chunk::setBlockDataSafe(Chunk*& lockedChunk, int c, ui16 val) {
    vvox::swapLockedChunk(this, lockedChunk);
    setBlockData(c, val);
}

inline void Chunk::setTertiaryData(int c, ui16 val) {
    _tertiaryDataContainer.set(c, val);
}

inline void Chunk::setTertiaryDataSafe(Chunk*& lockedChunk, int c, ui16 val) {
    vvox::swapLockedChunk(this, lockedChunk);
    setTertiaryData(c, val);
}

inline ui16 Chunk::getBlockData(int c) const {
    return _blockIDContainer.get(c);
}

inline ui16 Chunk::getBlockDataSafe(Chunk*& lockedChunk, int c) {
    vvox::swapLockedChunk(this, lockedChunk);
    return getBlockData(c);
}

inline int Chunk::getBlockID(int c) const {
    return _blockIDContainer.get(c) & 0x0FFF;
}

inline int Chunk::getBlockIDSafe(Chunk*& lockedChunk, int c) {
    vvox::swapLockedChunk(this, lockedChunk);
    return getBlockID(c);
}

inline const Block& Chunk::getBlock(int c) const {
    return Blocks[getBlockData(c)];
}

inline const Block& Chunk::getBlockSafe(Chunk*& lockedChunk, int c) {
    vvox::swapLockedChunk(this, lockedChunk);
    return Blocks[getBlockData(c)];
}
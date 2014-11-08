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

inline int Chunk::getLeftBlockData(int c)
{
    if (c%CHUNK_WIDTH > 0){
        return getBlockData(c - 1);
    } else if (left && left->isAccessible){
        return left->getBlockData(c + CHUNK_WIDTH - 1);
    }
    return -1;
}

inline int Chunk::getLeftBlockData(int c, int x, int *c2, Chunk **owner)
{
    *owner = this;
    if (x > 0){
        *c2 = c - 1;
        return getBlockData(c - 1);
    } else if (left && left->isAccessible){
        *owner = left;
        *c2 = c + CHUNK_WIDTH - 1;
        return left->getBlockData(c + CHUNK_WIDTH - 1);
    }
    *c2 = NULL;
    return VISITED_NODE;
}

inline int Chunk::getRightBlockData(int c)
{
    if (c%CHUNK_WIDTH < CHUNK_WIDTH - 1){
        return getBlockData(c + 1);
    } else if (right && right->isAccessible){
        return right->getBlockData(c - CHUNK_WIDTH + 1);
    }
    return -1;
}

inline int Chunk::getRightBlockData(int c, int x, int *c2, Chunk **owner)
{
    *owner = this;
    if (x < CHUNK_WIDTH - 1){
        *c2 = c + 1;
        return getBlockData(c + 1);
    } else if (right && right->isAccessible){
        *owner = right;
        *c2 = c - CHUNK_WIDTH + 1;
        return right->getBlockData(c - CHUNK_WIDTH + 1);
    }
    *c2 = NULL;
    return VISITED_NODE;
}

inline int Chunk::getFrontBlockData(int c)
{
    if ((c%CHUNK_LAYER) / CHUNK_WIDTH < CHUNK_WIDTH - 1){
        return getBlockData(c + CHUNK_WIDTH);
    } else if (front && front->isAccessible){
        return front->getBlockData(c - CHUNK_LAYER + CHUNK_WIDTH);
    }
    return -1;
}

inline int Chunk::getFrontBlockData(int c, int z, int *c2, Chunk **owner)
{
    *owner = this;
    if (z < CHUNK_WIDTH - 1){
        *c2 = c + CHUNK_WIDTH;
        return getBlockData(c + CHUNK_WIDTH);
    } else if (front && front->isAccessible){
        *owner = front;
        *c2 = c - CHUNK_LAYER + CHUNK_WIDTH;
        return front->getBlockData(c - CHUNK_LAYER + CHUNK_WIDTH);
    }
    *c2 = NULL;
    return 33;
}

inline int Chunk::getBackBlockData(int c)
{
    if ((c%CHUNK_LAYER) / CHUNK_WIDTH > 0){
        return getBlockData(c - CHUNK_WIDTH);
    } else if (back && back->isAccessible){
        return back->getBlockData(c + CHUNK_LAYER - CHUNK_WIDTH);
    }
    return -1;
}

inline int Chunk::getBackBlockData(int c, int z, int *c2, Chunk **owner)
{
    *owner = this;
    if (z > 0){
        *c2 = c - CHUNK_WIDTH;
        return getBlockData(c - CHUNK_WIDTH);
    } else if (back && back->isAccessible){
        *owner = back;
        *c2 = c + CHUNK_LAYER - CHUNK_WIDTH;
        return back->getBlockData(c + CHUNK_LAYER - CHUNK_WIDTH);
    }
    *c2 = NULL;
    return VISITED_NODE;
}

inline int Chunk::getBottomBlockData(int c)
{
    if (c / CHUNK_LAYER > 0){
        return getBlockData(c - CHUNK_LAYER);
    } else if (bottom && bottom->isAccessible){
        return bottom->getBlockData(c + CHUNK_SIZE - CHUNK_LAYER);
    }
    return -1;
}

inline int Chunk::getBottomBlockData(int c, int y, int *c2, Chunk **owner)
{
    *owner = this;
    if (y > 0){
        *c2 = c - CHUNK_LAYER;
        return getBlockData(c - CHUNK_LAYER);
    } else if (bottom && bottom->isAccessible){
        *owner = bottom;
        *c2 = c + CHUNK_SIZE - CHUNK_LAYER;
        return bottom->getBlockData(c + CHUNK_SIZE - CHUNK_LAYER);
    }
    *c2 = NULL;
    return VISITED_NODE;
}

inline int Chunk::getTopBlockData(int c)
{
    if (c / CHUNK_LAYER < CHUNK_WIDTH - 1){
        return getBlockData(c + CHUNK_LAYER);
    } else if (top && top->isAccessible){
        return top->getBlockData(c - CHUNK_SIZE + CHUNK_LAYER);
    }
    return -1;
}

inline int Chunk::getTopSunlight(int c)
{
    if (c / CHUNK_LAYER < CHUNK_WIDTH - 1){
        return getSunlight(c + CHUNK_LAYER);
    } else if (top && top->isAccessible){
        return top->getSunlight(c - CHUNK_SIZE + CHUNK_LAYER);
    }
    return 0;
}

inline int Chunk::getTopBlockData(int c, int *c2, Chunk **owner)
{
    *owner = this;
    if (c / CHUNK_LAYER < CHUNK_WIDTH - 1){
        *c2 = c + CHUNK_LAYER;
        return getBlockData(c + CHUNK_LAYER);
    } else if (top && top->isAccessible){
        *owner = top;
        *c2 = c - CHUNK_SIZE + CHUNK_LAYER;
        return top->getBlockData(c - CHUNK_SIZE + CHUNK_LAYER);
    }
    *c2 = NULL;
    return 33;
}

inline int Chunk::getTopBlockData(int c, int y, int *c2, Chunk **owner)
{
    *owner = this;
    if (y < CHUNK_WIDTH - 1){
        *c2 = c + CHUNK_LAYER;
        return getBlockData(c + CHUNK_LAYER);
    } else if (top && top->isAccessible){
        *owner = top;
        *c2 = c - CHUNK_SIZE + CHUNK_LAYER;
        return top->getBlockData(c - CHUNK_SIZE + CHUNK_LAYER);
    }
    *c2 = NULL;
    return VISITED_NODE;
}

inline void Chunk::getLeftLightData(int c, GLbyte &l, GLbyte &sl)
{
    if (c%CHUNK_WIDTH > 0){
        if (getBlock(c - 1).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
     //       l = (lightData[0][c - 1]);
    //        sl = (lightData[1][c - 1]);
        }
    } else if (left && left->isAccessible){
        if (left->getBlock(c + CHUNK_WIDTH - 1).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
       //     l = (left->lightData[0][c + CHUNK_WIDTH - 1]);
       //     sl = (left->lightData[1][c + CHUNK_WIDTH - 1]);
        }
    } else{
        l = sl = 0;
    }
}

inline void Chunk::getRightLightData(int c, GLbyte &l, GLbyte &sl)
{
    if (c%CHUNK_WIDTH < CHUNK_WIDTH - 1){
        if (getBlock(c + 1).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
   //         l = (lightData[0][c + 1]);
   //         sl = (lightData[1][c + 1]);
        }
    } else if (right && right->isAccessible){
        if (right->getBlock(c - CHUNK_WIDTH + 1).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
  //          l = (right->lightData[0][c - CHUNK_WIDTH + 1]);
  //          sl = (right->lightData[1][c - CHUNK_WIDTH + 1]);
        }
    } else{
        l = sl = 0;
    }
}

inline void Chunk::getFrontLightData(int c, GLbyte &l, GLbyte &sl)
{
    if ((c%CHUNK_LAYER) / CHUNK_WIDTH < CHUNK_WIDTH - 1){
        if (getBlock(c + CHUNK_WIDTH).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
  //          l = (lightData[0][c + CHUNK_WIDTH]);
  //          sl = (lightData[1][c + CHUNK_WIDTH]);
        }
    } else if (front && front->isAccessible){
        if (front->getBlock(c - CHUNK_LAYER + CHUNK_WIDTH).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
  //          l = (front->lightData[0][c - CHUNK_LAYER + CHUNK_WIDTH]);
  //          sl = (front->lightData[1][c - CHUNK_LAYER + CHUNK_WIDTH]);
        }
    } else{
        l = sl = 0;
    }
}

inline void Chunk::getBackLightData(int c, GLbyte &l, GLbyte &sl)
{
    if ((c%CHUNK_LAYER) / CHUNK_WIDTH > 0){
        if (getBlock(c - CHUNK_WIDTH).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
  //          l = (lightData[0][c - CHUNK_WIDTH]);
  //          sl = (lightData[1][c - CHUNK_WIDTH]);
        }
    } else if (back && back->isAccessible){
        if (back->getBlock(c + CHUNK_LAYER - CHUNK_WIDTH).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
    //        l = (back->lightData[0][c + CHUNK_LAYER - CHUNK_WIDTH]);
   //         sl = (back->lightData[1][c + CHUNK_LAYER - CHUNK_WIDTH]);
        }
    } else{
        l = sl = 0;
    }
}

inline void Chunk::getBottomLightData(int c, GLbyte &l, GLbyte &sl)
{
    if (c / CHUNK_LAYER > 0){
        if (getBlock(c - CHUNK_LAYER).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
    //        l = lightData[0][c - CHUNK_LAYER];
   //         sl = lightData[1][c - CHUNK_LAYER];
        }
    } else if (bottom && bottom->isAccessible){
        if (bottom->getBlock(c + CHUNK_SIZE - CHUNK_LAYER).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
    //        l = bottom->lightData[0][c + CHUNK_SIZE - CHUNK_LAYER];
    //        sl = bottom->lightData[1][c + CHUNK_SIZE - CHUNK_LAYER];
        }
    } else{
        l = sl = 0;
    }
}

inline void Chunk::getTopLightData(int c, GLbyte &l, GLbyte &sl)
{
    if (c / CHUNK_LAYER < CHUNK_WIDTH - 1){
        if (getBlock(c + CHUNK_LAYER).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
 //           l = (lightData[0][c + CHUNK_LAYER]);
 //           sl = (lightData[1][c + CHUNK_LAYER]);
        }
    } else if (top && top->isAccessible){
        if (top->getBlock(c - CHUNK_SIZE + CHUNK_LAYER).occlude != BlockOcclusion::NONE){
            l = sl = -1;
        } else{
    //        l = (top->lightData[0][c - CHUNK_SIZE + CHUNK_LAYER]);
   //         sl = (top->lightData[1][c - CHUNK_SIZE + CHUNK_LAYER]);
        }
    } else{
        l = sl = 0;
    }
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
#pragma once
#include "Chunk.h"
#include "global.h"
#include "BlockData.h"

inline void Chunk::changeState(ChunkStates State)
{
    //Only set the state if the new state is higher priority
    if (state > State){
        state = State;

        //isAccessible is a flag that prevents threads from trying to
        //acces the chunk when its loading or generating
        if (state > ChunkStates::GENERATE){
            isAccessible = 1;
        } else{
            isAccessible = 0;
        }
    }
}

inline int Chunk::getLeftBlockData(int c)
{
    if (c%CHUNK_WIDTH > 0){
        return data[c - 1];
    } else if (left && left->isAccessible){
        return (left->data[c + CHUNK_WIDTH - 1]);
    }
    return -1;
}

inline int Chunk::getLeftBlockData(int c, int x, int *c2, Chunk **owner)
{
    *owner = this;
    if (x > 0){
        *c2 = c - 1;
        return (data[c - 1]);
    } else if (left && left->isAccessible){
        *owner = left;
        *c2 = c + CHUNK_WIDTH - 1;
        return (left->data[c + CHUNK_WIDTH - 1]);
    }
    *c2 = NULL;
    return VISITED_NODE;
}

inline int Chunk::getRightBlockData(int c)
{
    if (c%CHUNK_WIDTH < CHUNK_WIDTH - 1){
        return (data[c + 1]);
    } else if (right && right->isAccessible){
        return (right->data[c - CHUNK_WIDTH + 1]);
    }
    return -1;
}

inline int Chunk::getRightBlockData(int c, int x, int *c2, Chunk **owner)
{
    *owner = this;
    if (x < CHUNK_WIDTH - 1){
        *c2 = c + 1;
        return (data[c + 1]);
    } else if (right && right->isAccessible){
        *owner = right;
        *c2 = c - CHUNK_WIDTH + 1;
        return (right->data[c - CHUNK_WIDTH + 1]);
    }
    *c2 = NULL;
    return VISITED_NODE;
}

inline int Chunk::getFrontBlockData(int c)
{
    if ((c%CHUNK_LAYER) / CHUNK_WIDTH < CHUNK_WIDTH - 1){
        return data[c + CHUNK_WIDTH];
    } else if (front && front->isAccessible){
        return front->data[c - CHUNK_LAYER + CHUNK_WIDTH];
    }
    return -1;
}

inline int Chunk::getFrontBlockData(int c, int z, int *c2, Chunk **owner)
{
    *owner = this;
    if (z < CHUNK_WIDTH - 1){
        *c2 = c + CHUNK_WIDTH;
        return ((data[c + CHUNK_WIDTH]));
    } else if (front && front->isAccessible){
        *owner = front;
        *c2 = c - CHUNK_LAYER + CHUNK_WIDTH;
        return ((front->data[c - CHUNK_LAYER + CHUNK_WIDTH]));
    }
    *c2 = NULL;
    return 33;
}

inline int Chunk::getBackBlockData(int c)
{
    if ((c%CHUNK_LAYER) / CHUNK_WIDTH > 0){
        return (data[c - CHUNK_WIDTH]);
    } else if (back && back->isAccessible){
        return back->data[c + CHUNK_LAYER - CHUNK_WIDTH];
    }
    return -1;
}

inline int Chunk::getBackBlockData(int c, int z, int *c2, Chunk **owner)
{
    *owner = this;
    if (z > 0){
        *c2 = c - CHUNK_WIDTH;
        return (data[c - CHUNK_WIDTH]);
    } else if (back && back->isAccessible){
        *owner = back;
        *c2 = c + CHUNK_LAYER - CHUNK_WIDTH;
        return (back->data[c + CHUNK_LAYER - CHUNK_WIDTH]);
    }
    *c2 = NULL;
    return VISITED_NODE;
}

inline int Chunk::getBottomBlockData(int c)
{
    if (c / CHUNK_LAYER > 0){
        return data[c - CHUNK_LAYER];
    } else if (bottom && bottom->isAccessible){
        return bottom->data[c + CHUNK_SIZE - CHUNK_LAYER];
    }
    return -1;
}

inline int Chunk::getBottomBlockData(int c, int y, int *c2, Chunk **owner)
{
    *owner = this;
    if (y > 0){
        *c2 = c - CHUNK_LAYER;
        return (data[c - CHUNK_LAYER]);
    } else if (bottom && bottom->isAccessible){
        *owner = bottom;
        *c2 = c + CHUNK_SIZE - CHUNK_LAYER;
        return (bottom->data[c + CHUNK_SIZE - CHUNK_LAYER]);
    }
    *c2 = NULL;
    return VISITED_NODE;
}

inline int Chunk::getTopBlockData(int c)
{
    if (c / CHUNK_LAYER < CHUNK_WIDTH - 1){
        return data[c + CHUNK_LAYER];
    } else if (top && top->isAccessible){
        return top->data[c - CHUNK_SIZE + CHUNK_LAYER];
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
        return (data[c + CHUNK_LAYER]);
    } else if (top && top->isAccessible){
        *owner = top;
        *c2 = c - CHUNK_SIZE + CHUNK_LAYER;
        return (top->data[c - CHUNK_SIZE + CHUNK_LAYER]);
    }
    *c2 = NULL;
    return 33;
}

inline int Chunk::getTopBlockData(int c, int y, int *c2, Chunk **owner)
{
    *owner = this;
    if (y < CHUNK_WIDTH - 1){
        *c2 = c + CHUNK_LAYER;
        return (data[c + CHUNK_LAYER]);
    } else if (top && top->isAccessible){
        *owner = top;
        *c2 = c - CHUNK_SIZE + CHUNK_LAYER;
        return (top->data[c - CHUNK_SIZE + CHUNK_LAYER]);
    }
    *c2 = NULL;
    return VISITED_NODE;
}

inline void Chunk::getLeftLightData(int c, GLbyte &l, GLbyte &sl)
{
    if (c%CHUNK_WIDTH > 0){
        if (Blocks[GETBLOCKTYPE(data[c - 1])].occlude){
            l = sl = -1;
        } else{
     //       l = (lightData[0][c - 1]);
    //        sl = (lightData[1][c - 1]);
        }
    } else if (left && left->isAccessible){
        if (Blocks[GETBLOCKTYPE(left->data[c + CHUNK_WIDTH - 1])].occlude){
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
        if (Blocks[GETBLOCKTYPE(data[c + 1])].occlude){
            l = sl = -1;
        } else{
   //         l = (lightData[0][c + 1]);
   //         sl = (lightData[1][c + 1]);
        }
    } else if (right && right->isAccessible){
        if (Blocks[GETBLOCKTYPE(right->data[c - CHUNK_WIDTH + 1])].occlude){
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
        if (Blocks[GETBLOCKTYPE(data[c + CHUNK_WIDTH])].occlude){
            l = sl = -1;
        } else{
  //          l = (lightData[0][c + CHUNK_WIDTH]);
  //          sl = (lightData[1][c + CHUNK_WIDTH]);
        }
    } else if (front && front->isAccessible){
        if (Blocks[GETBLOCKTYPE(front->data[c - CHUNK_LAYER + CHUNK_WIDTH])].occlude){
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
        if (Blocks[GETBLOCKTYPE(data[c - CHUNK_WIDTH])].occlude){
            l = sl = -1;
        } else{
  //          l = (lightData[0][c - CHUNK_WIDTH]);
  //          sl = (lightData[1][c - CHUNK_WIDTH]);
        }
    } else if (back && back->isAccessible){
        if (Blocks[GETBLOCKTYPE(back->data[c + CHUNK_LAYER - CHUNK_WIDTH])].occlude){
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
        if (Blocks[GETBLOCKTYPE(data[c - CHUNK_LAYER])].occlude){
            l = sl = -1;
        } else{
    //        l = lightData[0][c - CHUNK_LAYER];
   //         sl = lightData[1][c - CHUNK_LAYER];
        }
    } else if (bottom && bottom->isAccessible){
        if (Blocks[GETBLOCKTYPE(bottom->data[c + CHUNK_SIZE - CHUNK_LAYER])].occlude){
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
        if (Blocks[GETBLOCKTYPE(data[c + CHUNK_LAYER])].occlude){
            l = sl = -1;
        } else{
 //           l = (lightData[0][c + CHUNK_LAYER]);
 //           sl = (lightData[1][c + CHUNK_LAYER]);
        }
    } else if (top && top->isAccessible){
        if (Blocks[GETBLOCKTYPE(top->data[c - CHUNK_SIZE + CHUNK_LAYER])].occlude){
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
    return sunlightData[c] & 0x1F;
}

inline ui16 Chunk::getLampLight(int c) const {
    if (lampLightData) {
        return lampLightData[c];
    } else {
        return 0;
    }
}


inline void Chunk::setSunlight(int c, int val) {
    sunlightData[c] = (sunlightData[c] & 0xE0) | val;
}

void Chunk::setLampLight(int c, int val) {
    if (lampLightData == nullptr) {
        lampLightData = new ui16[CHUNK_SIZE];
    }
    lampLightData[c] = val;
}
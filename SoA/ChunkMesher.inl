#include "ChunkMesher.h"
#include "Chunk.h"

#define GET_L_COLOR(a) a[0] = (lightColor & LAMP_RED_MASK) >> LAMP_RED_SHIFT; \
    a[1] = (lightColor & LAMP_GREEN_MASK) >> LAMP_GREEN_SHIFT; \
    a[2] = lightColor & LAMP_BLUE_MASK;

inline void ChunkMesher::GetLeftLightData(int c, ui8 l[3], i8 &sl, ui16 *data, ui8* sunlightData, ui16* lampData)
{
    if (Blocks[GETBLOCKTYPE(data[c - 1])].occlude){
        memset(l, 0, 3);
        sl = -1;
    } else{
        ui16 lightColor = lampData[c - 1];
        GET_L_COLOR(l);
        sl = sunlightData[c - 1];
    }
}
inline void ChunkMesher::GetRightLightData(int c, ui8 l[3], i8 &sl, ui16 *data, ui8* sunlightData, ui16* lampData)
{

    if (Blocks[GETBLOCKTYPE(data[c + 1])].occlude){
        memset(l, 0, 3);
        sl = -1;
    } else{
        ui16 lightColor = lampData[c + 1];
        GET_L_COLOR(l);
        sl = sunlightData[c + 1];
    }
}
inline void ChunkMesher::GetFrontLightData(int c, ui8 l[3], i8 &sl, ui16 *data, ui8* sunlightData, ui16* lampData)
{
    if (Blocks[GETBLOCKTYPE(data[c + dataWidth])].occlude){
        memset(l, 0, 3);
        sl = -1;
    } else{
        ui16 lightColor = lampData[c + dataWidth];
        GET_L_COLOR(l);
        sl = sunlightData[c + dataWidth];
    }
}
inline void ChunkMesher::GetBackLightData(int c, ui8 l[3], i8 &sl, ui16 *data, ui8* sunlightData, ui16* lampData)
{
    if (Blocks[GETBLOCKTYPE(data[c - dataWidth])].occlude){
        memset(l, 0, 3);
        sl = -1;
    } else{
        ui16 lightColor = lampData[c - dataWidth];
        GET_L_COLOR(l);
        sl = sunlightData[c - dataWidth];
    }
}
inline void ChunkMesher::GetBottomLightData(int c, ui8 l[3], i8 &sl, ui16 *data, ui8* sunlightData, ui16* lampData)
{
    if (Blocks[GETBLOCKTYPE(data[c - dataLayer])].occlude){
        memset(l, 0, 3);
        sl = -1;
    } else{
        ui16 lightColor = lampData[c - dataLayer];
        GET_L_COLOR(l);
        sl = sunlightData[c - dataLayer];
    }
}
inline void ChunkMesher::GetTopLightData(int c, ui8 l[3], i8 &sl, ui16 *data, ui8* sunlightData, ui16* lampData)
{
    if (Blocks[GETBLOCKTYPE(data[c + dataLayer])].occlude){
        memset(l, 0, 3);
        sl = -1;
    } else{
        ui16 lightColor = lampData[c + dataLayer];
        GET_L_COLOR(l);
        sl = sunlightData[c + dataLayer];
    }
}
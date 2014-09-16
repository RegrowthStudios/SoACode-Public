#pragma once

#include "readerwriterqueue.h"

//Used to tell neighbors to update light
//struct LightMessage {
//    LightMessage() {};
//    LightMessage(ui16 BlockIndex, ui8 LightType, i8 LightValue) : blockIndex(BlockIndex), lightType(LightType), lightValue(LightValue) {}
//    ui16 blockIndex;
//    ui8 lightType;
//    i8 lightValue;
//};

struct SunlightRemovalNode
{
    SunlightRemovalNode(ui16 BlockIndex, ui8 OldLightVal) : blockIndex(BlockIndex), oldLightVal(OldLightVal){}
    ui16 blockIndex;
    ui8 oldLightVal;
};

struct SunlightUpdateNode
{
    SunlightUpdateNode(ui16 BlockIndex, ui8 LightVal) : blockIndex(BlockIndex), lightVal(LightVal){}
    ui16 blockIndex;
    ui8 lightVal;
};

struct LampLightRemovalNode
{
    LampLightRemovalNode(ui16 BlockIndex, ui16 OldLightColor) : blockIndex(BlockIndex), oldLightColor(OldLightColor){}
    ui16 blockIndex;
    ui16 oldLightColor;
};

struct LampLightUpdateNode
{
    LampLightUpdateNode(ui16 BlockIndex, ui16 LightColor) : blockIndex(BlockIndex), lightColor(LightColor){}
    ui16 blockIndex;
    ui16 lightColor;
};

class Chunk;

class VoxelLightEngine {
public:
    static void calculateLight(Chunk* chunk);
    static void calculateSunlightExtend(Chunk* chunk);
    static void calculateSunlightRemoval(Chunk* chunk);

    static void checkTopForSunlight(Chunk* chunk);
private:
    static void flushLightQueue(Chunk* chunk, moodycamel::ReaderWriterQueue<ui32>& queue);
    
    static void blockSunRay(Chunk* chunk, int xz, int y);
    static void extendSunRay(Chunk* chunk, int xz, int y);
    static void removeSunlightBFS(Chunk* chunk, int blockIndex, ui8 oldLightVal);
    static void placeSunlight(Chunk* chunk, int blockIndex, int intensity);
};
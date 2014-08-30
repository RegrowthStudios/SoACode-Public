#pragma once

#include "readerwriterqueue.h"

//Used to tell neighbors to update light
struct LightMessage {
    LightMessage() {};
    LightMessage(ui16 BlockIndex, ui8 LightType, i8 LightValue) : blockIndex(BlockIndex), lightType(LightType), lightValue(LightValue) {}
    ui16 blockIndex;
    ui8 lightType;
    i8 lightValue;
};

struct LightRemovalNode
{
    LightRemovalNode(ui16 C, ui8 Type, ui8 OldLightVal) : c(C), type(Type), oldLightVal(OldLightVal){}
    ui16 c;
    ui8 type;
    i8 oldLightVal;
};

struct LightUpdateNode
{
    LightUpdateNode(ui16 C, ui8 Type, i8 LightVal) : c(C), type(Type), lightVal(LightVal){}
    ui16 c;
    ui8 type;
    i8 lightVal;
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
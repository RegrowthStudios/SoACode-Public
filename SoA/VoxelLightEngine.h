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
    void calculateLight(Chunk* chunk);
    void calculateSunlightExtend(Chunk* chunk);
    void calculateSunlightRemoval(Chunk* chunk);

    void checkTopForSunlight(Chunk* chunk);
private:
   
    void blockSunRay(Chunk* chunk, int xz, int y);
    void extendSunRay(Chunk* chunk, int xz, int y);
    void removeSunlightBFS(Chunk* chunk, int blockIndex, ui8 oldLightVal);
    void placeSunlightBFS(Chunk* chunk, int blockIndex, ui8 intensity);
    void removeLampLightBFS(Chunk* chunk, int blockIndex, ui16 light);
    void placeLampLightBFS(Chunk* chunk, int blockIndex, ui16 intensity);

    void lockChunk(Chunk* chunk);

    Chunk* _lockedChunk = nullptr;
};
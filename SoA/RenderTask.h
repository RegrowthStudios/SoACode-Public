#pragma once
#include "Constants.h"

class Chunk;

// For Use With Render Task's Type
#define RENDER_TASK_RENDER 0
#define RENDER_TASK_WATER 1

// Sizes For A Padded Chunk
const int PADDED_CHUNK_WIDTH = (CHUNK_WIDTH + 2);
const int PADDED_CHUNK_LAYER = (PADDED_CHUNK_WIDTH * PADDED_CHUNK_WIDTH);
const int PADDED_CHUNK_SIZE = (PADDED_CHUNK_LAYER * PADDED_CHUNK_WIDTH);

enum class MeshJobType { DEFAULT, LIQUID };

// Represents A Mesh Creation Task
struct RenderTask {
public:
    // Helper Function To Set The Chunk Data
    void setChunk(Chunk* ch, MeshJobType cType);

    // Notice that the edges of the chunk data are padded. We have to do this because
    // We don't want to have to access neighboring chunks in multiple threads, that requires
    // us to use mutex locks. This way is much faster and also prevents bounds checking
    ui16 chData[PADDED_CHUNK_SIZE];
    ui16 chLampData[PADDED_CHUNK_SIZE];
    ui8 chSunlightData[PADDED_CHUNK_SIZE];
    ui8 biomes[CHUNK_LAYER]; //lookup for biomesLookupMap
    ui8 temperatures[CHUNK_LAYER];
    ui8 rainfalls[CHUNK_LAYER];
    ui8 depthMap[CHUNK_LAYER];
	i32 wSize;
	ui16 wvec[CHUNK_SIZE];
	i32 num;
    MeshJobType type; // RENDER_TASK_RENDER, RENDER_TASK_WATER
	i32v3 position;
    Chunk* chunk;
    int levelOfDetail;
};
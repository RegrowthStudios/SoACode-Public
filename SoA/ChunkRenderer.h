#pragma once

#ifndef ChunkRenderer_h__
#define ChunkRenderer_h__

#include <vector>
#include "types.h"
#include "ChunkMesh.h"
#include "GLProgram.h"

class GameRenderParams;

class ChunkRenderer {
public:
    static void drawSonar(const GameRenderParams* gameRenderParams);
    static void drawBlocks(const GameRenderParams* gameRenderParams);
    static void drawCutoutBlocks(const GameRenderParams* gameRenderParams);
    static void drawTransparentBlocks(const GameRenderParams* gameRenderParams);
    //static void drawPhysicsBlocks(const GameRenderParams* gameRenderParams);
    static void drawWater(const GameRenderParams* gameRenderParams);

    static void bindTransparentVao(ChunkMesh *CMI);
    static void bindCutoutVao(ChunkMesh *CMI);
    static void bindVao(ChunkMesh *CMI);
    static void bindWaterVao(ChunkMesh *CMI);
   
private:
    static void drawChunkBlocks(const ChunkMesh *CMI, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP);
    static void drawChunkTransparentBlocks(const ChunkMesh *CMI, const vg::GLProgram* program, const f64v3 &playerPos, const f32m4 &VP);
    static void drawChunkCutoutBlocks(const ChunkMesh *CMI, const vg::GLProgram* program, const f64v3 &playerPos, const f32m4 &VP);
    static void drawChunkWater(const ChunkMesh *CMI, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP);

};

#endif // ChunkRenderer_h__
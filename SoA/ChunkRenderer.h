#pragma once

#ifndef ChunkRenderer_h__
#define ChunkRenderer_h__

#include <Vorb/graphics/GLProgram.h>

#include "ChunkMesh.h"

class GameRenderParams;
class PhysicsBlockMesh;

class ChunkRenderer {
public:
    static void drawSonar(const GameRenderParams* gameRenderParams);
    static void drawBlocks(const GameRenderParams* gameRenderParams);
    static void drawCutoutBlocks(const GameRenderParams* gameRenderParams);
    static void drawTransparentBlocks(const GameRenderParams* gameRenderParams);
    static void drawWater(const GameRenderParams* gameRenderParams);

    static void bindTransparentVao(ChunkMesh *cm);
    static void bindCutoutVao(ChunkMesh *cm);
    static void bindVao(ChunkMesh *cm);
    static void bindWaterVao(ChunkMesh *cm);
   
private:
    static void drawChunkBlocks(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP);
    static void drawChunkTransparentBlocks(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &playerPos, const f32m4 &VP);
    static void drawChunkCutoutBlocks(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &playerPos, const f32m4 &VP);
    static void drawChunkWater(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP);

    static f32m4 worldMatrix; ///< Reusable world matrix for chunks
};

#endif // ChunkRenderer_h__
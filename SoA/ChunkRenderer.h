#pragma once

#ifndef ChunkRenderer_h__
#define ChunkRenderer_h__

#include <Vorb/graphics/GLProgram.h>

#include "ChunkMesh.h"

class GameRenderParams;
class PhysicsBlockMesh;

class ChunkRenderer {
public:
    void drawSonar(const GameRenderParams* gameRenderParams);
    void drawBlocks(const GameRenderParams* gameRenderParams);
    void drawCutoutBlocks(const GameRenderParams* gameRenderParams);
    void drawTransparentBlocks(const GameRenderParams* gameRenderParams);
    void drawWater(const GameRenderParams* gameRenderParams);

    static void bindTransparentVao(ChunkMesh *cm);
    static void bindCutoutVao(ChunkMesh *cm);
    static void bindVao(ChunkMesh *cm);
    static void bindWaterVao(ChunkMesh *cm);
   
    volatile f32 fadeDist;
private:
    void drawChunkBlocks(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP);
    void drawChunkTransparentBlocks(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &playerPos, const f32m4 &VP);
    void drawChunkCutoutBlocks(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &playerPos, const f32m4 &VP);
    void drawChunkWater(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP);

    f32m4 worldMatrix; ///< Reusable world matrix for chunks
};

#endif // ChunkRenderer_h__
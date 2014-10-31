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
    static void drawSonar(const std::vector <ChunkMesh *>& chunkMeshes, const f32m4 &VP, const f64v3 &position);
    static void drawBlocks(const std::vector <ChunkMesh *>& chunkMeshes, const f32m4 &VP, const GameRenderParams* gameRenderParams, const f64v3 &position, const f32v3& eyeDir);
    static void drawCutoutBlocks(const std::vector <ChunkMesh *>& chunkMeshes, const f32m4 &VP, const GameRenderParams* gameRenderParams, const f64v3 &position, const f32v3& eyeDir);
    static void drawTransparentBlocks(const std::vector <ChunkMesh *>& chunkMeshes, const f32m4 &VP, const GameRenderParams* gameRenderParams, const f64v3 &position, const f32v3& eyeDir);
    //static void drawPhysicsBlocks(f32m4 &VP, const f64v3 &position, GLfloat lightActive, const GLfloat *eyeDir);
    static void drawWater(const std::vector <ChunkMesh *>& chunkMeshes, const f32m4 &VP, const GameRenderParams* gameRenderParams, const f64v3 &position, bool underWater);

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
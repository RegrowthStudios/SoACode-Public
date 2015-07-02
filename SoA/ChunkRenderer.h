#pragma once

#ifndef ChunkRenderer_h__
#define ChunkRenderer_h__

#include <Vorb/graphics/GLProgram.h>

#include "ChunkMesh.h"

class GameRenderParams;
class PhysicsBlockMesh;

#define CHUNK_DIAGONAL_LENGTH 28.0f

class ChunkRenderer {
public:
    static void drawOpaque(const ChunkMesh* cm, const vg::GLProgram& program, const f64v3& PlayerPos, const f32m4& VP);
    static void drawTransparent(const ChunkMesh* cm, const vg::GLProgram& program, const f64v3& playerPos, const f32m4& VP);
    static void drawCutout(const ChunkMesh* cm, const vg::GLProgram& program, const f64v3& playerPos, const f32m4& VP);
    static void drawWater(const ChunkMesh* cm, const vg::GLProgram& program, const f64v3& PlayerPos, const f32m4& VP);

    static volatile f32 fadeDist;
private:
    static f32m4 worldMatrix; ///< Reusable world matrix for chunks
};

#endif // ChunkRenderer_h__

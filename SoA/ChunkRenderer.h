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
    // Loads the shaders. Call on render thread.
    void init();
    void dispose();

    void beginOpaque(VGTexture textureAtlas, const f32v3& sunDir, const f32v3& lightColor = f32v3(1.0f), const f32v3& ambient = f32v3(0.0f));
    void drawOpaque(const ChunkMesh* cm, const f64v3& PlayerPos, const f32m4& VP) const;
    static void drawOpaqueCustom(const ChunkMesh* cm, vg::GLProgram& m_program, const f64v3& PlayerPos, const f32m4& VP);

    void beginTransparent(VGTexture textureAtlas, const f32v3& sunDir, const f32v3& lightColor = f32v3(1.0f), const f32v3& ambient = f32v3(0.0f));
    void drawTransparent(const ChunkMesh* cm, const f64v3& playerPos, const f32m4& VP) const;
    
    void beginCutout(VGTexture textureAtlas, const f32v3& sunDir, const f32v3& lightColor = f32v3(1.0f), const f32v3& ambient = f32v3(0.0f));
    void drawCutout(const ChunkMesh* cm, const f64v3& playerPos, const f32m4& VP) const;

    void beginLiquid(VGTexture textureAtlas, const f32v3& sunDir, const f32v3& lightColor = f32v3(1.0f), const f32v3& ambient = f32v3(0.0f));
    void drawLiquid(const ChunkMesh* cm, const f64v3& PlayerPos, const f32m4& VP) const;

    // Unbinds the shader
    static void end();

    static volatile f32 fadeDist;
private:
    static f32m4 worldMatrix; ///< Reusable world matrix for chunks
    vg::GLProgram m_opaqueProgram;
    vg::GLProgram m_transparentProgram;
    vg::GLProgram m_cutoutProgram;
    vg::GLProgram m_waterProgram;
};

#endif // ChunkRenderer_h__

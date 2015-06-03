///
/// LenseFlareRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 25 Apr 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Renders lense flare for stars and other bright objects
///

#pragma once

#ifndef LenseFlareRenderer_h__
#define LenseFlareRenderer_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/gtypes.h>
#include <Vorb/graphics/GLProgram.h>

class ModPathResolver;
struct FlareKegProperties;

class LenseFlareRenderer {
public:
    LenseFlareRenderer(const ModPathResolver* textureResolver);
    ~LenseFlareRenderer();
    void render(const f32m4& VP, const f64v3& relCamPos,
                const f32v3& color,
                float aspectRatio,
                f32 size,
                f32 intensity);

    void dispose();
private:
    void lazyInit();
    void loadSprites(FlareKegProperties& kegProps);
    void initMesh();

    const ModPathResolver* m_textureResolver = nullptr;
    vg::GLProgram m_program;
    VGTexture m_texture = 0;
    ui32 m_texWidth = 0;
    ui32 m_texHeight = 0;
    VGBuffer m_vbo = 0;
    VGBuffer m_ibo = 0;
    VGVertexArray m_vao = 0;

    int m_numSprites = 0;
    f32 m_intensity = 0.0f;

    VGUniform m_unColor = 0; // TODO(Ben): UBO?
};

#endif // LenseFlareRenderer_h__

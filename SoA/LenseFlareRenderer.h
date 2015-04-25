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

DECL_VG(class GLProgram);
class ModPathResolver;

class LenseFlareRenderer {
public:
    LenseFlareRenderer(const ModPathResolver* textureResolver);
    ~LenseFlareRenderer();
    void render(const f32m4& VP, const f64v3& relCamPos,
                const ui32v2& screenDims, const f32v3& color,
                f32 size);

    void dispose();
private:
    void lazyInit();
    void initMesh();

    const ModPathResolver* m_textureResolver = nullptr;
    vg::GLProgram* m_program = nullptr;
    VGTexture m_texture = 0;
    VGBuffer m_vbo = 0;
    VGBuffer m_ibo = 0;
    VGVertexArray m_vao = 0;

    VGUniform m_unColor = 0; // TODO(Ben): UBO?
};

#endif // LenseFlareRenderer_h__

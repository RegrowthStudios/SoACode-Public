///
/// StarComponentRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 9 Apr 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Renderer for StarComponents
///

#pragma once

#ifndef StarComponentRenderer_h__
#define StarComponentRenderer_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/ecs/ECS.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/gtypes.h>
// #include <Vorb/script/Environment.h>
// #include <Vorb/script/Function.h>

class ModPathResolver;

struct StarComponent;

class StarComponentRenderer {
public:
    StarComponentRenderer();
    ~StarComponentRenderer();

    void init(const ModPathResolver* textureResolver);
    void initGL();

    void drawStar(const StarComponent& sCmp,
                  const f32m4& VP,
                  const f64q& orientation,
                  const f32v3& relCamPos,
                  const f32 zCoef);
    void drawCorona(StarComponent& sCmp,
                    const f32m4& VP,
                    const f32m4& V,
                    const f32v3& relCamPos,
                    const f32 zCoef);
    void drawGlow(const StarComponent& sCmp,
                  const f32m4& VP,
                  const f64v3& relCamPos,
                  float aspectRatio,
                  const f32v3& viewDirW,
                  const f32v3& viewRightW,
                  const f32v3& colorMult = f32v3(1.0f));
    void updateOcclusionQuery(StarComponent& sCmp,
                              const f32 zCoef,
                              const f32m4& VP,
                              const f64v3& relCamPos);

    void dispose();
    void disposeShaders();
    void disposeBuffers();

    f32v3 calculateStarColor(const StarComponent& sCmp);
    f64 calculateGlowSize(const StarComponent& sCmp, const f64v3& relCamPos);

private:
    void buildShaders();
    void buildMesh();
    void loadTempColorMap();
    void loadGlowTextures();
    f32v3 getColor(int index);
    f32v3 getTempColorShift(const StarComponent& sCmp);

    vg::GLProgram m_starProgram;
    vg::GLProgram m_coronaProgram;
    vg::GLProgram m_glowProgram;
    vg::GLProgram m_occlusionProgram;
    // Star
    VGBuffer m_sVbo = 0;
    VGIndexBuffer m_sIbo = 0;
    VGVertexArray m_sVao = 0;
    // Corona
    VGBuffer m_cVbo = 0;
    VGIndexBuffer m_cIbo = 0;
    VGVertexArray m_cVao = 0;
    // Glow
    VGVertexArray m_gVao = 0;
    // Occlusion
    VGVertexArray m_oVao = 0;
    
    vg::BitmapResource m_tempColorMap;
    VGTexture m_glowColorMap = 0;
    int m_numIndices = 0;

    // TODO(Ben): UBO
    VGUniform unWVP;
    VGUniform unDT;

    const ModPathResolver* m_textureResolver = nullptr;
};

#endif // StarComponentRenderer_h__

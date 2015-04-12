///
/// StarComponentRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 9 Apr 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Renderer for StarComponents
///

#pragma once

#ifndef StarComponentRenderer_h__
#define StarComponentRenderer_h__

#include <Vorb/ecs/ECS.h>
#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/gtypes.h>
#include <Vorb/graphics/ImageIO.h>

DECL_VG(class GLProgram)

struct StarComponent;

class StarComponentRenderer {
public:
    StarComponentRenderer();
    ~StarComponentRenderer();

    void drawStar(const StarComponent& sCmp,
                  const f32m4& VP,
                  const f64q& orientation,
                  const f32v3& relCamPos);
    void drawCorona(const StarComponent& sCmp,
                    const f32m4& VP,
                    const f32m4& V,
                    const f32v3& relCamPos);
    void drawGlow(const StarComponent& sCmp,
                  const f32m4& VP,
                  const f32m4& V,
                  const f32v3& relCamPos);

    void dispose();
    void disposeShaders();
    void disposeBuffers();
private:
    void checkLazyLoad();
    void buildShaders();
    void buildMesh();
    void loadTempColorMap();
    void loadGlowTexture();
    f32v3 calculateStarColor(const StarComponent& sCmp);
    f32 getTempBrightnessMod(const StarComponent& sCmp);

    vg::GLProgram* m_starProgram = nullptr;
    vg::GLProgram* m_coronaProgram = nullptr;
    vg::GLProgram* m_glowProgram = nullptr;
    // Star
    VGBuffer m_sVbo = 0;
    VGIndexBuffer m_sIbo = 0;
    VGVertexArray m_sVao = 0;
    // Corona
    VGBuffer m_cVbo = 0;
    VGIndexBuffer m_cIbo = 0;
    VGVertexArray m_cVao = 0;
    // Glow
    VGBuffer m_gVao = 0;
    
    vg::BitmapResource m_tempColorMap;
    int m_numIndices = 0;

    // TODO(Ben): UBO
    VGUniform unWVP;
    VGUniform unDT;

    VGTexture m_glowTexture = 0;
};

#endif // StarComponentRenderer_h__

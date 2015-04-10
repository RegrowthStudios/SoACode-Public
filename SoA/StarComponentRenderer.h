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

DECL_VG(class GLProgram)

struct StarComponent;

class StarComponentRenderer {
public:
    StarComponentRenderer();
    ~StarComponentRenderer();

    void draw(const StarComponent& sCmp,
              const f32m4& VP,
              const f32m4& V,
              const f64q& orientation,
              const f32v3& relCamPos);
    void disposeShaders();
private:
    void disposeBuffers();
    void drawStar(const StarComponent& sCmp,
                  const f32m4& VP,
                  const f64q& orientation,
                  const f32v3& relCamPos);
    void drawCorona(const StarComponent& sCmp,
                    const f32m4& VP,
                    const f32m4& V,
                    const f32v3& relCamPos);
    void buildShaders();
    void buildMesh();

    vg::GLProgram* m_starProgram = nullptr;
    vg::GLProgram* m_coronaProgram = nullptr;
    // Star
    VGBuffer m_sVbo = 0;
    VGIndexBuffer m_sIbo = 0;
    VGVertexArray m_sVao = 0;
    // Corona
    VGBuffer m_cVbo = 0;
    VGIndexBuffer m_cIbo = 0;
    VGVertexArray m_cVao = 0;
    int m_numIndices = 0;

    // TODO(Ben): UBO
    VGUniform unWVP;
    VGUniform unDT;
};

#endif // StarComponentRenderer_h__

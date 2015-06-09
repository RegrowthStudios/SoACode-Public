///
/// GasGiantComponentRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 3 Apr 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Renders gas giant components
///

#pragma once

#ifndef GasGiantComponentRenderer_h__
#define GasGiantComponentRenderer_h__

#include <Vorb/ecs/ECS.h>
#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/gtypes.h>
#include <Vorb/graphics/GLProgram.h>

struct GasGiantComponent;
struct SpaceLightComponent;
struct AtmosphereComponent;

struct GasGiantVertex {
    f32v3 position;
    f32 texCoord;
};

class GasGiantComponentRenderer {
public:
    ~GasGiantComponentRenderer();

    void initGL();

    void draw(const GasGiantComponent& ggCmp,
              const f32m4& VP,
              const f64q& orientation,
              const f32v3& relCamPos,
              const f32v3& lightDir,
              const float zCoef,
              const SpaceLightComponent* spCmp,
              const AtmosphereComponent* aCmp);
    void dispose();
private:
    void buildShader();
    void buildMesh();

    vg::GLProgram m_program;
    VGBuffer m_vbo = 0;
    VGIndexBuffer m_ibo = 0;
    VGVertexArray m_vao = 0;
    int m_numIndices = 0;

    // TODO(Ben): UBO
    VGUniform unWVP;
    VGUniform unDT;
};

#endif // GasGiantComponentRenderer_h__


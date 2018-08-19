///
/// AtmosphereComponentRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Mar 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Renders atmosphere components
///

#pragma once

#ifndef AtmosphereComponentRenderer_h__
#define AtmosphereComponentRenderer_h__

#include <Vorb/ecs/ECS.h>
#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/gtypes.h>
#include <Vorb/graphics/GLProgram.h>

struct AtmosphereComponent;
struct SpaceLightComponent;

class AtmosphereComponentRenderer {
public:
    ~AtmosphereComponentRenderer();

    void initGL();
    void draw(const AtmosphereComponent& aCmp,
              const f32m4& VP,
              const f32v3& relCamPos,
              const f32v3& lightDir,
              const f32 zCoef,
              const SpaceLightComponent* spComponent);
    void dispose();
private:
    void buildMesh();

    vg::GLProgram m_program;
    VGBuffer m_icoVbo = 0;
    VGIndexBuffer m_icoIbo = 0;
    VGVertexArray m_vao = 0;
    int m_numIndices = 0;
};

#endif // AtmosphereComponentRenderer_h__


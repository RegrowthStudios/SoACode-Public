///
/// PlanetRingsComponentRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 23 May 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// The renderer for PlanetRingsComponent
///

#pragma once

#ifndef PlanetRingsComponentRenderer_h__
#define PlanetRingsComponentRenderer_h__

#include <Vorb/ecs/ECS.h>
#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/gtypes.h>
#include <Vorb/graphics/FullQuadVBO.h>

struct SpaceLightComponent;
struct PlanetRingsComponent;

DECL_VG(class GLProgram)

class PlanetRingsComponentRenderer {
public:
    PlanetRingsComponentRenderer();
    ~PlanetRingsComponentRenderer();

    void draw(const PlanetRingsComponent& prCmp,
              const f32m4& VP,
              const f32v3& relCamPos,
              const f32v3& lightPos,
              const f32 planetRadius,
              const f32 zCoef,
              const SpaceLightComponent* spComponent);
    void disposeShader();
private:

    vg::GLProgram* m_program = nullptr;
    vg::FullQuadVBO m_quad;
    bool m_isInitialized = false;
};

#endif // PlanetRingsComponentRenderer_h__

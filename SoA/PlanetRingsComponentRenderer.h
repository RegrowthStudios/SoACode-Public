///
/// PlanetRingsComponentRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 23 May 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// The renderer for PlanetRingsComponent
///

#pragma once

#ifndef PlanetRingsComponentRenderer_h__
#define PlanetRingsComponentRenderer_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/ecs/ComponentTable.hpp>
#include <Vorb/ecs/ECS.h>
#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/gtypes.h>
#include "SpaceSystemComponents.h"

struct SpaceLightComponent;
struct PlanetRingsComponent;

// TODO(Ben): Use a renderable component instead
struct RenderableRing {
    PlanetRing ring;
    VGTexture texture;
};

class PlanetRingsComponentRenderer {
public:
    ~PlanetRingsComponentRenderer();

    void initGL();
    void draw(const PlanetRingsComponent& prCmp,
              vecs::EntityID eid,
              const f32m4& VP,
              const f32v3& relCamPos,
              const f32v3& lightPos,
              const f32 planetRadius,
              const f32 zCoef,
              const SpaceLightComponent* spComponent);
    void dispose();
private:
    // TODO(Ben): Use a renderable component instead
    std::unordered_map<vecs::EntityID, std::vector<RenderableRing>> m_renderableRings;
    vg::GLProgram m_program;
    vg::FullQuadVBO m_quad;
    bool m_isInitialized = false;
};

#endif // PlanetRingsComponentRenderer_h__

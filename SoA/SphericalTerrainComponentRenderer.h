///
/// SphericalTerrainComponentRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Jan 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Renders spherical terrain components
///

#pragma once

#ifndef SphericalTerrainComponentRenderer_h__
#define SphericalTerrainComponentRenderer_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/GLProgram.h>

class Camera;
struct AtmosphereComponent;
struct AxisRotationComponent;
struct NamePositionComponent;
struct SpaceLightComponent;
struct SphericalTerrainComponent;

class SphericalTerrainComponentRenderer {
public:
    ~SphericalTerrainComponentRenderer();
    void initGL();
    void draw(SphericalTerrainComponent& cmp,
              const Camera* camera,
              const f32v3& lightDir,
              const f64v3& position,
              const f32 zCoef,
              const SpaceLightComponent* spComponent,
              const AxisRotationComponent* arComponent,
              const AtmosphereComponent* aComponent);
    void dispose();
private:
    vg::GLProgram m_terrainProgram;
    vg::GLProgram m_waterProgram;
};

#endif // SphericalTerrainComponentRenderer_h__

///
/// FarTerrainComponentRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 22 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Renderer for far terrain patches
///

#pragma once

#ifndef FarTerrainComponentRenderer_h__
#define FarTerrainComponentRenderer_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/GLProgram.h>

class Camera;
struct AtmosphereComponent;
struct AxisRotationComponent;
struct FarTerrainComponent;
struct NamePositionComponent;
struct SpaceLightComponent;
struct SphericalTerrainComponent;

class FarTerrainComponentRenderer {
public:
    ~FarTerrainComponentRenderer();
    void draw(const FarTerrainComponent& cmp,
              const Camera* camera,
              const f64v3& lightDir,
              const f32 zCoef,
              const SpaceLightComponent* spComponent,
              const AxisRotationComponent* arComponent,
              const AtmosphereComponent* aComponent);
    void disposeShaders();
private:
    void buildShaders();

    vg::GLProgram m_farTerrainProgram;
    vg::GLProgram m_farWaterProgram;
};

#endif // FarTerrainComponentRenderer_h__

///
/// SphericalTerrainComponentRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Renders spherical terrain components
///

#pragma once

#ifndef SphericalTerrainComponentRenderer_h__
#define SphericalTerrainComponentRenderer_h__

#include <Vorb/VorbPreDecl.inl>

struct AxisRotationComponent;
struct NamePositionComponent;
struct SpaceLightComponent;
struct SphericalTerrainComponent;
class Camera;

DECL_VG(class GLProgram);

class SphericalTerrainComponentRenderer {
public:
    ~SphericalTerrainComponentRenderer();
    void draw(SphericalTerrainComponent& cmp,
              const Camera* camera,
              const f64v3& lightPos,
              const SpaceLightComponent* spComponent,
              const NamePositionComponent* npComponent,
              const AxisRotationComponent* arComponent);

private:
    void buildShaders();

    vg::GLProgram* m_terrainProgram = nullptr;
    vg::GLProgram* m_waterProgram = nullptr;
};

#endif // SphericalTerrainComponentRenderer_h__
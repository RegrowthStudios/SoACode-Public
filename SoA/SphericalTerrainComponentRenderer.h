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

struct SphericalTerrainComponent;
struct NamePositionComponent;
struct AxisRotationComponent;
class Camera;

namespace vorb {
    namespace core {
        namespace graphics {
            class GLProgram;
        }
    }
}
namespace vg = vorb::core::graphics;

class SphericalTerrainComponentRenderer {
public:
    ~SphericalTerrainComponentRenderer();
    void draw(SphericalTerrainComponent& cmp,
              const Camera* camera,
              const Camera* voxelCamera,
              vg::GLProgram* terrainProgram,
              vg::GLProgram* waterProgram,
              const f32v3& lightDir,
              const NamePositionComponent* npComponent,
              const AxisRotationComponent* arComponent);

private:
    void buildFarTerrainShaders();

    vg::GLProgram* m_farTerrainProgram = nullptr;
    vg::GLProgram* m_farWaterProgram = nullptr;
};

#endif // SphericalTerrainComponentRenderer_h__
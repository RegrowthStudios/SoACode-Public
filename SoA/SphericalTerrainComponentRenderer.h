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

class SphericalTerrainComponent;
class NamePositionComponent;
class AxisRotationComponent;
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
    void draw(SphericalTerrainComponent& cmp,
              const Camera* camera,
              vg::GLProgram* terrainProgram,
              vg::GLProgram* waterProgram,
              const NamePositionComponent* npComponent,
              const AxisRotationComponent* arComponent);
};

#endif // SphericalTerrainComponentRenderer_h__
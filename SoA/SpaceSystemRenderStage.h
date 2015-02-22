///
/// SpaceSystemRenderStage.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 5 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Render stage for drawing the space system
///

#pragma once

#ifndef SpaceSystemRenderStage_h__
#define SpaceSystemRenderStage_h__

#include <Vorb/graphics/IRenderStage.h>
#include <Vorb/ecs/ECS.h>
#include <Vorb/VorbPreDecl.inl>

#include "SphericalTerrainComponentRenderer.h"
#include "FarTerrainComponentRenderer.h"
#include "SystemARRenderer.h"

class App;
class MainMenuSystemViewer;
class SpaceSystem;
class GameSystem;
class SpriteBatch;
class SpriteFont;

class SpaceSystemRenderStage : public vg::IRenderStage {
public:
    SpaceSystemRenderStage(ui32v2 viewport,
                           SpaceSystem* spaceSystem,
                           GameSystem* gameSystem,
                           const MainMenuSystemViewer* systemViewer,
                           const Camera* spaceCamera,
                           const Camera* farTerrainCamera,
                           VGTexture selectorTexture);
    ~SpaceSystemRenderStage();

    void setViewport(const ui32v2& viewport) { m_viewport = f32v2(viewport); }

    /// Draws the render stage
    virtual void draw() override;
private:
    /// Renders the space bodies
    /// @param camera: Camera for rendering
    /// @param terrainProgram: Program for rendering terrain
    /// @param waterProgram: Program for rendering water
    void drawBodies();

    /// Gets light source relative to a component
    /// @param cmp: Spherical terrain component to query for
    /// @param pos: Position of brightest light
    /// @return brightest light source relative to cmp
    SpaceLightComponent* getBrightestLight(SphericalTerrainComponent& cmp, OUT f64v3& pos);

    f32v2 m_viewport;
    SpaceSystem* m_spaceSystem = nullptr;
    GameSystem* m_gameSystem = nullptr;
    const MainMenuSystemViewer* m_mainMenuSystemViewer = nullptr;
    const Camera* m_spaceCamera = nullptr;
    const Camera* m_farTerrainCamera = nullptr;
    VGTexture m_selectorTexture = 0;

    SystemARRenderer m_systemARRenderer;
    SphericalTerrainComponentRenderer m_sphericalTerrainComponentRenderer;
    FarTerrainComponentRenderer m_farTerrainComponentRenderer;
};

#endif // SpaceSystemRenderStage_h__
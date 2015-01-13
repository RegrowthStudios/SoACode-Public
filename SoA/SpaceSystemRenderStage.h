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
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ecs/ECS.h>

#include "OrbitComponentRenderer.h"
#include "SphericalTerrainComponentRenderer.h"

class App;
class MainMenuSystemViewer;
class SpaceSystem;
class SpriteBatch;
class SpriteFont;

class SpaceSystemRenderStage : public vg::IRenderStage {
public:
    SpaceSystemRenderStage(ui32v2 viewport,
                           SpaceSystem* spaceSystem,
                           const MainMenuSystemViewer* systemViewer,
                           const Camera* camera,
                           vg::GLProgram* colorProgram,
                           vg::GLProgram* terrainProgram,
                           vg::GLProgram* waterProgram,
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

    /// Renders the space paths
    /// @param camera: Camera for rendering
    /// @param colorProgram: glProgram for basic color
    void drawPaths();

    void drawHud();

    SpriteBatch* m_spriteBatch = nullptr;
    SpriteFont* m_spriteFont = nullptr;

    f32v2 m_viewport;
    SpaceSystem* m_spaceSystem = nullptr;
    const MainMenuSystemViewer* m_mainMenuSystemViewer = nullptr;
    const Camera* m_camera = nullptr;
    vg::GLProgram* m_colorProgram = nullptr;
    vg::GLProgram* m_terrainProgram = nullptr;
    vg::GLProgram* m_waterProgram = nullptr;
    VGTexture m_selectorTexture = 0;

    OrbitComponentRenderer m_orbitComponentRenderer;
    SphericalTerrainComponentRenderer m_sphericalTerrainComponentRenderer;
};

#endif // SpaceSystemRenderStage_h__
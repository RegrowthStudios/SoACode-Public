///
/// SystemARRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 22 Feb 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Augmented reality renderer for Space Systems
///

#pragma once

#ifndef SystemARRenderer_h__
#define SystemARRenderer_h__

#include <Vorb/graphics/gtypes.h>
#include <Vorb/VorbPreDecl.inl>
#include "OrbitComponentRenderer.h"
#include <Vorb/graphics/GLProgram.h>

class Camera;
class MainMenuSystemViewer;
class SpaceSystem;
class ModPathResolver;

DECL_VG(class SpriteBatch;
        class SpriteFont)

class SystemARRenderer {
public:
    SystemARRenderer();
    ~SystemARRenderer();

    void init(const ModPathResolver* textureResolver);
    void initGL();

    void draw(SpaceSystem* spaceSystem, const Camera* camera,
              OPT const MainMenuSystemViewer* systemViewer,
              const f32v2& viewport);

    void dispose();

private:
    void loadTextures();
    // Renders space paths
    void drawPaths();
    // Renders heads up display
    void drawHUD();

    vg::GLProgram m_colorProgram;
    vg::SpriteBatch* m_spriteBatch = nullptr;
    vg::SpriteFont* m_spriteFont = nullptr;

    // Helper variables to avoid passing
    const ModPathResolver* m_textureResolver = nullptr;
    SpaceSystem* m_spaceSystem = nullptr;
    const Camera* m_camera = nullptr;
    const MainMenuSystemViewer* m_systemViewer = nullptr;
    VGTexture m_selectorTexture = 0;
    VGTexture m_baryTexture = 0;
    f32v2 m_viewport;
    f32 m_zCoef;

    OrbitComponentRenderer m_orbitComponentRenderer;
};

#endif // SystemARRenderer_h__

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

#include "AtmosphereComponentRenderer.h"
#include "Camera.h"
#include "FarTerrainComponentRenderer.h"
#include "GasGiantComponentRenderer.h"
#include "LenseFlareRenderer.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainComponentRenderer.h"
#include "StarComponentRenderer.h"
#include "SystemARRenderer.h"

class App;
class GameSystem;
class MainMenuSystemViewer;
class SoaState;
class SpaceSystem;
class SpriteBatch;
class SpriteFont;
struct MTRenderState;

class SpaceSystemRenderStage : public vg::IRenderStage {
public:
    SpaceSystemRenderStage(const SoaState* soaState,
                           ui32v2 viewport,
                           SpaceSystem* spaceSystem,
                           GameSystem* gameSystem,
                           const MainMenuSystemViewer* systemViewer,
                           const Camera* spaceCamera,
                           const Camera* farTerrainCamera,
                           VGTexture selectorTexture);
    ~SpaceSystemRenderStage();

    void setViewport(const ui32v2& viewport) { m_viewport = f32v2(viewport); }

    /// Call this every frame before render
    void setRenderState(const MTRenderState* renderState);
    /// Draws the render stage
    virtual void render() override;

    /// Renders star glows requested in the render call. Call after HDR
    void renderStarGlows();

    virtual void reloadShader() override;

    /// Gets the desired near clipping plane based on distances of planets
    /// Returns 0 when it cannot be calculated
    /// @param verticalFOV: vertical fov of camera in degrees
    /// @param aspectRatio: camera aspect ratio
    f32 getDynamicNearPlane(float verticalFOV, float aspectRatio);

    void setShowAR(bool showAR) { m_showAR = showAR; }

    bool needsFaceTransitionAnimation = false; ///< true when we need to fade out camera for transition between faces
private:
    /// Renders the space bodies
    /// @param camera: Camera for rendering
    /// @param terrainProgram: Program for rendering terrain
    /// @param waterProgram: Program for rendering water
    void drawBodies();

    /// Gets light source relative to a component
    /// @param cmp: position component 
    /// @param pos: Returned position of brightest light
    /// @return brightest light source relative to cmp
    SpaceLightComponent* getBrightestLight(NamePositionComponent& npCmp, OUT f64v3& pos);

    /// Gets the position of a body using MTRenderState if needed
    /// @return pointer to the position
    const f64v3* getBodyPosition(NamePositionComponent& npCmp, vecs::EntityID eid);

    f32v2 m_viewport;
    SpaceSystem* m_spaceSystem = nullptr;
    GameSystem* m_gameSystem = nullptr;
    const MainMenuSystemViewer* m_mainMenuSystemViewer = nullptr;
    const Camera* m_spaceCamera = nullptr;
    const Camera* m_farTerrainCamera = nullptr;
    VGTexture m_selectorTexture = 0;
    const MTRenderState* m_renderState = nullptr;

    AtmosphereComponentRenderer m_atmosphereComponentRenderer;
    FarTerrainComponentRenderer m_farTerrainComponentRenderer;
    GasGiantComponentRenderer m_gasGiantComponentRenderer;
    LenseFlareRenderer m_lensFlareRenderer;
    SphericalTerrainComponentRenderer m_sphericalTerrainComponentRenderer;
    StarComponentRenderer m_starRenderer;
    SystemARRenderer m_systemARRenderer;
    f64 m_closestPatchDistance2 = 500.0; ///< Used for determining dynamic near clipping plane

    std::vector<std::pair<StarComponent, f64v3> > m_starGlowsToRender;
    bool m_showAR = true;
};

#endif // SpaceSystemRenderStage_h__
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

#include <Vorb/ecs/ECS.h>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/AssetLoader.h>

#include "CloudsComponentRenderer.h"
#include "AtmosphereComponentRenderer.h"
#include "Camera.h"
#include "FarTerrainComponentRenderer.h"
#include "GasGiantComponentRenderer.h"
#include "LenseFlareRenderer.h"
#include "PlanetRingsComponentRenderer.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainComponentRenderer.h"
#include "StarComponentRenderer.h"
#include "SystemARRenderer.h"
#include "IRenderStage.h"

class App;
class GameSystem;
class MainMenuSystemViewer;
struct SoaState;
class SpaceSystem;
class SpriteBatch;
class SpriteFont;
struct MTRenderState;

class SpaceSystemRenderStage : public IRenderStage {
public:
    void init(vui::GameWindow* window, StaticLoadContext& context) override;

    void hook(SoaState* state, const Camera* spaceCamera, const Camera* farTerrainCamera = nullptr);

    void load(StaticLoadContext& context) override;

    void dispose(StaticLoadContext& context) override;

    //TODO(Ben): Pointer to window viewport instead?
    void setViewport(const ui32v2& viewport) { m_viewport = f32v2(viewport); }

    /// Call this every frame before render
    void setRenderState(const MTRenderState* renderState);
    /// Draws the render stage
    virtual void render(const Camera* camera) override;

    /// Renders star glows requested in the render call. Call after HDR
    void renderStarGlows(const f32v3& colorMult);

    void setSystemViewer(const MainMenuSystemViewer* viewer) { m_mainMenuSystemViewer = viewer; }
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
    const MainMenuSystemViewer* m_mainMenuSystemViewer = nullptr;
    const Camera* m_spaceCamera = nullptr;
    const Camera* m_farTerrainCamera = nullptr;
    const MTRenderState* m_renderState = nullptr;
    
    CloudsComponentRenderer m_cloudsComponentRenderer;
    AtmosphereComponentRenderer m_atmosphereComponentRenderer;
    PlanetRingsComponentRenderer m_ringsRenderer;
    FarTerrainComponentRenderer m_farTerrainComponentRenderer;
    GasGiantComponentRenderer m_gasGiantComponentRenderer;
    LenseFlareRenderer m_lensFlareRenderer;
    SphericalTerrainComponentRenderer m_sphericalTerrainComponentRenderer;
    StarComponentRenderer m_starRenderer;
    SystemARRenderer m_systemARRenderer;
  
    std::vector<std::pair<StarComponent, f64v3> > m_starGlowsToRender;
    bool m_showAR = true;

    vcore::GLRPC m_rpc;
};

#endif // SpaceSystemRenderStage_h__
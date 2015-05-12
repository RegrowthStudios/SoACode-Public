/// 
///  GamePlayRenderPipeline.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements the render pipeline for the
///  GamePlayScreen.
///
#pragma once

#ifndef GamePlayRenderPipeline_h__
#define GamePlayRenderPipeline_h__

#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/RenderPipeline.h>
#include <Vorb/graphics/RTSwapChain.hpp>

#include "Camera.h"
#include "GameRenderParams.h"
#include "NightVisionRenderStage.h"
#include "ColoredFullQuadRenderer.h"

/// Forward declarations
class App;
class ChunkGridRenderStage;
class ChunkMeshManager;
class ChunkSlot;
class CutoutVoxelRenderStage;
class DevHudRenderStage;
class GameSystem;
class HdrRenderStage;
class LiquidVoxelRenderStage;
struct MTRenderState;
class MeshManager;
class NightVisionRenderStage;
class OpaqueVoxelRenderStage;
class PDA;
class PauseMenu;
class PauseMenuRenderStage;
class PdaRenderStage;
class PhysicsBlockRenderStage;
class Player;
class SkyboxRenderStage;
class SoaState;
class SpaceSystem;
class SpaceSystemRenderStage;
class TransparentVoxelRenderStage;

class GameplayRenderPipeline : public vg::RenderPipeline {
public:
    GameplayRenderPipeline();
    ~GameplayRenderPipeline();

    /// Initializes the pipeline and passes dependencies
    /// @param viewport: The viewport to draw to.

    void init(const ui32v4& viewport, const SoaState* soaState, const App* app,
              const PDA* pda,
              SpaceSystem* spaceSystem,
              GameSystem* gameSystem,
              const PauseMenu* pauseMenu);

    /// Call this every frame before render
    void setRenderState(const MTRenderState* renderState);

    /// Renders the pipeline.
    /// Make sure to call setRenderState first.
    virtual void render() override;

    /// Frees all resources
    virtual void destroy(bool shouldDisposeStages) override;

    /// Cycles the dev hud
    /// @param offset: How much to offset the current mode
    void cycleDevHud(int offset = 1);
    /// Toggle the visibility of night vision
    void toggleNightVision();
    /// Load night vision data
    void loadNightVision();
    /// Toggle the visibility of chunkGrid
    void toggleChunkGrid();
    /// Cycle poly mode for voxels
    void cycleDrawMode();

    void takeScreenshot() { m_shouldScreenshot = true; }
private:
    void updateCameras();
    void dumpScreenshot();

    SkyboxRenderStage* m_skyboxRenderStage = nullptr; ///< Renders the skybox
    PhysicsBlockRenderStage* m_physicsBlockRenderStage = nullptr; ///< Renders the physics blocks
    OpaqueVoxelRenderStage* m_opaqueVoxelRenderStage = nullptr; ///< Renders opaque voxels
    CutoutVoxelRenderStage* m_cutoutVoxelRenderStage = nullptr; ///< Renders cutout voxels
    ChunkGridRenderStage* m_chunkGridRenderStage = nullptr;
    TransparentVoxelRenderStage* m_transparentVoxelRenderStage = nullptr; ///< Renders transparent voxels
    LiquidVoxelRenderStage* m_liquidVoxelRenderStage = nullptr; ///< Renders liquid voxels
    DevHudRenderStage* m_devHudRenderStage = nullptr; ///< Renders the dev/debug HUD
    PdaRenderStage* m_pdaRenderStage = nullptr; ///< Renders the PDA
    PauseMenuRenderStage* m_pauseMenuRenderStage = nullptr; ///< Renders the pause menu
    NightVisionRenderStage* m_nightVisionRenderStage = nullptr; ///< Renders night vision
    HdrRenderStage* m_hdrRenderStage = nullptr; ///< Renders HDR post-processing
    SpaceSystemRenderStage* m_spaceSystemRenderStage = nullptr; ///< Render space and planets

    ColoredFullQuadRenderer m_coloredQuadRenderer; ///< For rendering full screen colored quads

    vg::GLRenderTarget* m_hdrFrameBuffer = nullptr; ///< Framebuffer needed for the HDR rendering
    vg::RTSwapChain<2>* m_swapChain = nullptr; ///< Swap chain of framebuffers used for post-processing
    vg::FullQuadVBO m_quad; ///< Quad used for post-processing

    GameRenderParams m_gameRenderParams; ///< Shared rendering parameters for voxels
    
    const SoaState* m_soaState = nullptr; ///< Game State

    // TODO: This is only for visualization purposes, must remove
    std::vector<NightVisionRenderParams> m_nvParams; ///< Different night vision styles
    i32 m_nvIndex = 0;
    VGEnum m_drawMode;

    ui32v4 m_viewport; ///< Viewport to draw to
    Camera m_spaceCamera; ///< handle to world camera
    Camera m_farTerrainCamera; ///< Camera for far terrain only
    Camera m_voxelCamera; ///< handle to voxel camera
    ChunkMeshManager* m_meshManager; ///< Handle to the meshes
    const MTRenderState* m_renderState = nullptr; ///< The current MT render state
    bool m_voxelsActive = false;
    float m_coloredQuadAlpha = 0.0f;
    bool m_increaseQuadAlpha = false;
    bool m_shouldScreenshot = false;
};

#endif // GamePlayRenderPipeline_h__
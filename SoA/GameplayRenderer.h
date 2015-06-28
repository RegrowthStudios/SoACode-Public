/// 
///  GameplayRenderer.h
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

#ifndef GamePlayRenderer_h__
#define GamePlayRenderer_h__

#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GBuffer.h>
#include <Vorb/graphics/RTSwapChain.hpp>
#include <Vorb/RPC.h>

#include "Camera.h"
#include "ChunkGridRenderStage.h"
#include "ColoredFullQuadRenderer.h"
#include "CutoutVoxelRenderStage.h"
#include "DevHudRenderStage.h"
#include "GameRenderParams.h"
#include "HdrRenderStage.h"
#include "LiquidVoxelRenderStage.h"
#include "NightVisionRenderStage.h"
#include "OpaqueVoxelRenderStage.h"
#include "PauseMenuRenderStage.h"
#include "PdaRenderStage.h"
#include "PhysicsBlockRenderStage.h"
#include "SkyboxRenderStage.h"
#include "SpaceSystemRenderStage.h"
#include "TransparentVoxelRenderStage.h"
#include "SsaoRenderStage.h"

/// Forward declarations
class App;
class ChunkGridRenderStage;
class ChunkMeshManager;
class ChunkSlot;
class CutoutVoxelRenderStage;
class DevHudRenderStage;
class GameSystem;
class GameplayScreen;
class HdrRenderStage;
class LiquidVoxelRenderStage;
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
class SpaceSystem;
class SpaceSystemRenderStage;
class TransparentVoxelRenderStage;
struct MTRenderState;
struct CommonState;
struct SoaState;

class GameplayRenderer {
public:
    /// Initializes the pipeline and passes dependencies
    void init(vui::GameWindow* window, StaticLoadContext& context,
              GameplayScreen* gameplayScreen, CommonState* commonState);

    /// Call this every frame before render
    void setRenderState(const MTRenderState* renderState);

    void hook();

    void load(StaticLoadContext& context);

    void dispose(StaticLoadContext& context);

    void reloadShaders();

    void updateGL();

    /// Renders the pipeline.
    /// Make sure to call setRenderState first.
    virtual void render();

    /// Cycles the dev hud
    /// @param offset: How much to offset the current mode
    void cycleDevHud(int offset = 1);
    /// Toggle the visibility of night vision
    void toggleNightVision();
    /// Load night vision data
    void loadNightVision();
    /// Toggle the visibility of chunkGrid
    void toggleChunkGrid();

    void toggleWireframe() { m_wireframe = !m_wireframe; }

    void takeScreenshot() { m_shouldScreenshot = true; }

    volatile const bool& isLoaded() const { return m_isLoaded; }

    struct {
        OpaqueVoxelRenderStage opaqueVoxel; ///< Renders opaque voxels
        CutoutVoxelRenderStage cutoutVoxel; ///< Renders cutout voxels
        ChunkGridRenderStage chunkGrid;
        TransparentVoxelRenderStage transparentVoxel; ///< Renders transparent voxels
        LiquidVoxelRenderStage liquidVoxel; ///< Renders liquid voxels
        DevHudRenderStage devHud; ///< Renders the dev/debug HUD
        PdaRenderStage pda; ///< Renders the PDA
        PauseMenuRenderStage pauseMenu; ///< Renders the pause menu
        NightVisionRenderStage nightVision; ///< Renders night vision
     //   SsaoRenderStage ssao; ///< Renders night vision
    } stages;

private:
    void updateCameras();
    void dumpScreenshot();

    ColoredFullQuadRenderer m_coloredQuadRenderer; ///< For rendering full screen colored quads

    vg::GBuffer m_hdrTarget; ///< Framebuffer needed for the HDR rendering
    vg::RTSwapChain<2> m_swapChain; ///< Swap chain of framebuffers used for post-processing

    GameRenderParams m_gameRenderParams; ///< Shared rendering parameters for voxels
    GameplayScreen* m_gameplayScreen = nullptr;

    vui::GameWindow* m_window;
    CommonState* m_commonState = nullptr;
    SoaState* m_state = nullptr; ///< Game State
    vcore::RPCManager m_glrpc;

    std::thread* m_loadThread = nullptr;
    volatile bool m_isLoaded = false;

    // TODO: This is only for visualization purposes, must remove
    std::vector<NightVisionRenderParams> m_nvParams; ///< Different night vision styles
    ui32 m_nvIndex = 0;

    ui32v4 m_viewport; ///< Viewport to draw to
    ChunkMeshManager* m_meshManager; ///< Handle to the meshes
    const MTRenderState* m_renderState = nullptr; ///< The current MT render state
    bool m_voxelsActive = false;
    float m_coloredQuadAlpha = 0.0f;
    bool m_increaseQuadAlpha = false;
    bool m_wireframe = false;
    bool m_shouldScreenshot = false;
};

#endif // GamePlayRenderer_h__

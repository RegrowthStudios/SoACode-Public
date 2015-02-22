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
#include <Vorb/graphics/IRenderPipeline.h>
#include <Vorb/graphics/RTSwapChain.hpp>

#include "Camera.h"
#include "GLProgramManager.h"
#include "GameRenderParams.h"
#include "NightVisionRenderStage.h"

/// Forward declarations
class App;
class ChunkGridRenderStage;
class ChunkSlot;
class CutoutVoxelRenderStage;
class DevHudRenderStage;
class GameSystem;
class HdrRenderStage;
class IAwesomiumInterface;
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
class SoaState;
class SpaceSystem;
class SpaceSystemRenderStage;
class TransparentVoxelRenderStage;

class GamePlayRenderPipeline : public vg::IRenderPipeline {
public:
    GamePlayRenderPipeline();
    ~GamePlayRenderPipeline();

    /// Initializes the pipeline and passes dependencies
    /// @param viewport: The viewport to draw to.

    void init(const ui32v4& viewport, const SoaState* soaState, const App* app,
              const PDA* pda,
              SpaceSystem* spaceSystem,
              GameSystem* gameSystem,
              const PauseMenu* pauseMenu);

    /// Renders the pipeline
    virtual void render() override;

    /// Frees all resources
    virtual void destroy() override;

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
private:
    void updateCameras();

    SkyboxRenderStage* _skyboxRenderStage = nullptr; ///< Renders the skybox
    PhysicsBlockRenderStage* _physicsBlockRenderStage = nullptr; ///< Renders the physics blocks
    OpaqueVoxelRenderStage* _opaqueVoxelRenderStage = nullptr; ///< Renders opaque voxels
    CutoutVoxelRenderStage* _cutoutVoxelRenderStage = nullptr; ///< Renders cutout voxels
    ChunkGridRenderStage* _chunkGridRenderStage = nullptr;
    TransparentVoxelRenderStage* _transparentVoxelRenderStage = nullptr; ///< Renders transparent voxels
    LiquidVoxelRenderStage* _liquidVoxelRenderStage = nullptr; ///< Renders liquid voxels
    DevHudRenderStage* _devHudRenderStage = nullptr; ///< Renders the dev/debug HUD
    PdaRenderStage* _pdaRenderStage = nullptr; ///< Renders the PDA
    PauseMenuRenderStage* _pauseMenuRenderStage = nullptr; ///< Renders the pause menu
    NightVisionRenderStage* _nightVisionRenderStage = nullptr; ///< Renders night vision
    HdrRenderStage* _hdrRenderStage = nullptr; ///< Renders HDR post-processing
    SpaceSystemRenderStage* m_spaceSystemRenderStage = nullptr; ///< Render space and planets

    vg::GLRenderTarget* _hdrFrameBuffer = nullptr; ///< Framebuffer needed for the HDR rendering
    vg::RTSwapChain<2>* _swapChain = nullptr; ///< Swap chain of framebuffers used for post-processing
    vg::FullQuadVBO _quad; ///< Quad used for post-processing

    GameRenderParams _gameRenderParams; ///< Shared rendering parameters for voxels
    
    const SoaState* m_soaState = nullptr; ///< Game State

    // TODO: This is only for visualization purposes, must remove
    std::vector<NightVisionRenderParams> _nvParams; ///< Different night vision styles
    i32 _nvIndex = 0;
    VGEnum m_drawMode;

    ui32v4 _viewport; ///< Viewport to draw to
    Camera m_spaceCamera; ///< handle to world camera
    Camera m_farTerrainCamera; ///< Camera for far terrain only
    Camera m_voxelCamera; ///< handle to voxel camera
    const MeshManager* _meshManager; ///< Handle to the meshes
    bool m_voxelsActive = false;
};

#endif // GamePlayRenderPipeline_h__
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

#include <Vorb/FullQuadVBO.h>
#include <Vorb/GLRenderTarget.h>
#include <Vorb/IRenderPipeline.h>
#include <Vorb/RTSwapChain.hpp>

#include "GameRenderParams.h"
#include "GLProgramManager.h"
#include "NightVisionRenderStage.h"

/// Forward declarations
class App;
class Camera;
class ChunkGridRenderStage;
class ChunkSlot;
class CutoutVoxelRenderStage;
class DevHudRenderStage;
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
class SpaceSystem;
class SpaceSystemRenderStage;
class TransparentVoxelRenderStage;

class GamePlayRenderPipeline : public vg::IRenderPipeline {
public:
    GamePlayRenderPipeline();
    ~GamePlayRenderPipeline();

    /// Initializes the pipeline and passes dependencies
    /// @param viewport: The viewport to draw to.
    /// @param chunkCamera: The camera used for voxel rendering.
    /// @param worldCamera: The camera used for planet rendering.
    /// @param app: Handle to the App
    /// @param player: Handle to the Player
    /// @param meshManager: Stores all needed meshes
    /// @param pda: The PDA to render
    /// @param glProgramManager: Contains all the needed GLPrograms
    /// @param spaceSystem: Used for planet and AR rendering
    /// @param pauseMenu: The PauseMenu to render
    /// @param chunkSlots: The chunk slots for debug rendering
    void init(const ui32v4& viewport, Camera* chunkCamera,
              const Camera* worldCamera, const App* app,
              const Player* player, const MeshManager* meshManager,
              const PDA* pda, const vg::GLProgramManager* glProgramManager,
              SpaceSystem* spaceSystem,
              const PauseMenu* pauseMenu, const std::vector<ChunkSlot>& chunkSlots);

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
    
    // TODO: This is only for visualization purposes, must remove
    std::vector<NightVisionRenderParams> _nvParams; ///< Different night vision styles
    i32 _nvIndex = 0;
    VGEnum m_drawMode;

    ui32v4 _viewport; ///< Viewport to draw to
    const Camera* _worldCamera = nullptr; ///< handle to world camera
    const Camera* _chunkCamera = nullptr; ///< handle to chunk camera
    const MeshManager* _meshManager; ///< Handle to the meshes
};

#endif // GamePlayRenderPipeline_h__
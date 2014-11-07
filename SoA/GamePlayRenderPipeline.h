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

#include "FullQuadVBO.h"
#include "GameRenderParams.h"
#include "GLProgramManager.h"
#include "GLRenderTarget.h"
#include "IRenderPipeline.h"
#include "RTSwapChain.hpp"

/// Forward declarations
class App;
class Camera;
class CutoutVoxelRenderStage;
class DevHudRenderStage;
class HdrRenderStage;
class IAwesomiumInterface;
class LiquidVoxelRenderStage;
class MeshManager;
class OpaqueVoxelRenderStage;
class PDA;
class PdaRenderStage;
class PlanetRenderStage;
class Player;
class SkyboxRenderStage;
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
    void init(const ui32v4& viewport, Camera* chunkCamera,
              const Camera* worldCamera, const App* app,
              const Player* player, const MeshManager* meshManager,
              const PDA* pda, const vg::GLProgramManager* glProgramManager);

    /// Renders the pipeline
    virtual void render() override;

    /// Frees all resources
    virtual void destroy() override;

    /// Cycles the dev hud
    /// @param offset: How much to offset the current mode
    void cycleDevHud(int offset = 1);
private: 
    SkyboxRenderStage* _skyboxRenderStage = nullptr; ///< Renders the skybox
    PlanetRenderStage* _planetRenderStage = nullptr; ///< Renders the planets
    OpaqueVoxelRenderStage* _opaqueVoxelRenderStage = nullptr; ///< Renders opaque voxels
    CutoutVoxelRenderStage* _cutoutVoxelRenderStage = nullptr; ///< Renders cutout voxels
    TransparentVoxelRenderStage* _transparentVoxelRenderStage = nullptr; ///< Renders transparent voxels
    LiquidVoxelRenderStage* _liquidVoxelRenderStage = nullptr; ///< Renders liquid voxels
    DevHudRenderStage* _devHudRenderStage = nullptr; ///< renders the dev/debug HUD
    PdaRenderStage* _pdaRenderStage = nullptr;
    HdrRenderStage* _hdrRenderStage = nullptr; ///< Renders HDR post-processing

    vg::GLRenderTarget* _hdrFrameBuffer = nullptr; ///< Framebuffer needed for the HDR rendering
    vg::RTSwapChain<2>* _swapChain = nullptr; ///< Swap chain of framebuffers used for post-processing
    vg::FullQuadVBO _quad; ///< Quad used for post-processing

    GameRenderParams _gameRenderParams; ///< Shared rendering parameters for voxels

    ui32v4 _viewport; ///< Viewport to draw to
    const Camera* _worldCamera = nullptr; ///< handle to world camera
    const Camera* _chunkCamera = nullptr; ///< handle to chunk camera
    const MeshManager* _meshManager; ///< Handle to the meshes
};

#endif // GamePlayRenderPipeline_h__